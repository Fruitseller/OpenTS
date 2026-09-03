/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "dsound.h"

#include <AudioToolbox/AudioToolbox.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <mutex>
#include <vector>

namespace
{
	constexpr DWORD DEFAULT_SECONDARY_BUFFER_SIZE = 32 * 1024;
	constexpr unsigned int OUTPUT_BUFFER_COUNT = 3;
	constexpr double OUTPUT_BUFFER_SECONDS = 0.02;

	bool Is_Supported_Format(WAVEFORMATEX const & format)
	{
		return(format.wFormatTag == WAVE_FORMAT_PCM
			&& (format.nChannels == 1 || format.nChannels == 2)
			&& (format.wBitsPerSample == 8 || format.wBitsPerSample == 16)
			&& format.nSamplesPerSec != 0);
	}

	WAVEFORMATEX Default_Format(void)
	{
		WAVEFORMATEX format = {};
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = 2;
		format.nSamplesPerSec = 22050;
		format.wBitsPerSample = 16;
		format.nBlockAlign = format.nChannels * (format.wBitsPerSample / 8);
		format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
		return(format);
	}

	AudioStreamBasicDescription Audio_Description(WAVEFORMATEX const & format)
	{
		AudioStreamBasicDescription description = {};
		description.mSampleRate = format.nSamplesPerSec;
		description.mFormatID = kAudioFormatLinearPCM;
		description.mFormatFlags = kAudioFormatFlagIsPacked;
		if (format.wBitsPerSample == 16) {
			description.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
		}
		description.mBytesPerPacket = format.nBlockAlign;
		description.mFramesPerPacket = 1;
		description.mBytesPerFrame = format.nBlockAlign;
		description.mChannelsPerFrame = format.nChannels;
		description.mBitsPerChannel = format.wBitsPerSample;
		return(description);
	}

	class DirectSoundBuffer final : public IDirectSoundBuffer
	{
		public:
			DirectSoundBuffer(bool primary, DWORD size, WAVEFORMATEX const & format) :
				Primary(primary),
				Format(format),
				Data(primary ? 0 : size, format.wBitsPerSample == 8 ? 0x80 : 0)
			{
			}

			~DirectSoundBuffer(void) override
			{
				Dispose_Queue();
			}

			HRESULT GetCurrentPosition(DWORD * play, DWORD * write) override
			{
				std::lock_guard lock(Mutex);
				if (Data.empty()) {
					if (play) *play = 0;
					if (write) *write = 0;
					return(DS_OK);
				}

				DWORD position = PlaybackPosition % static_cast<DWORD>(Data.size());
				if (play) *play = position;
				if (write) {
					DWORD lead = std::max<DWORD>(Format.nBlockAlign, Format.nAvgBytesPerSec / 50);
					*write = (position + lead) % static_cast<DWORD>(Data.size());
				}
				return(DS_OK);
			}

			HRESULT GetFormat(WAVEFORMATEX * format, DWORD size, DWORD * written) override
			{
				if (written) *written = sizeof(Format);
				if (!format || size < sizeof(Format)) return(E_INVALIDARG);
				std::lock_guard lock(Mutex);
				*format = Format;
				return(DS_OK);
			}

			HRESULT GetStatus(DWORD * status) override
			{
				if (!status) return(E_INVALIDARG);
				std::lock_guard lock(Mutex);
				if (!Primary && Playing && Queue) {
					UInt32 running = 0;
					UInt32 size = sizeof(running);
					if (AudioQueueGetProperty(Queue, kAudioQueueProperty_IsRunning, &running, &size) == noErr && !running) {
						Playing = false;
					}
				}
				*status = Playing ? DSBSTATUS_PLAYING : 0;
				if (Playing && Looping) *status |= DSBSTATUS_LOOPING;
				return(DS_OK);
			}

			HRESULT Lock(DWORD offset, DWORD bytes, void ** first, DWORD * first_size,
				void ** second, DWORD * second_size, DWORD flags) override
			{
				if (Primary || Data.empty() || !first || !first_size || !second || !second_size) {
					return(E_INVALIDARG);
				}

				Mutex.lock();
				Locked = true;
				if (flags & DSBLOCK_ENTIREBUFFER) {
					offset = 0;
					bytes = static_cast<DWORD>(Data.size());
				} else if (flags & DSBLOCK_FROMWRITECURSOR) {
					offset = PlaybackPosition % static_cast<DWORD>(Data.size());
				}
				offset %= static_cast<DWORD>(Data.size());
				bytes = std::min<DWORD>(bytes, static_cast<DWORD>(Data.size()));
				*first_size = std::min<DWORD>(bytes, static_cast<DWORD>(Data.size()) - offset);
				*first = Data.data() + offset;
				*second_size = bytes - *first_size;
				*second = *second_size ? Data.data() : nullptr;
				return(DS_OK);
			}

			HRESULT Play(DWORD, DWORD, DWORD flags) override
			{
				if (Primary) {
					std::lock_guard lock(Mutex);
					Playing = true;
					Looping = (flags & DSBPLAY_LOOPING) != 0;
					return(DS_OK);
				}

				std::lock_guard lock(Mutex);
				Looping = (flags & DSBPLAY_LOOPING) != 0;
				bool created = false;
				if (!Queue) {
					if (!Create_Queue()) return(E_FAIL);
					created = true;
				}
				if (Playing) return(DS_OK);
				Playing = true;
				if (created || !Primed) {
					FillPosition = PlaybackPosition;
					for (AudioQueueBufferRef buffer : OutputBuffers) {
						Fill_Output_Buffer(buffer);
					}
					Primed = true;
				}
				if (AudioQueueStart(Queue, nullptr) != noErr) {
					Playing = false;
					return(E_FAIL);
				}
				return(DS_OK);
			}

			ULONG Release(void) override
			{
				delete this;
				return(0);
			}

			HRESULT Restore(void) override
			{
				return(DS_OK);
			}

			HRESULT SetCurrentPosition(DWORD position) override
			{
				std::lock_guard lock(Mutex);
				PlaybackPosition = Data.empty() ? 0 : position % static_cast<DWORD>(Data.size());
				FillPosition = PlaybackPosition;
				return(DS_OK);
			}

			HRESULT SetFormat(WAVEFORMATEX const * format) override
			{
				if (!format || !Is_Supported_Format(*format)) return(E_INVALIDARG);
				Dispose_Queue();
				std::lock_guard lock(Mutex);
				Format = *format;
				return(DS_OK);
			}

			HRESULT SetVolume(LONG volume) override
			{
				std::lock_guard lock(Mutex);
				Volume = std::pow(10.0f, std::clamp(volume, DSBVOLUME_MIN, DSBVOLUME_MAX) / 2000.0f);
				if (Queue) AudioQueueSetParameter(Queue, kAudioQueueParam_Volume, Volume);
				return(DS_OK);
			}

			HRESULT Stop(void) override
			{
				AudioQueueRef queue = nullptr;
				{
					std::lock_guard lock(Mutex);
					Playing = false;
					queue = Queue;
					Primed = false;
				}
				if (queue) {
					AudioQueueStop(queue, true);
					AudioQueueReset(queue);
				}
				return(DS_OK);
			}

			HRESULT Unlock(void *, DWORD, void *, DWORD) override
			{
				if (!Locked) return(E_INVALIDARG);
				Locked = false;
				Mutex.unlock();
				return(DS_OK);
			}

		private:
			static void Output_Callback(void * context, AudioQueueRef, AudioQueueBufferRef buffer)
			{
				static_cast<DirectSoundBuffer *>(context)->Refill_Output_Buffer(buffer);
			}

			bool Create_Queue(void)
			{
				AudioStreamBasicDescription description = Audio_Description(Format);
				if (AudioQueueNewOutput(&description, Output_Callback, this, nullptr, nullptr, 0, &Queue) != noErr) {
					Queue = nullptr;
					return(false);
				}

				DWORD requested = std::max<DWORD>(Format.nBlockAlign,
					static_cast<DWORD>(Format.nAvgBytesPerSec * OUTPUT_BUFFER_SECONDS));
				OutputBufferSize = requested - requested % Format.nBlockAlign;
				for (AudioQueueBufferRef & buffer : OutputBuffers) {
					if (AudioQueueAllocateBuffer(Queue, OutputBufferSize, &buffer) != noErr) {
						Dispose_Queue_Unlocked();
						return(false);
					}
				}
				AudioQueueSetParameter(Queue, kAudioQueueParam_Volume, Volume);
				return(true);
			}

			void Dispose_Queue(void)
			{
				AudioQueueRef queue = nullptr;
				{
					std::lock_guard lock(Mutex);
					Playing = false;
					queue = Queue;
					Queue = nullptr;
					OutputBuffers.fill(nullptr);
				}
				if (queue) AudioQueueDispose(queue, true);
			}

			void Dispose_Queue_Unlocked(void)
			{
				AudioQueueRef queue = Queue;
				Queue = nullptr;
				OutputBuffers.fill(nullptr);
				if (queue) AudioQueueDispose(queue, true);
			}

			void Refill_Output_Buffer(AudioQueueBufferRef buffer)
			{
				std::lock_guard lock(Mutex);
				if (Playing && Queue) {
					PlaybackPosition = (PlaybackPosition + OutputBufferSize) % static_cast<DWORD>(Data.size());
					Fill_Output_Buffer(buffer);
				}
			}

			void Fill_Output_Buffer(AudioQueueBufferRef buffer)
			{
				if (!buffer || Data.empty()) return;
				BYTE * output = static_cast<BYTE *>(buffer->mAudioData);
				DWORD remaining = OutputBufferSize;
				while (remaining != 0) {
					DWORD available = static_cast<DWORD>(Data.size()) - FillPosition;
					DWORD count = std::min(available, remaining);
					std::memcpy(output, Data.data() + FillPosition, count);
					output += count;
					remaining -= count;
					FillPosition += count;
					if (FillPosition == Data.size()) {
						FillPosition = 0;
						if (!Looping) {
							Playing = false;
							std::memset(output, Format.wBitsPerSample == 8 ? 0x80 : 0, remaining);
							remaining = 0;
						}
					}
				}
				buffer->mAudioDataByteSize = OutputBufferSize;
				AudioQueueEnqueueBuffer(Queue, buffer, 0, nullptr);
			}

			bool Primary = false;
			WAVEFORMATEX Format = {};
			std::vector<BYTE> Data;
			std::mutex Mutex;
			AudioQueueRef Queue = nullptr;
			std::array<AudioQueueBufferRef, OUTPUT_BUFFER_COUNT> OutputBuffers = {};
			DWORD OutputBufferSize = 0;
			DWORD PlaybackPosition = 0;
			DWORD FillPosition = 0;
			float Volume = 1.0f;
			bool Playing = false;
			bool Looping = false;
			bool Primed = false;
			bool Locked = false;
	};

	class DirectSound final : public IDirectSound
	{
		public:
			HRESULT CreateSoundBuffer(DSBUFFERDESC const * descriptor,
				LPDIRECTSOUNDBUFFER * buffer, IUnknown *) override
			{
				if (!descriptor || !buffer) return(E_INVALIDARG);
				bool primary = descriptor->dwSize == sizeof(*descriptor)
					&& (descriptor->dwFlags & DSBCAPS_PRIMARYBUFFER) != 0;
				WAVEFORMATEX format = Default_Format();
				DWORD size = DEFAULT_SECONDARY_BUFFER_SIZE;
				if (descriptor->dwSize == sizeof(*descriptor)) {
					if (descriptor->lpwfxFormat && Is_Supported_Format(*descriptor->lpwfxFormat)) {
						format = *descriptor->lpwfxFormat;
					}
					if (descriptor->dwBufferBytes != 0 && descriptor->dwBufferBytes <= 16 * 1024 * 1024) {
						size = descriptor->dwBufferBytes;
					}
				}
				*buffer = new DirectSoundBuffer(primary, size, format);
				return(DS_OK);
			}

			HRESULT GetCaps(DSCAPS * caps) override
			{
				if (!caps || caps->dwSize < sizeof(*caps)) return(E_INVALIDARG);
				caps->dwFlags = 0;
				return(DS_OK);
			}

			ULONG Release(void) override
			{
				delete this;
				return(0);
			}

			HRESULT SetCooperativeLevel(HWND, DWORD) override
			{
				return(DS_OK);
			}
	};
}


HRESULT DirectSoundCreate(GUID const *, LPDIRECTSOUND * sound, IUnknown *)
{
	if (!sound) return(E_INVALIDARG);
	*sound = new DirectSound;
	return(DS_OK);
}
