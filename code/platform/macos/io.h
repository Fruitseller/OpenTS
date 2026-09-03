#pragma once
#include "wincompat.h"

inline long filelength(int descriptor)
{
	struct stat status = {};
	return(fstat(descriptor, &status) == 0 ? static_cast<long>(status.st_size) : -1);
}
