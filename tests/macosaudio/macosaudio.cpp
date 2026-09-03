/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "dsound.h"

#include <CoreAudio/CoreAudio.h>

#include <chrono>
#include <cstring>
#include <string_view>
#include <thread>

namespace
{
	bool Has_Default_Output_Device(void)
	{
		AudioObjectPropertyAddress address = {
			kAudioHardwarePropertyDefaultOutputDevice,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMain,
		};
		AudioDeviceID device = kAudioObjectUnknown;
		UInt32 size = sizeof(device);
		return(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device) == noErr
			&& device != kAudioObjectUnknown);
	}
}

int main(int argc, char ** argv)
{
	LPDIRECTSOUND sound = nullptr;
	if (DirectSoundCreate(nullptr, &sound, nullptr) != DS_OK || !sound) return(1);

	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 22050;
	format.wBitsPerSample = 8;
	format.nBlockAlign = 1;
	format.nAvgBytesPerSec = 22050;

	DSBUFFERDESC descriptor = {};
	descriptor.dwSize = sizeof(descriptor);
	descriptor.dwFlags = DSBCAPS_CTRLVOLUME;
	descriptor.dwBufferBytes = 16;
	descriptor.lpwfxFormat = &format;

	LPDIRECTSOUNDBUFFER buffer = nullptr;
	if (sound->CreateSoundBuffer(&descriptor, &buffer, nullptr) != DS_OK || !buffer) return(2);

	void * first = nullptr;
	void * second = nullptr;
	DWORD first_size = 0;
	DWORD second_size = 0;
	if (buffer->Lock(12, 8, &first, &first_size, &second, &second_size, 0) != DS_OK) return(3);
	if (first_size != 4 || second_size != 4 || !first || !second) return(4);
	std::memset(first, 0x11, first_size);
	std::memset(second, 0x22, second_size);
	if (buffer->Unlock(first, first_size, second, second_size) != DS_OK) return(5);
	if (buffer->Lock(0, 16, &first, &first_size, &second, &second_size, 0) != DS_OK) return(6);
	if (first_size != 16 || second_size != 0 || second != nullptr) return(7);
	BYTE const * bytes = static_cast<BYTE const *>(first);
	if (bytes[0] != 0x22 || bytes[3] != 0x22 || bytes[4] != 0x80 || bytes[11] != 0x80
		|| bytes[12] != 0x11 || bytes[15] != 0x11) return(8);
	if (buffer->Unlock(first, first_size, second, second_size) != DS_OK) return(9);

	if (buffer->SetCurrentPosition(15) != DS_OK) return(10);
	DWORD play = 0;
	DWORD write = 0;
	if (buffer->GetCurrentPosition(&play, &write) != DS_OK || play != 15) return(11);

	WAVEFORMATEX returned = {};
	DWORD written = 0;
	if (buffer->GetFormat(&returned, sizeof(returned), &written) != DS_OK) return(12);
	if (written != sizeof(returned) || returned.nSamplesPerSec != 22050) return(13);
	if (buffer->SetVolume(DSBVOLUME_MIN) != DS_OK) return(14);

	bool const playback_requested = argc == 2 && std::string_view(argv[1]) == "--playback";
	if (playback_requested && !Has_Default_Output_Device()) {
		buffer->Release();
		sound->Release();
		return(77);
	}
	if (playback_requested) {
		DWORD status = 0;
		if (buffer->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) return(15);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (buffer->GetStatus(&status) != DS_OK || !(status & DSBSTATUS_PLAYING)) return(16);
		if (buffer->Stop() != DS_OK) return(17);
		if (buffer->GetStatus(&status) != DS_OK || status != 0) return(18);
	}

	buffer->Release();
	sound->Release();
	return(0);
}
