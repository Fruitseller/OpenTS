/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "wincompat.h"

struct WAVEFORMATEX
{
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
	WORD wBitsPerSample;
	WORD cbSize;
};

using LPWAVEFORMATEX = WAVEFORMATEX *;
using LPCWAVEFORMATEX = WAVEFORMATEX const *;

struct DSBUFFERDESC
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwBufferBytes;
	DWORD dwReserved;
	LPWAVEFORMATEX lpwfxFormat;
	GUID guid3DAlgorithm;
};

struct DSCAPS
{
	DWORD dwSize;
	DWORD dwFlags;
};

constexpr HRESULT DS_OK = S_OK;
constexpr HRESULT DSERR_BUFFERLOST = static_cast<HRESULT>(0x88780096u);
constexpr WORD WAVE_FORMAT_PCM = 1;
constexpr DWORD DSBCAPS_PRIMARYBUFFER = 0x00000001;
constexpr DWORD DSBCAPS_CTRLVOLUME = 0x00000080;
constexpr DWORD DSBCAPS_GETCURRENTPOSITION2 = 0x00010000;
constexpr DWORD DSBPLAY_LOOPING = 0x00000001;
constexpr DWORD DSBSTATUS_PLAYING = 0x00000001;
constexpr DWORD DSBSTATUS_LOOPING = 0x00000004;
constexpr DWORD DSBLOCK_FROMWRITECURSOR = 0x00000001;
constexpr DWORD DSBLOCK_ENTIREBUFFER = 0x00000002;
constexpr LONG DSBVOLUME_MIN = -10000;
constexpr LONG DSBVOLUME_MAX = 0;
constexpr DWORD DSCAPS_EMULDRIVER = 0x00000020;
constexpr DWORD DSSCL_PRIORITY = 2;

class IDirectSoundBuffer
{
	public:
		virtual ~IDirectSoundBuffer(void) = default;
		virtual HRESULT GetCurrentPosition(DWORD * play, DWORD * write) = 0;
		virtual HRESULT GetFormat(WAVEFORMATEX * format, DWORD size, DWORD * written) = 0;
		virtual HRESULT GetStatus(DWORD * status) = 0;
		virtual HRESULT Lock(DWORD offset, DWORD bytes, void ** first, DWORD * first_size,
			void ** second, DWORD * second_size, DWORD flags) = 0;
		virtual HRESULT Play(DWORD, DWORD, DWORD flags) = 0;
		virtual ULONG Release(void) = 0;
		virtual HRESULT Restore(void) = 0;
		virtual HRESULT SetCurrentPosition(DWORD position) = 0;
		virtual HRESULT SetFormat(WAVEFORMATEX const * format) = 0;
		virtual HRESULT SetVolume(LONG volume) = 0;
		virtual HRESULT Stop(void) = 0;
		virtual HRESULT Unlock(void * first, DWORD first_size, void * second, DWORD second_size) = 0;
};

using LPDIRECTSOUNDBUFFER = IDirectSoundBuffer *;

class IDirectSound
{
	public:
		virtual ~IDirectSound(void) = default;
		virtual HRESULT CreateSoundBuffer(DSBUFFERDESC const * descriptor,
			LPDIRECTSOUNDBUFFER * buffer, IUnknown *) = 0;
		virtual HRESULT GetCaps(DSCAPS * caps) = 0;
		virtual ULONG Release(void) = 0;
		virtual HRESULT SetCooperativeLevel(HWND, DWORD) = 0;
};

using LPDIRECTSOUND = IDirectSound *;

HRESULT DirectSoundCreate(GUID const *, LPDIRECTSOUND * sound, IUnknown *);
