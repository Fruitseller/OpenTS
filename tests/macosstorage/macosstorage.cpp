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

	std::wstring Widen(std::string const & text)
	{
		return(std::wstring(text.begin(), text.end()));
	}

	std::vector<std::uint8_t> Pattern(std::size_t size, std::uint8_t seed)
	{
		std::vector<std::uint8_t> result(size);
		for (std::size_t index = 0; index < size; ++index) result[index] = static_cast<std::uint8_t>(seed + index * 37);
		return(result);
	}

	bool Write_Stream(IStorage * storage, wchar_t const * name, std::vector<std::uint8_t> const & data)
	{
		IStreamPtr stream;
		if (FAILED(storage->CreateStream(name, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stream))) return(false);
		ULONG written = 0;
		return(SUCCEEDED(stream->Write(data.data(), static_cast<ULONG>(data.size()), &written)) && written == data.size());
	}

	bool Read_Stream(IStorage * storage, wchar_t const * name, std::vector<std::uint8_t> const & expected)
	{
		IStreamPtr stream;
		if (FAILED(storage->OpenStream(name, nullptr, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &stream))) return(false);
		std::vector<std::uint8_t> actual(expected.size());
		ULONG read = 0;
		if (FAILED(stream->Read(actual.data(), static_cast<ULONG>(actual.size()), &read)) || read != actual.size()) return(false);
		std::uint8_t extra = 0;
		return(stream->Read(&extra, 1, &read) == S_FALSE && read == 0 && actual == expected);
	}

	bool Write_Properties(IStorage * storage)
	{
		IPropertySetStoragePtr sets;
		if (FAILED(storage->QueryInterface(IID_IPropertySetStorage, reinterpret_cast<void **>(&sets)))) return(false);
		IPropertyStoragePtr properties;
		if (FAILED(sets->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT,
			STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &properties))) return(false);
		std::array<PROPSPEC, 3> specifications = {};
		for (PROPSPEC & specification : specifications) specification.ulKind = PRSPEC_PROPID;
		specifications[0].propid = 2;
		specifications[1].propid = 9;
		specifications[2].propid = 13;
		std::array<PROPVARIANT, 3> values = {};
		values[0].vt = VT_LPWSTR;
		values[0].pwszVal = const_cast<wchar_t *>(L"Mac save \u2603");
		values[1].vt = VT_I4;
		values[1].lVal = 0x1020304;
		values[2].vt = VT_FILETIME;
		values[2].filetime = {0x89ABCDEF, 0x01234567};
		return(SUCCEEDED(properties->WriteMultiple(values.size(), specifications.data(), values.data(), PID_FIRST_USABLE)));
	}

	bool Read_Properties(IStorage * storage)
	{
		IPropertySetStoragePtr sets;
		if (FAILED(storage->QueryInterface(IID_IPropertySetStorage, reinterpret_cast<void **>(&sets)))) return(false);
		IPropertyStoragePtr properties;
		if (FAILED(sets->Open(FMTID_SummaryInformation, STGM_READ | STGM_SHARE_EXCLUSIVE, &properties))) return(false);
		std::array<PROPSPEC, 3> specifications = {};
		for (PROPSPEC & specification : specifications) specification.ulKind = PRSPEC_PROPID;
		specifications[0].propid = 2;
		specifications[1].propid = 9;
		specifications[2].propid = 13;
		std::array<PROPVARIANT, 3> values = {};
		HRESULT const result = properties->ReadMultiple(values.size(), specifications.data(), values.data());
		bool const valid = SUCCEEDED(result) && values[0].vt == VT_LPWSTR && values[0].pwszVal &&
			std::wstring(values[0].pwszVal) == L"Mac save \u2603" && values[1].vt == VT_I4 &&
			values[1].lVal == 0x1020304 && values[2].vt == VT_FILETIME &&
			values[2].filetime.dwLowDateTime == 0x89ABCDEF && values[2].filetime.dwHighDateTime == 0x01234567;
		for (PROPVARIANT & value : values) PropVariantClear(&value);
		return(valid);
	}
}


int main(int argc, char ** argv)
{
	bool const keep_fixture = argc == 2;
	std::filesystem::path const fixture = keep_fixture ? std::filesystem::path(argv[1]) :
		std::filesystem::temp_directory_path() / ("opents-storage-" + std::to_string(getpid()) + ".sav");
	std::wstring const path = Widen(fixture.string());
	std::vector<std::uint8_t> const small = Pattern(319, 7);
	std::vector<std::uint8_t> const large = Pattern(11037, 19);

	IStoragePtr storage;
	Check(SUCCEEDED(StgCreateDocfile(path.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &storage)),
		"StgCreateDocfile creates a writable compound file");
	Check(storage && Write_Stream(storage, L"SMALL", small), "a short stream is accepted for mini-stream storage");
	Check(storage && Write_Stream(storage, L"CONTENTS", large), "a regular stream crosses several FAT sectors");
	Check(storage && Write_Properties(storage), "summary information accepts strings, integers, and times");
	Check(storage && SUCCEEDED(storage->Commit(0)), "committing the root writes the compound file");
	storage.Release();

	std::ifstream input(fixture, std::ios::binary);
	std::array<std::uint8_t, 8> signature = {};
	input.read(reinterpret_cast<char *>(signature.data()), signature.size());
	std::array<std::uint8_t, 8> const expected_signature = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
	Check(signature == expected_signature, "the saved file has the OLE compound-file signature");

	Check(SUCCEEDED(StgOpenStorage(path.c_str(), nullptr, STGM_READ | STGM_SHARE_DENY_WRITE, nullptr, 0, &storage)),
		"StgOpenStorage reopens the committed file");
	Check(storage && Read_Stream(storage, L"SMALL", small), "the mini stream round-trips without padding bytes");
	Check(storage && Read_Stream(storage, L"CONTENTS", large), "the regular stream round-trips across its FAT chain");
	Check(storage && Read_Properties(storage), "the property-set stream round-trips all supported value types");
	IStreamPtr missing;
	Check(storage && storage->OpenStream(L"MISSING", nullptr, STGM_READ, 0, &missing) == STG_E_FILENOTFOUND,
		"opening an absent stream returns STG_E_FILENOTFOUND");
	storage.Release();

	if (!keep_fixture) {
		std::error_code error;
		std::filesystem::remove(fixture, error);
	}
	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
