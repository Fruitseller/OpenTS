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
#include <vector>


namespace {

bool Check_Golden(unsigned char const * source, int source_size,
		unsigned char const * expected, int expected_size)
{
	std::vector<unsigned char> compressed(source_size * 2 + 16);
	int const compressed_size = LCW_Comp(source, compressed.data(), source_size);
	return(compressed_size == expected_size
		&& std::memcmp(compressed.data(), expected, expected_size) == 0);
}


bool Check_Command_Forms(void)
{
	unsigned char const literals[] = {1, 2, 3};
	unsigned char const literals_expected[] = {0x83, 1, 2, 3, 0x80};
	unsigned char const short_copy[] = {'A', 'B', 'C', 'A', 'B', 'C'};
	unsigned char const short_copy_expected[] = {0x83, 'A', 'B', 'C', 0x00, 0x03, 0x80};
	unsigned char const medium_copy[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K'
	};
	unsigned char const medium_copy_expected[] = {
		0x8B, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
		0xC8, 0x00, 0x00, 0x80
	};
	unsigned char repeated[66];
	std::memset(repeated, 0x5A, sizeof(repeated));
	unsigned char const repeated_expected[] = {0x81, 0x5A, 0xFE, 0x41, 0x00, 0x5A, 0x80};

	return(Check_Golden(literals, sizeof(literals), literals_expected, sizeof(literals_expected))
		&& Check_Golden(short_copy, sizeof(short_copy), short_copy_expected, sizeof(short_copy_expected))
		&& Check_Golden(medium_copy, sizeof(medium_copy), medium_copy_expected, sizeof(medium_copy_expected))
		&& Check_Golden(repeated, sizeof(repeated), repeated_expected, sizeof(repeated_expected)));
}


bool Check_Round_Trip(void)
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
	return(compressed_size > 0
		&& compressed_size <= static_cast<int>(sizeof(compressed))
		&& restored_size == SIZE
		&& std::memcmp(source, restored, SIZE) == 0);
}

} // namespace


int main(void)
{
	bool const commands = Check_Command_Forms();
	bool const round_trip = Check_Round_Trip();
	std::printf("%-52s %s\n", "LCW command byte parity", commands ? "ok" : "FAILED");
	std::printf("%-52s %s\n", "LCW round trip", round_trip ? "ok" : "FAILED");
	return(commands && round_trip ? 0 : 1);
}
