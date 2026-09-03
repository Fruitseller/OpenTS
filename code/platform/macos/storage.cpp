/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"

#include <array>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <optional>

namespace
{
	constexpr std::uint32_t FREE_SECTOR = 0xFFFFFFFFU;
	constexpr std::uint32_t END_OF_CHAIN = 0xFFFFFFFEU;
	constexpr std::uint32_t FAT_SECTOR = 0xFFFFFFFDU;
	constexpr std::uint32_t DIFAT_SECTOR = 0xFFFFFFFCU;
	constexpr std::size_t HEADER_SIZE = 512;
	constexpr std::size_t SECTOR_SIZE = 512;
	constexpr std::size_t MINI_SECTOR_SIZE = 64;
	constexpr std::size_t MINI_STREAM_CUTOFF = 4096;
	constexpr wchar_t SUMMARY_INFORMATION_NAME[] = L"\x0005SummaryInformation";

	using Bytes = std::vector<std::uint8_t>;

	struct CaseInsensitiveLess
	{
		bool operator()(std::wstring const & left, std::wstring const & right) const
		{
			auto const count = std::min(left.size(), right.size());
			for (std::size_t index = 0; index < count; ++index) {
				wchar_t const a = std::towupper(left[index]);
				wchar_t const b = std::towupper(right[index]);
				if (a != b) return(a < b);
			}
			return(left.size() < right.size());
		}
	};

	struct CompoundFileNameLess
	{
		bool operator()(std::wstring const & left, std::wstring const & right) const
		{
			if (left.size() != right.size()) return(left.size() < right.size());
			for (std::size_t index = 0; index < left.size(); ++index) {
				wchar_t const a = std::towupper(left[index]);
				wchar_t const b = std::towupper(right[index]);
				if (a != b) return(a < b);
			}
			return(false);
		}
	};

	struct PropertyValue
	{
		VARTYPE Type = VT_EMPTY;
		LONG Integer = 0;
		FILETIME Time = {};
		std::wstring String;
	};

	struct Document
	{
		std::mutex Mutex;
		std::string Path;
		bool Writable = false;
		bool Dirty = false;
		std::map<std::wstring, Bytes, CaseInsensitiveLess> Streams;
		std::map<PROPID, PropertyValue> SummaryProperties;
	};

	void Put16(Bytes & bytes, std::size_t offset, std::uint16_t value)
	{
		bytes[offset] = static_cast<std::uint8_t>(value);
		bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8);
	}

	void Put32(Bytes & bytes, std::size_t offset, std::uint32_t value)
	{
		for (int index = 0; index < 4; ++index) bytes[offset + index] = static_cast<std::uint8_t>(value >> (index * 8));
	}

	void Put64(Bytes & bytes, std::size_t offset, std::uint64_t value)
	{
		for (int index = 0; index < 8; ++index) bytes[offset + index] = static_cast<std::uint8_t>(value >> (index * 8));
	}

	std::optional<std::uint16_t> Get16(Bytes const & bytes, std::size_t offset)
	{
		if (offset > bytes.size() || bytes.size() - offset < 2) return(std::nullopt);
		return(static_cast<std::uint16_t>(bytes[offset]) |
			static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
	}

	std::optional<std::uint32_t> Get32(Bytes const & bytes, std::size_t offset)
	{
		if (offset > bytes.size() || bytes.size() - offset < 4) return(std::nullopt);
		return(static_cast<std::uint32_t>(bytes[offset]) |
			static_cast<std::uint32_t>(bytes[offset + 1]) << 8 |
			static_cast<std::uint32_t>(bytes[offset + 2]) << 16 |
			static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
	}

	std::optional<std::uint64_t> Get64(Bytes const & bytes, std::size_t offset)
	{
		auto const low = Get32(bytes, offset);
		auto const high = Get32(bytes, offset + 4);
		if (!low || !high) return(std::nullopt);
		return(static_cast<std::uint64_t>(*low) | static_cast<std::uint64_t>(*high) << 32);
	}

	std::string UTF8Path(WCHAR const * path)
	{
		std::string result;
		if (!path) return(result);
		for (; *path; ++path) {
			std::uint32_t const code = static_cast<std::uint32_t>(*path);
			if (code <= 0x7F) {
				result.push_back(static_cast<char>(code));
			} else if (code <= 0x7FF) {
				result.push_back(static_cast<char>(0xC0 | code >> 6));
				result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			} else if (code <= 0xFFFF) {
				result.push_back(static_cast<char>(0xE0 | code >> 12));
				result.push_back(static_cast<char>(0x80 | (code >> 6 & 0x3F)));
				result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			} else {
				result.push_back(static_cast<char>(0xF0 | code >> 18));
				result.push_back(static_cast<char>(0x80 | (code >> 12 & 0x3F)));
				result.push_back(static_cast<char>(0x80 | (code >> 6 & 0x3F)));
				result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
			}
		}
		std::replace(result.begin(), result.end(), '\\', '/');
		return(result);
	}

	void AppendUTF16(Bytes & bytes, std::wstring const & string, bool terminate)
	{
		for (wchar_t character : string) {
			std::uint32_t const code = static_cast<std::uint32_t>(character);
			if (code <= 0xFFFF) {
				std::size_t const offset = bytes.size();
				bytes.resize(offset + 2);
				Put16(bytes, offset, static_cast<std::uint16_t>(code));
			} else {
				std::uint32_t const value = code - 0x10000;
				std::size_t const offset = bytes.size();
				bytes.resize(offset + 4);
				Put16(bytes, offset, static_cast<std::uint16_t>(0xD800 | value >> 10));
				Put16(bytes, offset + 2, static_cast<std::uint16_t>(0xDC00 | (value & 0x3FF)));
			}
		}
		if (terminate) {
			bytes.push_back(0);
			bytes.push_back(0);
		}
	}

	std::wstring ReadUTF16(Bytes const & bytes, std::size_t offset, std::size_t units)
	{
		std::wstring result;
		for (std::size_t index = 0; index < units; ++index) {
			auto const first = Get16(bytes, offset + index * 2);
			if (!first || *first == 0) break;
			if (*first >= 0xD800 && *first <= 0xDBFF && index + 1 < units) {
				auto const second = Get16(bytes, offset + (++index) * 2);
				if (!second || *second < 0xDC00 || *second > 0xDFFF) return(std::wstring{});
				result.push_back(static_cast<wchar_t>(0x10000 + ((*first - 0xD800) << 10) + (*second - 0xDC00)));
			} else {
				result.push_back(static_cast<wchar_t>(*first));
			}
		}
		return(result);
	}

	void PutGUID(Bytes & bytes, std::size_t offset, GUID const & identifier)
	{
		Put32(bytes, offset, identifier.Data1);
		Put16(bytes, offset + 4, identifier.Data2);
		Put16(bytes, offset + 6, identifier.Data3);
		std::copy(std::begin(identifier.Data4), std::end(identifier.Data4), bytes.begin() + offset + 8);
	}

	std::optional<GUID> GetGUID(Bytes const & bytes, std::size_t offset)
	{
		auto const data1 = Get32(bytes, offset);
		auto const data2 = Get16(bytes, offset + 4);
		auto const data3 = Get16(bytes, offset + 6);
		if (!data1 || !data2 || !data3 || offset > bytes.size() || bytes.size() - offset < 16) return(std::nullopt);
		GUID result = {*data1, *data2, *data3, {}};
		std::copy_n(bytes.begin() + offset + 8, 8, result.Data4);
		return(result);
	}

	std::size_t Align4(std::size_t value)
	{
		return((value + 3) & ~std::size_t(3));
	}

	Bytes WritePropertySet(std::map<PROPID, PropertyValue> const & properties)
	{
		struct SerializedProperty { PROPID Identifier; Bytes Data; };
		std::vector<SerializedProperty> values;
		for (auto const & [identifier, value] : properties) {
			SerializedProperty property = {identifier, Bytes(4)};
			Put32(property.Data, 0, value.Type);
			switch (value.Type) {
				case VT_I4:
					property.Data.resize(8);
					Put32(property.Data, 4, static_cast<std::uint32_t>(value.Integer));
					break;
				case VT_FILETIME:
					property.Data.resize(12);
					Put32(property.Data, 4, value.Time.dwLowDateTime);
					Put32(property.Data, 8, value.Time.dwHighDateTime);
					break;
				case VT_LPWSTR: {
					Bytes encoded;
					AppendUTF16(encoded, value.String, true);
					property.Data.resize(8);
					Put32(property.Data, 4, static_cast<std::uint32_t>(encoded.size() / 2));
					property.Data.insert(property.Data.end(), encoded.begin(), encoded.end());
					property.Data.resize(Align4(property.Data.size()));
					break;
				}
				default:
					continue;
			}
			values.push_back(std::move(property));
		}

		std::size_t const section_offset = 48;
		std::size_t const value_offset = 8 + values.size() * 8;
		Bytes section(value_offset);
		Put32(section, 4, static_cast<std::uint32_t>(values.size()));
		std::size_t cursor = value_offset;
		for (std::size_t index = 0; index < values.size(); ++index) {
			Put32(section, 8 + index * 8, values[index].Identifier);
			Put32(section, 12 + index * 8, static_cast<std::uint32_t>(cursor));
			section.insert(section.end(), values[index].Data.begin(), values[index].Data.end());
			cursor += values[index].Data.size();
		}
		Put32(section, 0, static_cast<std::uint32_t>(section.size()));

		Bytes result(section_offset);
		Put16(result, 0, 0xFFFE);
		Put16(result, 2, 0);
		Put32(result, 4, 0x00020006);
		PutGUID(result, 8, GUID{});
		Put32(result, 24, 1);
		PutGUID(result, 28, FMTID_SummaryInformation);
		Put32(result, 44, static_cast<std::uint32_t>(section_offset));
		result.insert(result.end(), section.begin(), section.end());
		return(result);
	}

	bool ReadPropertySet(Bytes const & bytes, std::map<PROPID, PropertyValue> & properties)
	{
		auto const byte_order = Get16(bytes, 0);
		auto const set_count = Get32(bytes, 24);
		if (!byte_order || *byte_order != 0xFFFE || !set_count) return(false);
		for (std::uint32_t set = 0; set < *set_count; ++set) {
			std::size_t const descriptor = 28 + static_cast<std::size_t>(set) * 20;
			auto const identifier = GetGUID(bytes, descriptor);
			auto const section_offset = Get32(bytes, descriptor + 16);
			if (!identifier || !section_offset || *identifier != FMTID_SummaryInformation) continue;
			auto const section_size = Get32(bytes, *section_offset);
			auto const count = Get32(bytes, *section_offset + 4);
			if (!section_size || !count || *section_offset > bytes.size() ||
				*section_size > bytes.size() - *section_offset) return(false);
			for (std::uint32_t index = 0; index < *count; ++index) {
				auto const property_id = Get32(bytes, *section_offset + 8 + index * 8);
				auto const relative = Get32(bytes, *section_offset + 12 + index * 8);
				if (!property_id || !relative) return(false);
				std::size_t const offset = *section_offset + *relative;
				auto const type = Get32(bytes, offset);
				if (!type) return(false);
				PropertyValue value;
				value.Type = static_cast<VARTYPE>(*type);
				switch (value.Type) {
					case VT_I4: {
						auto const integer = Get32(bytes, offset + 4);
						if (!integer) return(false);
						value.Integer = static_cast<LONG>(*integer);
						break;
					}
					case VT_FILETIME: {
						auto const low = Get32(bytes, offset + 4);
						auto const high = Get32(bytes, offset + 8);
						if (!low || !high) return(false);
						value.Time = {*low, *high};
						break;
					}
					case VT_LPWSTR: {
						auto const count = Get32(bytes, offset + 4);
						if (!count || *count > (bytes.size() - std::min(bytes.size(), offset + 8)) / 2) return(false);
						value.String = ReadUTF16(bytes, offset + 8, *count);
						break;
					}
					default:
						continue;
				}
				properties[*property_id] = std::move(value);
			}
			return(true);
		}
		return(false);
	}

	struct DirectoryEntry
	{
		std::wstring Name;
		std::uint8_t Type = 0;
		std::uint8_t Color = 1;
		std::uint32_t Left = FREE_SECTOR;
		std::uint32_t Right = FREE_SECTOR;
		std::uint32_t Child = FREE_SECTOR;
		std::uint32_t Start = END_OF_CHAIN;
		std::uint64_t Size = 0;
	};

	void SetChain(std::vector<std::uint32_t> & fat, std::uint32_t start, std::size_t count)
	{
		for (std::size_t index = 0; index < count; ++index) {
			fat[start + index] = index + 1 == count ? END_OF_CHAIN : start + static_cast<std::uint32_t>(index + 1);
		}
	}

	std::uint32_t BuildDirectoryTree(std::vector<DirectoryEntry> & entries,
		std::vector<std::uint32_t> const & ordered, std::size_t begin, std::size_t end)
	{
		if (begin == end) return(FREE_SECTOR);
		std::size_t const middle = begin + (end - begin) / 2;
		std::uint32_t const entry = ordered[middle];
		entries[entry].Left = BuildDirectoryTree(entries, ordered, begin, middle);
		entries[entry].Right = BuildDirectoryTree(entries, ordered, middle + 1, end);
		return(entry);
	}

	void ColorDeepestDirectoryNodes(std::vector<DirectoryEntry> & entries, std::uint32_t entry,
		std::size_t depth, std::size_t maximum_depth)
	{
		if (entry == FREE_SECTOR) return;
		entries[entry].Color = depth == maximum_depth && depth != 0 ? 0 : 1;
		ColorDeepestDirectoryNodes(entries, entries[entry].Left, depth + 1, maximum_depth);
		ColorDeepestDirectoryNodes(entries, entries[entry].Right, depth + 1, maximum_depth);
	}

	std::size_t DirectoryDepth(std::vector<DirectoryEntry> const & entries, std::uint32_t entry)
	{
		if (entry == FREE_SECTOR) return(0);
		return(1 + std::max(DirectoryDepth(entries, entries[entry].Left),
			DirectoryDepth(entries, entries[entry].Right)));
	}

	bool WriteCompoundFile(Document const & document)
	{
		struct StreamPlan {
			std::wstring Name;
			Bytes const * Data;
			bool Mini;
			std::uint32_t Start = END_OF_CHAIN;
			std::size_t SectorCount = 0;
		};

		Bytes property_stream;
		std::map<std::wstring, Bytes, CaseInsensitiveLess> streams = document.Streams;
		if (!document.SummaryProperties.empty()) {
			property_stream = WritePropertySet(document.SummaryProperties);
			streams[SUMMARY_INFORMATION_NAME] = property_stream;
		}

		std::vector<StreamPlan> plans;
		Bytes mini_stream;
		std::vector<std::uint32_t> mini_fat;
		std::size_t regular_sector_count = 0;
		for (auto const & [name, data] : streams) {
			StreamPlan plan = {name, &data, !data.empty() && data.size() < MINI_STREAM_CUTOFF};
			if (plan.Mini) {
				plan.Start = static_cast<std::uint32_t>(mini_fat.size());
				plan.SectorCount = (data.size() + MINI_SECTOR_SIZE - 1) / MINI_SECTOR_SIZE;
				std::size_t const old_size = mini_stream.size();
				mini_stream.resize(old_size + plan.SectorCount * MINI_SECTOR_SIZE);
				std::copy(data.begin(), data.end(), mini_stream.begin() + old_size);
				std::size_t const mini_start = mini_fat.size();
				mini_fat.resize(mini_start + plan.SectorCount, FREE_SECTOR);
				for (std::size_t index = 0; index < plan.SectorCount; ++index) {
					mini_fat[mini_start + index] = index + 1 == plan.SectorCount ? END_OF_CHAIN :
						static_cast<std::uint32_t>(mini_start + index + 1);
				}
			} else if (!data.empty()) {
				plan.SectorCount = (data.size() + SECTOR_SIZE - 1) / SECTOR_SIZE;
				regular_sector_count += plan.SectorCount;
			}
			plans.push_back(plan);
		}

		std::size_t const mini_stream_sectors = (mini_stream.size() + SECTOR_SIZE - 1) / SECTOR_SIZE;
		std::size_t const mini_fat_sectors = (mini_fat.size() * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;
		std::size_t const directory_sectors = std::max<std::size_t>(1, (plans.size() + 1 + 3) / 4);
		std::size_t const base_sectors = regular_sector_count + mini_stream_sectors + mini_fat_sectors + directory_sectors;
		std::size_t fat_sectors = 0;
		std::size_t difat_sectors = 0;
		for (;;) {
			std::size_t const next_fat = (base_sectors + fat_sectors + difat_sectors + 127) / 128;
			std::size_t const next_difat = next_fat <= 109 ? 0 : (next_fat - 109 + 126) / 127;
			if (next_fat == fat_sectors && next_difat == difat_sectors) break;
			fat_sectors = next_fat;
			difat_sectors = next_difat;
		}
		std::size_t const total_sectors = base_sectors + fat_sectors + difat_sectors;
		if (total_sectors > std::numeric_limits<std::uint32_t>::max()) return(false);

		std::uint32_t cursor = 0;
		for (StreamPlan & plan : plans) {
			if (!plan.Mini && plan.SectorCount != 0) {
				plan.Start = cursor;
				cursor += static_cast<std::uint32_t>(plan.SectorCount);
			}
		}
		std::uint32_t const mini_stream_start = mini_stream_sectors ? cursor : END_OF_CHAIN;
		cursor += static_cast<std::uint32_t>(mini_stream_sectors);
		std::uint32_t const mini_fat_start = mini_fat_sectors ? cursor : END_OF_CHAIN;
		cursor += static_cast<std::uint32_t>(mini_fat_sectors);
		std::uint32_t const directory_start = cursor;
		cursor += static_cast<std::uint32_t>(directory_sectors);
		std::uint32_t const fat_start = cursor;
		cursor += static_cast<std::uint32_t>(fat_sectors);
		std::uint32_t const difat_start = difat_sectors ? cursor : END_OF_CHAIN;

		std::vector<std::uint32_t> fat(total_sectors, FREE_SECTOR);
		for (StreamPlan const & plan : plans) {
			if (!plan.Mini && plan.SectorCount) SetChain(fat, plan.Start, plan.SectorCount);
		}
		if (mini_stream_sectors) SetChain(fat, mini_stream_start, mini_stream_sectors);
		if (mini_fat_sectors) SetChain(fat, mini_fat_start, mini_fat_sectors);
		SetChain(fat, directory_start, directory_sectors);
		for (std::size_t index = 0; index < fat_sectors; ++index) fat[fat_start + index] = FAT_SECTOR;
		for (std::size_t index = 0; index < difat_sectors; ++index) fat[difat_start + index] = DIFAT_SECTOR;

		std::vector<DirectoryEntry> entries(1);
		entries[0].Name = L"Root Entry";
		entries[0].Type = 5;
		entries[0].Start = mini_stream_start;
		entries[0].Size = mini_stream.size();
		for (StreamPlan const & plan : plans) {
			DirectoryEntry entry;
			entry.Name = plan.Name;
			entry.Type = 2;
			entry.Start = plan.Start;
			entry.Size = plan.Data->size();
			entries.push_back(entry);
		}
		std::vector<std::uint32_t> ordered;
		for (std::uint32_t index = 1; index < entries.size(); ++index) ordered.push_back(index);
		std::sort(ordered.begin(), ordered.end(), [&](std::uint32_t left, std::uint32_t right) {
			return(CompoundFileNameLess{}(entries[left].Name, entries[right].Name));
		});
		entries[0].Child = BuildDirectoryTree(entries, ordered, 0, ordered.size());
		if (entries[0].Child != FREE_SECTOR) {
			std::size_t const depth = DirectoryDepth(entries, entries[0].Child);
			ColorDeepestDirectoryNodes(entries, entries[0].Child, 0, depth - 1);
		}

		Bytes file(HEADER_SIZE + total_sectors * SECTOR_SIZE);
		std::array<std::uint8_t, 8> const signature = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
		std::copy(signature.begin(), signature.end(), file.begin());
		Put16(file, 24, 0x003E);
		Put16(file, 26, 3);
		Put16(file, 28, 0xFFFE);
		Put16(file, 30, 9);
		Put16(file, 32, 6);
		Put32(file, 40, 0);
		Put32(file, 44, static_cast<std::uint32_t>(fat_sectors));
		Put32(file, 48, directory_start);
		Put32(file, 52, 0);
		Put32(file, 56, MINI_STREAM_CUTOFF);
		Put32(file, 60, mini_fat_start);
		Put32(file, 64, static_cast<std::uint32_t>(mini_fat_sectors));
		Put32(file, 68, difat_start);
		Put32(file, 72, static_cast<std::uint32_t>(difat_sectors));
		for (std::size_t index = 0; index < 109; ++index) {
			Put32(file, 76 + index * 4, index < fat_sectors ? fat_start + static_cast<std::uint32_t>(index) : FREE_SECTOR);
		}

		auto sector_offset = [](std::uint32_t sector) { return(HEADER_SIZE + static_cast<std::size_t>(sector) * SECTOR_SIZE); };
		for (StreamPlan const & plan : plans) {
			if (plan.Mini || !plan.SectorCount) continue;
			std::copy(plan.Data->begin(), plan.Data->end(), file.begin() + sector_offset(plan.Start));
		}
		if (!mini_stream.empty()) std::copy(mini_stream.begin(), mini_stream.end(), file.begin() + sector_offset(mini_stream_start));
		for (std::size_t index = 0; index < mini_fat.size(); ++index) {
			Put32(file, sector_offset(mini_fat_start) + index * 4, mini_fat[index]);
		}
		for (std::size_t index = mini_fat.size(); index < mini_fat_sectors * 128; ++index) {
			Put32(file, sector_offset(mini_fat_start) + index * 4, FREE_SECTOR);
		}
		for (std::size_t index = 0; index < entries.size(); ++index) {
			std::size_t const offset = sector_offset(directory_start) + index * 128;
			Bytes name;
			AppendUTF16(name, entries[index].Name.substr(0, 31), true);
			std::copy(name.begin(), name.end(), file.begin() + offset);
			Put16(file, offset + 64, static_cast<std::uint16_t>(name.size()));
			file[offset + 66] = entries[index].Type;
			file[offset + 67] = entries[index].Color;
			Put32(file, offset + 68, entries[index].Left);
			Put32(file, offset + 72, entries[index].Right);
			Put32(file, offset + 76, entries[index].Child);
			Put32(file, offset + 116, entries[index].Start);
			Put64(file, offset + 120, entries[index].Size);
		}
		for (std::size_t sector = 0; sector < fat_sectors; ++sector) {
			for (std::size_t entry = 0; entry < 128; ++entry) {
				std::size_t const index = sector * 128 + entry;
				Put32(file, sector_offset(fat_start + static_cast<std::uint32_t>(sector)) + entry * 4,
					index < fat.size() ? fat[index] : FREE_SECTOR);
			}
		}
		std::size_t fat_index = 109;
		for (std::size_t sector = 0; sector < difat_sectors; ++sector) {
			std::size_t const offset = sector_offset(difat_start + static_cast<std::uint32_t>(sector));
			for (std::size_t entry = 0; entry < 127; ++entry) {
				Put32(file, offset + entry * 4, fat_index < fat_sectors ?
					fat_start + static_cast<std::uint32_t>(fat_index++) : FREE_SECTOR);
			}
			Put32(file, offset + 127 * 4, sector + 1 == difat_sectors ? END_OF_CHAIN :
				difat_start + static_cast<std::uint32_t>(sector + 1));
		}

		std::string const temporary = document.Path + ".tmp." + std::to_string(getpid());
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		if (!output) return(false);
		output.write(reinterpret_cast<char const *>(file.data()), static_cast<std::streamsize>(file.size()));
		output.close();
		if (!output) {
			std::filesystem::remove(temporary);
			return(false);
		}
		std::error_code error;
		std::filesystem::rename(temporary, document.Path, error);
		if (error) std::filesystem::remove(temporary);
		return(!error);
	}

	bool ReadCompoundFile(Document & document)
	{
		std::ifstream input(document.Path, std::ios::binary);
		if (!input) return(false);
		Bytes file((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
		std::array<std::uint8_t, 8> const signature = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
		if (file.size() < HEADER_SIZE || !std::equal(signature.begin(), signature.end(), file.begin())) return(false);
		auto const major = Get16(file, 26);
		auto const shift = Get16(file, 30);
		auto const mini_shift = Get16(file, 32);
		auto const fat_count = Get32(file, 44);
		auto const directory_start = Get32(file, 48);
		auto const cutoff = Get32(file, 56);
		auto const mini_fat_start = Get32(file, 60);
		auto const mini_fat_count = Get32(file, 64);
		auto const difat_start = Get32(file, 68);
		auto const difat_count = Get32(file, 72);
		if (!major || (*major != 3 && *major != 4) || !shift || !mini_shift || !fat_count ||
			!directory_start || !cutoff || !mini_fat_start || !mini_fat_count || !difat_start || !difat_count) return(false);
		std::size_t const sector_size = std::size_t(1) << *shift;
		std::size_t const mini_sector_size = std::size_t(1) << *mini_shift;
		std::size_t const header_size = sector_size;
		if (sector_size < 512 || sector_size > 4096 || file.size() < header_size) return(false);
		std::size_t const sector_count = (file.size() - header_size) / sector_size;
		auto sector_offset = [&](std::uint32_t sector) -> std::optional<std::size_t> {
			if (sector >= sector_count) return(std::nullopt);
			return(header_size + static_cast<std::size_t>(sector) * sector_size);
		};

		std::vector<std::uint32_t> fat_sectors;
		for (std::size_t index = 0; index < 109 && fat_sectors.size() < *fat_count; ++index) {
			auto const sector = Get32(file, 76 + index * 4);
			if (!sector) return(false);
			if (*sector != FREE_SECTOR) fat_sectors.push_back(*sector);
		}
		std::uint32_t difat = *difat_start;
		for (std::uint32_t chain = 0; chain < *difat_count && fat_sectors.size() < *fat_count; ++chain) {
			auto const offset = sector_offset(difat);
			if (!offset) return(false);
			std::size_t const entries = sector_size / 4 - 1;
			for (std::size_t index = 0; index < entries && fat_sectors.size() < *fat_count; ++index) {
				auto const sector = Get32(file, *offset + index * 4);
				if (!sector) return(false);
				if (*sector != FREE_SECTOR) fat_sectors.push_back(*sector);
			}
			auto const next = Get32(file, *offset + entries * 4);
			if (!next) return(false);
			difat = *next;
		}
		if (fat_sectors.size() != *fat_count) return(false);
		std::vector<std::uint32_t> fat;
		for (std::uint32_t sector : fat_sectors) {
			auto const offset = sector_offset(sector);
			if (!offset) return(false);
			for (std::size_t index = 0; index < sector_size / 4; ++index) {
				auto const value = Get32(file, *offset + index * 4);
				if (!value) return(false);
				fat.push_back(*value);
			}
		}

		auto read_chain = [&](std::uint32_t start, std::uint64_t length) -> std::optional<Bytes> {
			Bytes result;
			std::uint32_t current = start;
			std::size_t steps = 0;
			while (current != END_OF_CHAIN && result.size() < length) {
				if (current >= fat.size() || ++steps > sector_count) return(std::nullopt);
				auto const offset = sector_offset(current);
				if (!offset) return(std::nullopt);
				std::size_t const amount = static_cast<std::size_t>(std::min<std::uint64_t>(sector_size, length - result.size()));
				result.insert(result.end(), file.begin() + *offset, file.begin() + *offset + amount);
				current = fat[current];
			}
			if (result.size() != length) return(std::nullopt);
			return(result);
		};

		std::uint64_t const directory_length = static_cast<std::uint64_t>(sector_count) * sector_size;
		auto directory = read_chain(*directory_start, directory_length);
		if (!directory) {
			Bytes collected;
			std::uint32_t current = *directory_start;
			std::size_t steps = 0;
			while (current != END_OF_CHAIN) {
				if (current >= fat.size() || ++steps > sector_count) return(false);
				auto const offset = sector_offset(current);
				if (!offset) return(false);
				collected.insert(collected.end(), file.begin() + *offset, file.begin() + *offset + sector_size);
				current = fat[current];
			}
			directory = std::move(collected);
		}

		std::vector<DirectoryEntry> entries;
		for (std::size_t offset = 0; offset + 128 <= directory->size(); offset += 128) {
			auto const name_bytes = Get16(*directory, offset + 64);
			auto const start = Get32(*directory, offset + 116);
			auto const size = Get64(*directory, offset + 120);
			if (!name_bytes || !start || !size || *name_bytes > 64 || (*name_bytes & 1)) return(false);
			DirectoryEntry entry;
			entry.Name = *name_bytes >= 2 ? ReadUTF16(*directory, offset, *name_bytes / 2 - 1) : L"";
			entry.Type = (*directory)[offset + 66];
			entry.Start = *start;
			entry.Size = *size;
			entries.push_back(std::move(entry));
		}
		auto const root = std::find_if(entries.begin(), entries.end(), [](DirectoryEntry const & entry) { return(entry.Type == 5); });
		if (root == entries.end()) return(false);
		auto mini_stream = read_chain(root->Start, root->Size);
		if (!mini_stream && root->Size != 0) return(false);
		if (!mini_stream) mini_stream = Bytes{};
		auto mini_fat_bytes = read_chain(*mini_fat_start, static_cast<std::uint64_t>(*mini_fat_count) * sector_size);
		if (!mini_fat_bytes && *mini_fat_count != 0) return(false);
		std::vector<std::uint32_t> mini_fat;
		if (mini_fat_bytes) {
			for (std::size_t offset = 0; offset + 4 <= mini_fat_bytes->size(); offset += 4) {
				auto const value = Get32(*mini_fat_bytes, offset);
				if (!value) return(false);
				mini_fat.push_back(*value);
			}
		}
		auto read_mini_chain = [&](std::uint32_t start, std::uint64_t length) -> std::optional<Bytes> {
			Bytes result;
			std::uint32_t current = start;
			std::size_t steps = 0;
			while (current != END_OF_CHAIN && result.size() < length) {
				if (current >= mini_fat.size() || ++steps > mini_fat.size()) return(std::nullopt);
				std::size_t const offset = static_cast<std::size_t>(current) * mini_sector_size;
				if (offset > mini_stream->size() || mini_stream->size() - offset < mini_sector_size) return(std::nullopt);
				std::size_t const amount = static_cast<std::size_t>(std::min<std::uint64_t>(mini_sector_size, length - result.size()));
				result.insert(result.end(), mini_stream->begin() + offset, mini_stream->begin() + offset + amount);
				current = mini_fat[current];
			}
			if (result.size() != length) return(std::nullopt);
			return(result);
		};

		for (DirectoryEntry const & entry : entries) {
			if (entry.Type != 2 || entry.Name.empty()) continue;
			std::optional<Bytes> stream;
			if (entry.Size == 0) stream = Bytes{};
			else if (entry.Size < *cutoff) stream = read_mini_chain(entry.Start, entry.Size);
			else stream = read_chain(entry.Start, entry.Size);
			if (!stream) return(false);
			document.Streams[entry.Name] = std::move(*stream);
		}
		auto property = document.Streams.find(SUMMARY_INFORMATION_NAME);
		if (property != document.Streams.end()) ReadPropertySet(property->second, document.SummaryProperties);
		return(true);
	}

	HRESULT CommitDocument(std::shared_ptr<Document> const & document)
	{
		std::lock_guard lock(document->Mutex);
		if (!document->Writable) return(STG_E_ACCESSDENIED);
		if (!WriteCompoundFile(*document)) return(STG_E_WRITEFAULT);
		document->Dirty = false;
		return(S_OK);
	}

	class Stream final : public IStream
	{
		public:
			Stream(std::shared_ptr<Document> document, std::wstring name, std::uint64_t position = 0) :
				Document_(std::move(document)), Name_(std::move(name)), Position_(position) {}

			HRESULT QueryInterface(REFIID identifier, void ** result) override
			{
				if (!result) return(E_POINTER);
				*result = nullptr;
				if (identifier == IID_IUnknown || identifier == IID_IStream) *result = static_cast<IStream *>(this);
				if (!*result) return(E_NOINTERFACE);
				AddRef();
				return(S_OK);
			}
			ULONG AddRef(void) override { return(++References_); }
			ULONG Release(void) override
			{
				ULONG const count = --References_;
				if (count == 0) delete this;
				return(count);
			}
			HRESULT Read(void * destination, ULONG count, ULONG * read) override
			{
				if (read) *read = 0;
				if (!destination && count) return(STG_E_INVALIDPOINTER);
				std::lock_guard lock(Document_->Mutex);
				auto const entry = Document_->Streams.find(Name_);
				if (entry == Document_->Streams.end()) return(STG_E_FILENOTFOUND);
				std::size_t const available = Position_ < entry->second.size() ? entry->second.size() - static_cast<std::size_t>(Position_) : 0;
				std::size_t const amount = std::min<std::size_t>(count, available);
				if (amount) std::memcpy(destination, entry->second.data() + Position_, amount);
				Position_ += amount;
				if (read) *read = static_cast<ULONG>(amount);
				return(amount == count ? S_OK : S_FALSE);
			}
			HRESULT Write(void const * source, ULONG count, ULONG * written) override
			{
				if (written) *written = 0;
				if (!source && count) return(STG_E_INVALIDPOINTER);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				if (Position_ > std::numeric_limits<std::size_t>::max() - count) return(STG_E_MEDIUMFULL);
				std::lock_guard lock(Document_->Mutex);
				Bytes & data = Document_->Streams[Name_];
				std::size_t const end = static_cast<std::size_t>(Position_) + count;
				if (end > data.size()) data.resize(end);
				if (count) std::memcpy(data.data() + Position_, source, count);
				Position_ = end;
				Document_->Dirty = true;
				if (written) *written = count;
				return(S_OK);
			}
			HRESULT Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER * position) override
			{
				std::lock_guard lock(Document_->Mutex);
				auto const entry = Document_->Streams.find(Name_);
				if (entry == Document_->Streams.end()) return(STG_E_FILENOTFOUND);
				std::int64_t base = 0;
				if (origin == STREAM_SEEK_CUR) base = static_cast<std::int64_t>(Position_);
				else if (origin == STREAM_SEEK_END) base = static_cast<std::int64_t>(entry->second.size());
				else if (origin != STREAM_SEEK_SET) return(STG_E_INVALIDFUNCTION);
				if (move.QuadPart < -base) return(STG_E_INVALIDFUNCTION);
				Position_ = static_cast<std::uint64_t>(base + move.QuadPart);
				if (position) position->QuadPart = Position_;
				return(S_OK);
			}
			HRESULT SetSize(ULARGE_INTEGER size) override
			{
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				if (size.QuadPart > std::numeric_limits<std::size_t>::max()) return(STG_E_MEDIUMFULL);
				std::lock_guard lock(Document_->Mutex);
				Document_->Streams[Name_].resize(static_cast<std::size_t>(size.QuadPart));
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT CopyTo(IStream * target, ULARGE_INTEGER count, ULARGE_INTEGER * read, ULARGE_INTEGER * written) override
			{
				if (!target) return(E_POINTER);
				if (read) read->QuadPart = 0;
				if (written) written->QuadPart = 0;
				std::array<std::uint8_t, 16384> buffer;
				std::uint64_t remaining = count.QuadPart;
				while (remaining) {
					ULONG got = 0;
					ULONG const request = static_cast<ULONG>(std::min<std::uint64_t>(remaining, buffer.size()));
					HRESULT result = Read(buffer.data(), request, &got);
					if (FAILED(result)) return(result);
					if (!got) break;
					ULONG put = 0;
					result = target->Write(buffer.data(), got, &put);
					if (FAILED(result)) return(result);
					if (read) read->QuadPart += got;
					if (written) written->QuadPart += put;
					if (put != got) return(STG_E_WRITEFAULT);
					remaining -= got;
				}
				return(S_OK);
			}
			HRESULT Commit(DWORD) override { return(S_OK); }
			HRESULT Revert(void) override { return(STG_E_INVALIDFUNCTION); }
			HRESULT LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return(STG_E_INVALIDFUNCTION); }
			HRESULT UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return(STG_E_INVALIDFUNCTION); }
			HRESULT Stat(STATSTG * status, DWORD flags) override
			{
				if (!status) return(E_POINTER);
				*status = {};
				std::lock_guard lock(Document_->Mutex);
				auto const entry = Document_->Streams.find(Name_);
				if (entry == Document_->Streams.end()) return(STG_E_FILENOTFOUND);
				status->type = STGTY_STREAM;
				status->cbSize.QuadPart = entry->second.size();
				if (!(flags & STATFLAG_NONAME)) {
					status->pwcsName = static_cast<wchar_t *>(malloc((Name_.size() + 1) * sizeof(wchar_t)));
					if (!status->pwcsName) return(E_OUTOFMEMORY);
					std::copy(Name_.begin(), Name_.end(), status->pwcsName);
					status->pwcsName[Name_.size()] = 0;
				}
				return(S_OK);
			}
			HRESULT Clone(IStream ** clone) override
			{
				if (!clone) return(E_POINTER);
				*clone = new (std::nothrow) Stream(Document_, Name_, Position_);
				return(*clone ? S_OK : E_OUTOFMEMORY);
			}

		private:
			std::atomic<ULONG> References_ {1};
			std::shared_ptr<Document> Document_;
			std::wstring Name_;
			std::uint64_t Position_ = 0;
	};

	class PropertyStorage final : public IPropertyStorage
	{
		public:
			explicit PropertyStorage(std::shared_ptr<Document> document) : Document_(std::move(document)) {}
			HRESULT QueryInterface(REFIID identifier, void ** result) override
			{
				if (!result) return(E_POINTER);
				*result = nullptr;
				if (identifier == IID_IUnknown || identifier == IID_IPropertyStorage) *result = static_cast<IPropertyStorage *>(this);
				if (!*result) return(E_NOINTERFACE);
				AddRef();
				return(S_OK);
			}
			ULONG AddRef(void) override { return(++References_); }
			ULONG Release(void) override
			{
				ULONG const count = --References_;
				if (!count) delete this;
				return(count);
			}
			HRESULT ReadMultiple(ULONG count, PROPSPEC const * specifications, PROPVARIANT * values) override
			{
				if ((!specifications || !values) && count) return(E_POINTER);
				std::lock_guard lock(Document_->Mutex);
				HRESULT result = S_OK;
				for (ULONG index = 0; index < count; ++index) {
					values[index] = {};
					if (specifications[index].ulKind != PRSPEC_PROPID) { result = S_FALSE; continue; }
					auto const property = Document_->SummaryProperties.find(specifications[index].propid);
					if (property == Document_->SummaryProperties.end()) { result = S_FALSE; continue; }
					values[index].vt = property->second.Type;
					switch (property->second.Type) {
						case VT_I4: values[index].lVal = property->second.Integer; break;
						case VT_FILETIME: values[index].filetime = property->second.Time; break;
						case VT_LPWSTR:
							values[index].pwszVal = static_cast<wchar_t *>(malloc((property->second.String.size() + 1) * sizeof(wchar_t)));
							if (!values[index].pwszVal) return(E_OUTOFMEMORY);
							std::copy(property->second.String.begin(), property->second.String.end(), values[index].pwszVal);
							values[index].pwszVal[property->second.String.size()] = 0;
							break;
						default: values[index].vt = VT_EMPTY; result = S_FALSE; break;
					}
				}
				return(result);
			}
			HRESULT WriteMultiple(ULONG count, PROPSPEC const * specifications, PROPVARIANT const * values, PROPID) override
			{
				if ((!specifications || !values) && count) return(E_POINTER);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				std::lock_guard lock(Document_->Mutex);
				for (ULONG index = 0; index < count; ++index) {
					if (specifications[index].ulKind != PRSPEC_PROPID) return(E_INVALIDARG);
					PropertyValue value;
					value.Type = values[index].vt;
					switch (value.Type) {
						case VT_I4: value.Integer = values[index].lVal; break;
						case VT_FILETIME: value.Time = values[index].filetime; break;
						case VT_LPWSTR:
							if (!values[index].pwszVal) return(E_INVALIDARG);
							value.String = values[index].pwszVal;
							break;
						default: return(E_INVALIDARG);
					}
					Document_->SummaryProperties[specifications[index].propid] = std::move(value);
				}
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT DeleteMultiple(ULONG count, PROPSPEC const * specifications) override
			{
				if (!specifications && count) return(E_POINTER);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				std::lock_guard lock(Document_->Mutex);
				for (ULONG index = 0; index < count; ++index) {
					if (specifications[index].ulKind == PRSPEC_PROPID) Document_->SummaryProperties.erase(specifications[index].propid);
				}
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT ReadPropertyNames(ULONG, PROPID const *, LPOLESTR *) override { return(E_NOTIMPL); }
			HRESULT WritePropertyNames(ULONG, PROPID const *, LPOLESTR const *) override { return(E_NOTIMPL); }
			HRESULT DeletePropertyNames(ULONG, PROPID const *) override { return(E_NOTIMPL); }
			HRESULT Commit(DWORD) override { return(S_OK); }
			HRESULT Revert(void) override { return(STG_E_INVALIDFUNCTION); }
			HRESULT Enum(IEnumSTATPROPSTG **) override { return(E_NOTIMPL); }
			HRESULT SetTimes(FILETIME const *, FILETIME const *, FILETIME const *) override { return(S_OK); }
			HRESULT SetClass(REFCLSID) override { return(S_OK); }
			HRESULT Stat(STATPROPSETSTG * status) override
			{
				if (!status) return(E_POINTER);
				*status = {};
				status->fmtid = FMTID_SummaryInformation;
				return(S_OK);
			}

		private:
			std::atomic<ULONG> References_ {1};
			std::shared_ptr<Document> Document_;
	};

	class Storage final : public IStorage, public IPropertySetStorage
	{
		public:
			explicit Storage(std::shared_ptr<Document> document) : Document_(std::move(document)) {}
			HRESULT QueryInterface(REFIID identifier, void ** result) override
			{
				if (!result) return(E_POINTER);
				*result = nullptr;
				if (identifier == IID_IUnknown || identifier == IID_IStorage) *result = static_cast<IStorage *>(this);
				else if (identifier == IID_IPropertySetStorage) *result = static_cast<IPropertySetStorage *>(this);
				if (!*result) return(E_NOINTERFACE);
				AddRef();
				return(S_OK);
			}
			ULONG AddRef(void) override { return(++References_); }
			ULONG Release(void) override
			{
				ULONG const count = --References_;
				if (!count) delete this;
				return(count);
			}
			HRESULT CreateStream(WCHAR const * name, DWORD, DWORD, DWORD, IStream ** stream) override
			{
				if (!name || !stream) return(E_POINTER);
				*stream = nullptr;
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				{
					std::lock_guard lock(Document_->Mutex);
					Document_->Streams[name].clear();
					Document_->Dirty = true;
				}
				*stream = new (std::nothrow) Stream(Document_, name);
				return(*stream ? S_OK : E_OUTOFMEMORY);
			}
			HRESULT OpenStream(WCHAR const * name, void *, DWORD, DWORD, IStream ** stream) override
			{
				if (!name || !stream) return(E_POINTER);
				*stream = nullptr;
				{
					std::lock_guard lock(Document_->Mutex);
					if (!Document_->Streams.contains(name)) return(STG_E_FILENOTFOUND);
				}
				*stream = new (std::nothrow) Stream(Document_, name);
				return(*stream ? S_OK : E_OUTOFMEMORY);
			}
			HRESULT CreateStorage(WCHAR const *, DWORD, DWORD, DWORD, IStorage **) override { return(E_NOTIMPL); }
			HRESULT OpenStorage(WCHAR const *, IStorage *, DWORD, SNB, DWORD, IStorage **) override { return(E_NOTIMPL); }
			HRESULT CopyTo(DWORD, IID const *, SNB, IStorage *) override { return(E_NOTIMPL); }
			HRESULT MoveElementTo(WCHAR const *, IStorage *, WCHAR const *, DWORD) override { return(E_NOTIMPL); }
			HRESULT Commit(DWORD) override { return(CommitDocument(Document_)); }
			HRESULT Revert(void) override { return(STG_E_INVALIDFUNCTION); }
			HRESULT EnumElements(DWORD, void *, DWORD, IEnumSTATSTG **) override { return(E_NOTIMPL); }
			HRESULT DestroyElement(WCHAR const * name) override
			{
				if (!name) return(E_POINTER);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				std::lock_guard lock(Document_->Mutex);
				if (!Document_->Streams.erase(name)) return(STG_E_FILENOTFOUND);
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT RenameElement(WCHAR const * old_name, WCHAR const * new_name) override
			{
				if (!old_name || !new_name) return(E_POINTER);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				std::lock_guard lock(Document_->Mutex);
				auto node = Document_->Streams.extract(old_name);
				if (node.empty()) return(STG_E_FILENOTFOUND);
				node.key() = new_name;
				Document_->Streams.insert(std::move(node));
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT SetElementTimes(WCHAR const *, FILETIME const *, FILETIME const *, FILETIME const *) override { return(S_OK); }
			HRESULT SetClass(REFCLSID) override { return(S_OK); }
			HRESULT SetStateBits(DWORD, DWORD) override { return(S_OK); }
			HRESULT Stat(STATSTG * status, DWORD flags) override
			{
				if (!status) return(E_POINTER);
				*status = {};
				status->type = STGTY_STORAGE;
				if (!(flags & STATFLAG_NONAME)) {
					std::wstring name;
					for (char character : Document_->Path) name.push_back(static_cast<unsigned char>(character));
					status->pwcsName = static_cast<wchar_t *>(malloc((name.size() + 1) * sizeof(wchar_t)));
					if (!status->pwcsName) return(E_OUTOFMEMORY);
					std::copy(name.begin(), name.end(), status->pwcsName);
					status->pwcsName[name.size()] = 0;
				}
				return(S_OK);
			}
			HRESULT Create(REFFMTID identifier, CLSID const *, DWORD, DWORD, IPropertyStorage ** storage) override
			{
				if (!storage) return(E_POINTER);
				*storage = nullptr;
				if (identifier != FMTID_SummaryInformation) return(E_NOTIMPL);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				*storage = new (std::nothrow) PropertyStorage(Document_);
				return(*storage ? S_OK : E_OUTOFMEMORY);
			}
			HRESULT Open(REFFMTID identifier, DWORD, IPropertyStorage ** storage) override
			{
				if (!storage) return(E_POINTER);
				*storage = nullptr;
				if (identifier != FMTID_SummaryInformation) return(STG_E_FILENOTFOUND);
				{
					std::lock_guard lock(Document_->Mutex);
					if (Document_->SummaryProperties.empty()) return(STG_E_FILENOTFOUND);
				}
				*storage = new (std::nothrow) PropertyStorage(Document_);
				return(*storage ? S_OK : E_OUTOFMEMORY);
			}
			HRESULT Delete(REFFMTID identifier) override
			{
				if (identifier != FMTID_SummaryInformation) return(STG_E_FILENOTFOUND);
				if (!Document_->Writable) return(STG_E_ACCESSDENIED);
				std::lock_guard lock(Document_->Mutex);
				Document_->SummaryProperties.clear();
				Document_->Streams.erase(SUMMARY_INFORMATION_NAME);
				Document_->Dirty = true;
				return(S_OK);
			}
			HRESULT Enum(IEnumSTATPROPSETSTG **) override { return(E_NOTIMPL); }

		private:
			std::atomic<ULONG> References_ {1};
			std::shared_ptr<Document> Document_;
	};
}

HRESULT StgCreateDocfile(WCHAR const * path, DWORD mode, DWORD, IStorage ** storage)
{
	if (!path || !storage) return(E_POINTER);
	*storage = nullptr;
	std::string const native_path = UTF8Path(path);
	if (native_path.empty()) return(E_INVALIDARG);
	if (!(mode & STGM_CREATE) && std::filesystem::exists(native_path)) return(STG_E_FILEALREADYEXISTS);
	auto document = std::make_shared<Document>();
	document->Path = native_path;
	document->Writable = true;
	document->Dirty = true;
	*storage = new (std::nothrow) Storage(std::move(document));
	return(*storage ? S_OK : E_OUTOFMEMORY);
}

HRESULT StgOpenStorage(WCHAR const * path, IStorage *, DWORD mode, SNB, DWORD, IStorage ** storage)
{
	if (!path || !storage) return(E_POINTER);
	*storage = nullptr;
	auto document = std::make_shared<Document>();
	document->Path = UTF8Path(path);
	document->Writable = (mode & STGM_WRITE) || (mode & STGM_READWRITE);
	if (!ReadCompoundFile(*document)) return(STG_E_FILENOTFOUND);
	*storage = new (std::nothrow) Storage(std::move(document));
	return(*storage ? S_OK : E_OUTOFMEMORY);
}

HRESULT PropVariantClear(PROPVARIANT * value)
{
	if (!value) return(E_INVALIDARG);
	if (value->vt == VT_LPWSTR) free(value->pwszVal);
	*value = {};
	return(S_OK);
}

HRESULT CoFileTimeNow(FILETIME * time)
{
	if (!time) return(E_POINTER);
	auto const now = std::chrono::system_clock::now().time_since_epoch();
	std::uint64_t const ticks = static_cast<std::uint64_t>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(now).count() / 100) + 116444736000000000ULL;
	time->dwLowDateTime = static_cast<DWORD>(ticks);
	time->dwHighDateTime = static_cast<DWORD>(ticks >> 32);
	return(S_OK);
}
