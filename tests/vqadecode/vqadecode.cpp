/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include <cstddef>
#include <cstdio>
#include <cstring>


extern "C" unsigned short * HicolorTable;

extern "C" void __cdecl UnVQ1_C1_TABLE_ALT(unsigned char * codebook,
	unsigned char * pointers, unsigned char * buffer, size_t blocksperrow,
	size_t numrows, size_t bufwidth);

unsigned short * HicolorTable = nullptr;


namespace {

constexpr unsigned short GUARD = 0xA5A5u;


bool Check_Row(unsigned short const * buffer, int row, unsigned short const * expected)
{
	return(std::memcmp(buffer + row * 4, expected, 4 * sizeof(unsigned short)) == 0);
}


bool Check_Guard_Row(unsigned short const * buffer, int row)
{
	unsigned short const expected[4] = {GUARD, GUARD, GUARD, GUARD};
	return(Check_Row(buffer, row, expected));
}


bool Check_Solid(void)
{
	unsigned short table[2] = {0, 0x1234u};
	unsigned char codebook[32] = {};
	unsigned char pointers[2] = {1, 0x80u};
	unsigned short buffer[16];
	unsigned short const expected[4] = {0x1234u, 0x1234u, 0x1234u, 0x1234u};

	HicolorTable = table;
	for (unsigned short & pixel : buffer) {
		pixel = GUARD;
	}
	UnVQ1_C1_TABLE_ALT(codebook, pointers, reinterpret_cast<unsigned char *>(buffer), 1, 1, 4);

	return(Check_Row(buffer, 0, expected) && Check_Guard_Row(buffer, 1)
		&& Check_Row(buffer, 2, expected) && Check_Guard_Row(buffer, 3));
}


bool Check_Codebook(void)
{
	unsigned short codebook[16];
	unsigned char pointers[2] = {0, 0};
	unsigned short buffer[16];

	for (int index = 0; index < 16; ++index) {
		codebook[index] = static_cast<unsigned short>(index + 1);
		buffer[index] = GUARD;
	}
	UnVQ1_C1_TABLE_ALT(reinterpret_cast<unsigned char *>(codebook), pointers,
		reinterpret_cast<unsigned char *>(buffer), 1, 1, 4);

	return(Check_Row(buffer, 0, codebook) && Check_Guard_Row(buffer, 1)
		&& Check_Row(buffer, 2, codebook + 8) && Check_Guard_Row(buffer, 3));
}

}	// namespace


int main(void)
{
	bool const solid = Check_Solid();
	bool const codebook = Check_Codebook();
	std::printf("%-52s %s\n", "VQA 4x2 solid row parity", solid ? "ok" : "FAILED");
	std::printf("%-52s %s\n", "VQA 4x2 codebook row parity", codebook ? "ok" : "FAILED");
	return(solid && codebook ? 0 : 1);
}
