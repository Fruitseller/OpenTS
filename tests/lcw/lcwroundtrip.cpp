/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "lcw.h"

#include <cstdio>
#include <cstring>


int main(void)
{
	constexpr int SIZE = 8192;
	unsigned char source[SIZE];
	unsigned char compressed[SIZE + (SIZE + 62) / 63 + 1];
	unsigned char restored[SIZE];

	unsigned int state = 0x12345678u;
	for (int index = 0; index < SIZE; ++index) {
		state = state * 1664525u + 1013904223u;
		source[index] = static_cast<unsigned char>(state >> 24);
	}

	int const compressed_size = LCW_Comp(source, compressed, SIZE);
	unsigned int const restored_size = LCW_Uncomp(compressed, restored, SIZE);
	bool const matches = compressed_size > 0
		&& compressed_size <= static_cast<int>(sizeof(compressed))
		&& restored_size == SIZE
		&& std::memcmp(source, restored, SIZE) == 0;

	std::printf("%-52s %s\n", "LCW x64 fallback round trip", matches ? "ok" : "FAILED");
	return(matches ? 0 : 1);
}
