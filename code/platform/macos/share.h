#pragma once

#define _SH_DENYNO 0

inline int _sopen(char const * path, int flags, int, int mode = 0)
{
	return(open(path, flags, mode));
}
