#pragma once
#include "wincompat.h"

struct TIMECAPS
{
	UINT wPeriodMin;
	UINT wPeriodMax;
};

constexpr MMRESULT TIMERR_NOERROR = 0;

inline MMRESULT timeGetDevCaps(TIMECAPS * caps, UINT size)
{
	if (caps == nullptr || size < sizeof(*caps)) {
		return TIMERR_NOCANDO;
	}
	caps->wPeriodMin = 1;
	caps->wPeriodMax = 1000;
	return TIMERR_NOERROR;
}
