#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <malloc/malloc.h>

inline void * _aligned_malloc(std::size_t size, std::size_t alignment)
{
	void * result = nullptr;
	return(posix_memalign(&result, alignment, size) == 0 ? result : nullptr);
}

inline void _aligned_free(void * pointer)
{
	std::free(pointer);
}

inline void * _aligned_realloc(void * pointer, std::size_t size, std::size_t alignment)
{
	if (!pointer) return(_aligned_malloc(size, alignment));
	if (size == 0) {
		_aligned_free(pointer);
		return(nullptr);
	}

	void * replacement = _aligned_malloc(size, alignment);
	if (!replacement) return(nullptr);
	std::memcpy(replacement, pointer, std::min(size, malloc_size(pointer)));
	_aligned_free(pointer);
	return(replacement);
}
