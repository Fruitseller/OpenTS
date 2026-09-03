/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 ******************************************************************************/

// Pins SHAEngine to the FIPS 180-1 SHA-1 test vectors. The engine hashes mix
// digests, so it must produce identical bytes on every build architecture.

#include "sha.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::string Hex(unsigned char const * digest, int length)
{
	std::string result;
	char byte[3];
	for (int index = 0; index < length; index++) {
		std::snprintf(byte, sizeof(byte), "%02X", digest[index]);
		result += byte;
	}
	return result;
}

bool Check(char const * name, void const * data, int length, char const * expected)
{
	SHAEngine engine;
	engine.Hash(data, length);
	unsigned char digest[20];
	int const size = engine.Result(digest);
	std::string const actual = Hex(digest, 20);
	if (size != 20 || actual != expected) {
		std::fprintf(stderr, "FAIL %s: size=%d digest=%s expected=%s\n",
			name, size, actual.c_str(), expected);
		return false;
	}
	std::printf("PASS %s\n", name);
	return true;
}

} // namespace

int main()
{
	bool ok = true;

	if (SHAEngine::Digest_Size() != 20) {
		std::fprintf(stderr, "FAIL digest size %d != 20\n", SHAEngine::Digest_Size());
		ok = false;
	}

	ok &= Check("abc", "abc", 3, "A9993E364706816ABA3E25717850C26C9CD0D89D");

	char const two_blocks[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	ok &= Check("two-block", two_blocks, static_cast<int>(std::strlen(two_blocks)),
		"84983E441C3BD26EBAAE4AA1F95129E5E54670F1");

	std::vector<char> const million(1000000, 'a');
	ok &= Check("million-a", million.data(), static_cast<int>(million.size()),
		"34AA973CD4C4DAA4F61EEB2BDBAD27316534016F");

	// Incremental hashing across uneven chunk sizes must match one-shot hashing.
	{
		SHAEngine engine;
		char const * text = two_blocks;
		int remaining = static_cast<int>(std::strlen(two_blocks));
		int const chunks[] = {1, 7, 13, 64};
		int cursor = 0;
		int which = 0;
		while (remaining > 0) {
			int step = chunks[which++ % 4];
			if (step > remaining) step = remaining;
			engine.Hash(text + cursor, step);
			cursor += step;
			remaining -= step;
		}
		unsigned char digest[20];
		engine.Result(digest);
		if (Hex(digest, 20) != "84983E441C3BD26EBAAE4AA1F95129E5E54670F1") {
			std::fprintf(stderr, "FAIL incremental digest=%s\n", Hex(digest, 20).c_str());
			ok = false;
		} else {
			std::printf("PASS incremental\n");
		}
	}

	return ok ? 0 : 1;
}
