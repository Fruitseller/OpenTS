/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#if defined(OPENTS_MACOS)

#include "wincompat.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <optional>
#include <unordered_set>

extern "C" int _NSGetExecutablePath(char * buffer, std::uint32_t * size);

namespace
{
	constexpr std::uint32_t RESOURCE_DIRECTORY_INDEX = 2;
	constexpr std::uint32_t RESOURCE_DIRECTORY_FLAG = 0x80000000u;

	struct ResourceKey {
		bool IsInteger = true;
		std::uint32_t Integer = 0;
		std::string Name;
	};

	struct ResourceRecord {
		ResourceKey Type;
		ResourceKey Name;
		std::size_t Offset = 0;
		std::size_t Size = 0;
	};

	struct ResourceModule {
		std::filesystem::path Path;
		std::vector<std::uint8_t> Image;
		std::vector<ResourceRecord> Resources;
	};

	std::mutex LoadedModulesMutex;

	// Global constructors fetch strings before this translation unit's globals
	// are built, so the registry is constructed on first use.
	std::unordered_set<ResourceModule *> & Loaded_Modules()
	{
		static std::unordered_set<ResourceModule *> modules;
		return(modules);
	}

	bool Range_Is_Valid(std::size_t offset, std::size_t size, std::size_t limit)
	{
		return(offset <= limit && size <= limit - offset);
	}

	std::optional<std::uint16_t> Read_U16(std::vector<std::uint8_t> const & data, std::size_t offset)
	{
		if (!Range_Is_Valid(offset, 2, data.size())) return(std::nullopt);
		return(static_cast<std::uint16_t>(data[offset]) |
			(static_cast<std::uint16_t>(data[offset + 1]) << 8));
	}

	std::optional<std::uint32_t> Read_U32(std::vector<std::uint8_t> const & data, std::size_t offset)
	{
		if (!Range_Is_Valid(offset, 4, data.size())) return(std::nullopt);
		return(static_cast<std::uint32_t>(data[offset]) |
			(static_cast<std::uint32_t>(data[offset + 1]) << 8) |
			(static_cast<std::uint32_t>(data[offset + 2]) << 16) |
			(static_cast<std::uint32_t>(data[offset + 3]) << 24));
	}

	class PEResourceReader {
	public:
		explicit PEResourceReader(std::vector<std::uint8_t> const & image) : Image(image) {}

		bool Read(std::vector<ResourceRecord> & resources)
		{
			if (!Read_Headers()) return(false);

			std::vector<ResourceKey> path;
			std::unordered_set<std::uint32_t> active_directories;
			return(Read_Directory(0, path, active_directories, resources) && !resources.empty());
		}

	private:
		struct Section {
			std::uint32_t VirtualAddress;
			std::uint32_t VirtualSize;
			std::uint32_t RawOffset;
			std::uint32_t RawSize;
		};

		bool Read_Headers()
		{
			if (Image.size() < 0x40 || Image[0] != 'M' || Image[1] != 'Z') return(false);

			auto const pe_offset = Read_U32(Image, 0x3c);
			if (!pe_offset || !Range_Is_Valid(*pe_offset, 24, Image.size())) return(false);
			if (Image[*pe_offset] != 'P' || Image[*pe_offset + 1] != 'E' ||
				Image[*pe_offset + 2] != 0 || Image[*pe_offset + 3] != 0) return(false);

			auto const section_count = Read_U16(Image, *pe_offset + 6);
			auto const optional_size = Read_U16(Image, *pe_offset + 20);
			if (!section_count || !optional_size) return(false);

			std::size_t const optional_offset = *pe_offset + 24;
			if (!Range_Is_Valid(optional_offset, *optional_size, Image.size())) return(false);
			auto const magic = Read_U16(Image, optional_offset);
			if (!magic) return(false);

			std::size_t directory_offset;
			std::size_t directory_count_offset;
			if (*magic == 0x10b) {
				directory_offset = optional_offset + 96;
				directory_count_offset = optional_offset + 92;
			} else if (*magic == 0x20b) {
				directory_offset = optional_offset + 112;
				directory_count_offset = optional_offset + 108;
			} else {
				return(false);
			}

			auto const directory_count = Read_U32(Image, directory_count_offset);
			if (!directory_count || *directory_count <= RESOURCE_DIRECTORY_INDEX ||
				!Range_Is_Valid(directory_offset, (*directory_count) * 8ull, optional_offset + *optional_size)) {
				return(false);
			}

			auto const resource_rva = Read_U32(Image, directory_offset + RESOURCE_DIRECTORY_INDEX * 8);
			auto const resource_size = Read_U32(Image, directory_offset + RESOURCE_DIRECTORY_INDEX * 8 + 4);
			if (!resource_rva || !resource_size || *resource_rva == 0 || *resource_size < 16) return(false);
			ResourceRVA = *resource_rva;
			ResourceSize = *resource_size;

			std::size_t const section_offset = optional_offset + *optional_size;
			if (!Range_Is_Valid(section_offset, static_cast<std::size_t>(*section_count) * 40, Image.size())) return(false);
			for (std::size_t index = 0; index < *section_count; ++index) {
				std::size_t const offset = section_offset + index * 40;
				auto const virtual_size = Read_U32(Image, offset + 8);
				auto const virtual_address = Read_U32(Image, offset + 12);
				auto const raw_size = Read_U32(Image, offset + 16);
				auto const raw_offset = Read_U32(Image, offset + 20);
				if (!virtual_size || !virtual_address || !raw_size || !raw_offset) return(false);
				Sections.push_back({*virtual_address, *virtual_size, *raw_offset, *raw_size});
			}

			auto const root_offset = RVA_To_Offset(ResourceRVA, 16);
			if (!root_offset) return(false);
			ResourceOffset = *root_offset;
			return(Range_Is_Valid(ResourceOffset, ResourceSize, Image.size()));
		}

		std::optional<std::size_t> RVA_To_Offset(std::uint32_t rva, std::size_t size) const
		{
			for (Section const & section : Sections) {
				std::uint64_t const extent = std::max(section.VirtualSize, section.RawSize);
				if (rva < section.VirtualAddress || static_cast<std::uint64_t>(rva) >= section.VirtualAddress + extent) continue;

				std::uint64_t const delta = rva - section.VirtualAddress;
				if (delta > section.RawSize || size > section.RawSize - delta) return(std::nullopt);
				std::uint64_t const offset = section.RawOffset + delta;
				if (offset > std::numeric_limits<std::size_t>::max() ||
					!Range_Is_Valid(static_cast<std::size_t>(offset), size, Image.size())) return(std::nullopt);
				return(static_cast<std::size_t>(offset));
			}
			return(std::nullopt);
		}

		std::optional<std::size_t> Resource_Offset(std::uint32_t relative_offset, std::size_t size) const
		{
			if (relative_offset > ResourceSize || size > ResourceSize - relative_offset) return(std::nullopt);
			if (relative_offset > std::numeric_limits<std::uint32_t>::max() - ResourceRVA) return(std::nullopt);
			return(RVA_To_Offset(ResourceRVA + relative_offset, size));
		}

		std::optional<ResourceKey> Read_Key(std::uint32_t value) const
		{
			ResourceKey key;
			if ((value & RESOURCE_DIRECTORY_FLAG) == 0) {
				key.Integer = value;
				return(key);
			}

			std::uint32_t const relative_offset = value & ~RESOURCE_DIRECTORY_FLAG;
			auto const name_offset = Resource_Offset(relative_offset, 2);
			if (!name_offset) return(std::nullopt);
			auto const length = Read_U16(Image, *name_offset);
			if (!length || relative_offset > std::numeric_limits<std::uint32_t>::max() - 2 ||
				!Resource_Offset(relative_offset + 2, static_cast<std::size_t>(*length) * 2)) return(std::nullopt);

			key.IsInteger = false;
			key.Name.reserve(*length);
			for (std::size_t index = 0; index < *length; ++index) {
				auto const character = Read_U16(Image, *name_offset + 2 + index * 2);
				if (!character) return(std::nullopt);
				Append_UTF8(key.Name, *character);
			}
			return(key);
		}

		static void Append_UTF8(std::string & text, std::uint16_t character)
		{
			if (character < 0x80) {
				text.push_back(static_cast<char>(character));
			} else if (character < 0x800) {
				text.push_back(static_cast<char>(0xc0 | character >> 6));
				text.push_back(static_cast<char>(0x80 | (character & 0x3f)));
			} else {
				text.push_back(static_cast<char>(0xe0 | character >> 12));
				text.push_back(static_cast<char>(0x80 | ((character >> 6) & 0x3f)));
				text.push_back(static_cast<char>(0x80 | (character & 0x3f)));
			}
		}

		bool Read_Directory(std::uint32_t relative_offset, std::vector<ResourceKey> & path,
			std::unordered_set<std::uint32_t> & active_directories, std::vector<ResourceRecord> & resources)
		{
			if (path.size() > 8 || !active_directories.insert(relative_offset).second) return(false);
			auto const directory_offset = Resource_Offset(relative_offset, 16);
			if (!directory_offset) return(false);

			auto const named_count = Read_U16(Image, *directory_offset + 12);
			auto const integer_count = Read_U16(Image, *directory_offset + 14);
			if (!named_count || !integer_count) return(false);
			std::size_t const entry_count = static_cast<std::size_t>(*named_count) + *integer_count;
			if (entry_count > ResourceSize / 8) return(false);
			if (relative_offset > std::numeric_limits<std::uint32_t>::max() - 16) return(false);
			auto const entries_offset = Resource_Offset(relative_offset + 16, entry_count * 8);
			if (!entries_offset) return(false);

			for (std::size_t index = 0; index < entry_count; ++index) {
				auto const key_value = Read_U32(Image, *entries_offset + index * 8);
				auto const child_value = Read_U32(Image, *entries_offset + index * 8 + 4);
				if (!key_value || !child_value) return(false);
				auto const key = Read_Key(*key_value);
				if (!key) return(false);
				path.push_back(*key);
				if (*child_value & RESOURCE_DIRECTORY_FLAG) {
					if (!Read_Directory(*child_value & ~RESOURCE_DIRECTORY_FLAG, path, active_directories, resources)) return(false);
				} else {
					if (path.size() < 2 || !Read_Data(*child_value, path, resources)) return(false);
				}
				path.pop_back();
			}

			active_directories.erase(relative_offset);
			return(true);
		}

		bool Read_Data(std::uint32_t relative_offset, std::vector<ResourceKey> const & path,
			std::vector<ResourceRecord> & resources) const
		{
			auto const entry_offset = Resource_Offset(relative_offset, 16);
			if (!entry_offset) return(false);
			auto const data_rva = Read_U32(Image, *entry_offset);
			auto const data_size = Read_U32(Image, *entry_offset + 4);
			if (!data_rva || !data_size) return(false);
			auto const data_offset = RVA_To_Offset(*data_rva, *data_size);
			if (!data_offset) return(false);
			resources.push_back({path[0], path[1], *data_offset, *data_size});
			return(true);
		}

		std::vector<std::uint8_t> const & Image;
		std::vector<Section> Sections;
		std::uint32_t ResourceRVA = 0;
		std::uint32_t ResourceSize = 0;
		std::size_t ResourceOffset = 0;
	};

	std::optional<std::filesystem::path> Executable_Path()
	{
		std::uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		if (size == 0) return(std::nullopt);
		std::vector<char> path(size);
		if (_NSGetExecutablePath(path.data(), &size) != 0) return(std::nullopt);
		std::error_code error;
		auto canonical = std::filesystem::weakly_canonical(path.data(), error);
		return(error ? std::filesystem::path(path.data()) : canonical);
	}

	std::optional<std::filesystem::path> Resolve_Path(char const * path)
	{
		if (!path || !*path) return(std::nullopt);
		std::filesystem::path candidate(path);
		std::error_code error;
		if (std::filesystem::is_regular_file(candidate, error)) return(std::filesystem::absolute(candidate, error));
		if (candidate.is_absolute()) return(std::nullopt);

		auto const executable = Executable_Path();
		if (!executable) return(std::nullopt);
		candidate = executable->parent_path() / candidate;
		if (std::filesystem::is_regular_file(candidate, error)) return(std::filesystem::absolute(candidate, error));
		return(std::nullopt);
	}

	std::unique_ptr<ResourceModule> Read_Module(std::filesystem::path const & path)
	{
		std::ifstream input(path, std::ios::binary | std::ios::ate);
		if (!input) return(nullptr);
		auto const length = input.tellg();
		if (length <= 0 || static_cast<std::uint64_t>(length) > std::numeric_limits<std::size_t>::max()) return(nullptr);

		auto module = std::make_unique<ResourceModule>();
		module->Path = path;
		module->Image.resize(static_cast<std::size_t>(length));
		input.seekg(0);
		if (!input.read(reinterpret_cast<char *>(module->Image.data()), length)) return(nullptr);
		if (!PEResourceReader(module->Image).Read(module->Resources)) return(nullptr);
		return(module);
	}

	ResourceModule * Find_Module(HMODULE handle)
	{
		auto * module = static_cast<ResourceModule *>(handle);
		std::lock_guard lock(LoadedModulesMutex);
		return(Loaded_Modules().contains(module) ? module : nullptr);
	}

	ResourceRecord * Find_Record(HRSRC handle)
	{
		std::lock_guard lock(LoadedModulesMutex);
		for (ResourceModule * module : Loaded_Modules()) {
			for (ResourceRecord & resource : module->Resources) {
				if (&resource == handle) return(&resource);
			}
		}
		return(nullptr);
	}

	bool Keys_Match(ResourceKey const & key, LPCSTR value)
	{
		std::uintptr_t const integer = reinterpret_cast<std::uintptr_t>(value);
		if (integer <= std::numeric_limits<WORD>::max()) return(key.IsInteger && key.Integer == integer);
		return(!key.IsInteger && value && key.Name == value);
	}

	char To_CP1252(std::uint16_t character)
	{
		if (character <= 0xff) return(static_cast<char>(character));
		constexpr std::array<std::uint16_t, 27> unicode = {
			0x20ac, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030,
			0x0160, 0x2039, 0x0152, 0x017d, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022,
			0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x017e, 0x0178
		};
		constexpr std::array<unsigned char, 27> encoded = {
			0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
			0x8a, 0x8b, 0x8c, 0x8e, 0x91, 0x92, 0x93, 0x94, 0x95,
			0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9e, 0x9f
		};
		for (std::size_t index = 0; index < unicode.size(); ++index) {
			if (unicode[index] == character) return(static_cast<char>(encoded[index]));
		}
		return('?');
	}
}

HMODULE LoadLibrary(LPCTSTR path)
{
	auto const resolved = Resolve_Path(path);
	if (!resolved) return(nullptr);
	auto module = Read_Module(*resolved);
	if (!module) return(nullptr);

	auto * result = module.release();
	{
		std::lock_guard lock(LoadedModulesMutex);
		Loaded_Modules().insert(result);
	}
	return(result);
}


BOOL FreeLibrary(HMODULE handle)
{
	auto * module = static_cast<ResourceModule *>(handle);
	{
		std::lock_guard lock(LoadedModulesMutex);
		if (Loaded_Modules().erase(module) == 0) return(FALSE);
	}
	delete module;
	return(TRUE);
}


FARPROC GetProcAddress(HMODULE, LPCSTR)
{
	return(nullptr);
}


HRSRC FindResource(HMODULE handle, LPCSTR name, LPCSTR type)
{
	ResourceModule * module = Find_Module(handle);
	if (!module) return(nullptr);
	for (ResourceRecord & resource : module->Resources) {
		if (Keys_Match(resource.Type, type) && Keys_Match(resource.Name, name)) return(&resource);
	}
	return(nullptr);
}


// The executable is not a PE module on macOS, so a template looked up through its
// instance handle can only come from a loaded resource library.
void const * OpenTSMacOS_Find_Dialog_Template(HMODULE preferred, LPCSTR name)
{
	std::lock_guard lock(LoadedModulesMutex);
	auto const find_in = [name](ResourceModule const * module) -> void const * {
		for (ResourceRecord const & resource : module->Resources) {
			if (Keys_Match(resource.Type, RT_DIALOG) && Keys_Match(resource.Name, name)) {
				return(module->Image.data() + resource.Offset);
			}
		}
		return(nullptr);
	};
	auto * const requested = static_cast<ResourceModule *>(preferred);
	if (requested != nullptr && Loaded_Modules().contains(requested)) {
		if (void const * found = find_in(requested)) return(found);
	}
	for (ResourceModule const * module : Loaded_Modules()) {
		if (module == requested) continue;
		if (void const * found = find_in(module)) return(found);
	}
	return(nullptr);
}


char OpenTSMacOS_To_CP1252(unsigned short character)
{
	return(To_CP1252(character));
}


HGLOBAL LoadResource(HMODULE, HRSRC resource)
{
	return(Find_Record(resource));
}


LPVOID LockResource(HGLOBAL handle)
{
	ResourceRecord * resource = Find_Record(handle);
	if (!resource) return(nullptr);
	std::lock_guard lock(LoadedModulesMutex);
	for (ResourceModule * module : Loaded_Modules()) {
		for (ResourceRecord & candidate : module->Resources) {
			if (&candidate == resource) return(module->Image.data() + resource->Offset);
		}
	}
	return(nullptr);
}


int LoadString(HINSTANCE handle, UINT identifier, LPSTR buffer, int size)
{
	if (!buffer || size <= 0) return(0);
	buffer[0] = '\0';
	HRSRC const handle_resource = FindResource(handle, MAKEINTRESOURCE(identifier / 16 + 1), RT_STRING);
	ResourceRecord * resource = Find_Record(handle_resource);
	if (!resource) return(0);

	ResourceModule * module = Find_Module(handle);
	if (!module) return(0);
	std::size_t offset = resource->Offset;
	std::size_t const end = offset + resource->Size;
	for (UINT index = 0; index <= identifier % 16; ++index) {
		auto const length = Read_U16(module->Image, offset);
		if (!length) return(0);
		offset += 2;
		if (!Range_Is_Valid(offset, static_cast<std::size_t>(*length) * 2, end)) return(0);
		if (index == identifier % 16) {
			int const copied = std::min<int>(*length, size - 1);
			for (int character = 0; character < copied; ++character) {
				auto const value = Read_U16(module->Image, offset + character * 2);
				if (!value) return(0);
				buffer[character] = To_CP1252(*value);
			}
			buffer[copied] = '\0';
			return(copied);
		}
		offset += static_cast<std::size_t>(*length) * 2;
	}
	return(0);
}


DWORD GetModuleFileName(HMODULE handle, LPSTR path, DWORD size)
{
	if (!path || size == 0) return(0);
	std::optional<std::filesystem::path> source;
	if (handle) {
		if (ResourceModule * module = Find_Module(handle)) source = module->Path;
	} else {
		source = Executable_Path();
	}
	if (!source) {
		path[0] = '\0';
		return(0);
	}

	std::string const text = source->string();
	std::size_t const copied = std::min<std::size_t>(text.size(), size - 1);
	std::memcpy(path, text.data(), copied);
	path[copied] = '\0';
	return(static_cast<DWORD>(copied));
}

#endif
