/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace
{
	int Failures = 0;

	void Check(bool condition, char const * what)
	{
		std::printf("%-68s %s\n", what, condition ? "ok" : "FAILED");
		if (!condition) Failures++;
	}

	void Write_U16(std::vector<std::uint8_t> & data, std::size_t offset, std::uint16_t value)
	{
		data[offset] = static_cast<std::uint8_t>(value);
		data[offset + 1] = static_cast<std::uint8_t>(value >> 8);
	}

	void Write_U32(std::vector<std::uint8_t> & data, std::size_t offset, std::uint32_t value)
	{
		data[offset] = static_cast<std::uint8_t>(value);
		data[offset + 1] = static_cast<std::uint8_t>(value >> 8);
		data[offset + 2] = static_cast<std::uint8_t>(value >> 16);
		data[offset + 3] = static_cast<std::uint8_t>(value >> 24);
	}

	std::vector<std::uint8_t> Make_String_Resource_Image(bool pe32_plus = false)
	{
		std::vector<std::uint8_t> image(0x400);
		image[0] = 'M';
		image[1] = 'Z';
		Write_U32(image, 0x3c, 0x80);

		image[0x80] = 'P';
		image[0x81] = 'E';
		Write_U16(image, 0x84, 0x14c);
		Write_U16(image, 0x86, 1);
		std::uint16_t const optional_size = pe32_plus ? 0xf0 : 0xe0;
		Write_U16(image, 0x94, optional_size);

		std::size_t const optional = 0x98;
		Write_U16(image, optional, pe32_plus ? 0x20b : 0x10b);
		std::size_t const directory_count = optional + (pe32_plus ? 108 : 92);
		std::size_t const resource_directory = optional + (pe32_plus ? 128 : 112);
		Write_U32(image, directory_count, 16);
		Write_U32(image, resource_directory, 0x1000);
		Write_U32(image, resource_directory + 4, 0x100);

		std::size_t const section = optional + optional_size;
		std::array<char, 8> const name = {'.', 'r', 's', 'r', 'c', 0, 0, 0};
		std::copy(name.begin(), name.end(), image.begin() + section);
		Write_U32(image, section + 8, 0x100);
		Write_U32(image, section + 12, 0x1000);
		Write_U32(image, section + 16, 0x200);
		Write_U32(image, section + 20, 0x200);

		std::size_t const resource = 0x200;
		Write_U16(image, resource + 14, 1);
		Write_U32(image, resource + 16, 6);
		Write_U32(image, resource + 20, 0x80000018);

		Write_U16(image, resource + 0x18 + 14, 1);
		Write_U32(image, resource + 0x28, 1);
		Write_U32(image, resource + 0x2c, 0x80000030);

		Write_U16(image, resource + 0x30 + 14, 1);
		Write_U32(image, resource + 0x40, 0x409);
		Write_U32(image, resource + 0x44, 0x48);

		constexpr std::size_t string_data = 0x58;
		Write_U32(image, resource + 0x48, 0x1000 + string_data);
		Write_U32(image, resource + 0x4c, 42);

		std::size_t cursor = resource + string_data;
		Write_U16(image, cursor, 0);
		cursor += 2;
		Write_U16(image, cursor, 0);
		cursor += 2;
		Write_U16(image, cursor, 6);
		cursor += 2;
		for (std::uint16_t character : {0x0043, 0x0061, 0x0066, 0x00e9, 0x0020, 0x20ac}) {
			Write_U16(image, cursor, character);
			cursor += 2;
		}
		for (int index = 3; index < 16; ++index) {
			Write_U16(image, cursor, 0);
			cursor += 2;
		}
		return(image);
	}

	bool Write_File(std::filesystem::path const & path, std::vector<std::uint8_t> const & data)
	{
		std::ofstream output(path, std::ios::binary);
		return(output.write(reinterpret_cast<char const *>(data.data()), data.size()).good());
	}
}


int main(int argc, char ** argv)
{
	if (argc == 2) {
		HMODULE module = LoadLibrary(argv[1]);
		char text[256] = {};
		Check(module != nullptr, "the supplied PE resource module loads");
		Check(module && LoadString(module, 1, text, sizeof(text)) > 0,
			"the supplied module exposes its first string");
		if (module) FreeLibrary(module);
		std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
		return(Failures == 0 ? 0 : 1);
	}

	std::filesystem::path const fixture = std::filesystem::temp_directory_path() /
		("opents-peresource-" + std::to_string(getpid()) + ".dll");
	std::vector<std::uint8_t> image = Make_String_Resource_Image();
	Check(Write_File(fixture, image), "a synthetic PE resource fixture can be written");

	HMODULE module = LoadLibrary(fixture.c_str());
	Check(module != nullptr, "a PE32 resource-only module loads");

	char text[32] = {};
	std::array<unsigned char, 6> const expected = {'C', 'a', 'f', 0xe9, ' ', 0x80};
	int const loaded = LoadString(module, 2, text, sizeof(text));
	Check(loaded == static_cast<int>(expected.size()), "LoadString returns the converted byte count");
	Check(std::equal(expected.begin(), expected.end(), reinterpret_cast<unsigned char *>(text)),
		"LoadString converts UTF-16 text to Windows-1252");
	Check(text[loaded] == '\0', "LoadString terminates the destination string");

	char short_text[5] = {};
	Check(LoadString(module, 2, short_text, sizeof(short_text)) == 4 && short_text[4] == '\0',
		"LoadString truncates to leave room for the terminator");
	Check(LoadString(module, 17, text, sizeof(text)) == 0 && text[0] == '\0',
		"a missing string returns zero and an empty destination");

	HRSRC resource = FindResource(module, MAKEINTRESOURCE(1), RT_STRING);
	Check(resource != nullptr, "FindResource locates an integer resource");
	HGLOBAL loaded_resource = LoadResource(module, resource);
	Check(loaded_resource == resource, "LoadResource validates and returns the resource handle");
	Check(LockResource(loaded_resource) != nullptr, "LockResource exposes the resource bytes");

	char module_path[MAX_PATH] = {};
	Check(GetModuleFileName(module, module_path, sizeof(module_path)) > 0 &&
		std::filesystem::path(module_path).filename() == fixture.filename(),
		"GetModuleFileName returns the resource module path");
	Check(FreeLibrary(module) == TRUE, "FreeLibrary releases a loaded module");
	Check(FreeLibrary(module) == FALSE, "FreeLibrary rejects a stale module handle");

	image = Make_String_Resource_Image(true);
	Check(Write_File(fixture, image), "the synthetic PE32+ fixture can be written");
	module = LoadLibrary(fixture.c_str());
	Check(module != nullptr && LoadString(module, 2, text, sizeof(text)) == 6,
		"a PE32+ resource module uses the same resource layout");
	if (module) FreeLibrary(module);

	image[0] = 0;
	Check(Write_File(fixture, image), "the malformed fixture can be written");
	Check(LoadLibrary(fixture.c_str()) == nullptr, "a malformed PE image is rejected");

	std::error_code error;
	std::filesystem::remove(fixture, error);
	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
