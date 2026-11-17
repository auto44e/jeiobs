/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
#include "jeiobs/workspace/IniScan.hpp"

#include "jeiobs/workspace/Limits.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace Jeiobs::Workspace {
namespace {

constexpr std::size_t kNoIndex = std::numeric_limits<std::size_t>::max();

Error MakeError(ErrorCode code, std::string detail = {})
{
	return {code, std::move(detail)};
}

bool IsValidUtf8(std::string_view bytes)
{
	for (std::size_t index = 0U; index < bytes.size();) {
		const unsigned char first = static_cast<unsigned char>(bytes[index]);
		std::size_t width = 0U;
		std::uint32_t value = 0U;
		if (first <= 0x7fU) {
			width = 1U;
			value = first;
		} else if (first >= 0xc2U && first <= 0xdfU) {
			width = 2U;
			value = first & 0x1fU;
		} else if (first >= 0xe0U && first <= 0xefU) {
			width = 3U;
			value = first & 0x0fU;
		} else if (first >= 0xf0U && first <= 0xf4U) {
			width = 4U;
			value = first & 0x07U;
		} else {
			return false;
		}
		if (index + width > bytes.size()) {
			return false;
		}
		for (std::size_t offset = 1U; offset < width; ++offset) {
			const unsigned char continuation = static_cast<unsigned char>(bytes[index + offset]);
			if ((continuation & 0xc0U) != 0x80U) {
				return false;
			}
			value = (value << 6U) | (continuation & 0x3fU);
		}
		if ((width == 2U && value < 0x80U) || (width == 3U && value < 0x800U) ||
		    (width == 4U && value < 0x10000U) || (value >= 0xd800U && value <= 0xdfffU) || value > 0x10ffffU) {
			return false;
		}
		index += width;
	}
	return true;
}

std::size_t TrimLeft(std::string_view line)
{
	std::size_t offset = 0U;
	while (offset < line.size() && (line[offset] == ' ' || line[offset] == '\t')) {
		++offset;
	}
	return offset;
}

bool IsNewline(char character)
{
	return character == '\r' || character == '\n';
}

} // namespace

IniScanResult IniScan::Scan(std::string_view bytes)
{
	IniScanResult result;
	try {
		if (bytes.size() > Limits::kDocumentBytes) {
			result.error = MakeError(ErrorCode::Limit, "blob");
			return result;
		}
		if (bytes.find('\0') != std::string_view::npos) {
			result.error = MakeError(ErrorCode::Nul);
			return result;
		}
		if (!IsValidUtf8(bytes)) {
			result.error = MakeError(ErrorCode::Utf8);
			return result;
		}

		std::size_t offset = 0U;
		if (bytes.size() >= 3U && static_cast<unsigned char>(bytes[0]) == 0xefU &&
		    static_cast<unsigned char>(bytes[1]) == 0xbbU && static_cast<unsigned char>(bytes[2]) == 0xbfU) {
			result.has_bom = true;
			offset = 3U;
		}
		std::size_t section_index = kNoIndex;
		while (offset < bytes.size() || (offset == 0U && bytes.empty())) {
			const std::size_t line_start = offset;
			std::size_t line_end = offset;
			while (line_end < bytes.size() && !IsNewline(bytes[line_end])) {
				++line_end;
			}
			if (line_end - line_start > Limits::kIniLineBytes) {
				result.error = MakeError(ErrorCode::Limit, "line");
				return result;
			}

			const std::string_view line = bytes.substr(line_start, line_end - line_start);
			const std::size_t first = TrimLeft(line);
			std::size_t parse_start = first;
			if (first < line.size() && line[first] == '[') {
				const std::size_t close = line.find(']', first + 1U);
				bool exact_header = false;
				if (close != std::string_view::npos && close > first + 1U) {
					std::size_t trailing = close + 1U;
					while (trailing < line.size() &&
					       (line[trailing] == ' ' || line[trailing] == '\t')) {
						++trailing;
					}
					exact_header = trailing == line.size() || line[trailing] == ';' ||
						       line[trailing] == '#';
				}
				if (!exact_header) {
					result.uncertain = true;
				}
				if (exact_header) {
					if (result.sections.size() >= Limits::kIniSections) {
						result.error = MakeError(ErrorCode::Limit, "values");
						return result;
					}
					const std::size_t name_start = line_start + first + 1U;
					const std::size_t name_end = line_start + close;
					IniSectionOccurrence section;
					section.name.assign(bytes.substr(name_start, name_end - name_start));
					section.start = name_start;
					section.end = name_end;
					result.sections.push_back(std::move(section));
					section_index = result.sections.size() - 1U;
					parse_start = close + 1U;
				} else {
					parse_start = first;
				}
			}

			const std::size_t key_first = TrimLeft(line.substr(parse_start));
			const std::size_t key_begin = parse_start + key_first;
			if (key_begin < line.size() && line[key_begin] != ';' && line[key_begin] != '#') {
				const std::size_t equals = line.find('=', key_begin);
				if (equals != std::string_view::npos) {
					const std::size_t key_end = equals;
					if (key_end > key_begin) {
						if (result.keys.size() >= Limits::kIniKeys) {
							result.error = MakeError(ErrorCode::Limit, "values");
							return result;
						}
						const std::size_t key_start_offset = line_start + key_begin;
						const std::size_t key_end_offset = line_start + key_end;
						const std::size_t value_start = line_start + equals + 1U;
						const std::size_t value_end = line_start + line.size();
						IniKeyOccurrence key;
						key.start = value_start;
						key.end = value_end;
						key.value_start = value_start;
						key.value_end = value_end;
						key.section_index = section_index;
						key.key_start = key_start_offset;
						key.key_end = key_end_offset;

						result.keys.push_back(std::move(key));
					}
				}
			}

			if (line_end == bytes.size()) {
				break;
			}
			const char newline = bytes[line_end];
			if (newline == '\r') {
				if (line_end + 1U < bytes.size() && bytes[line_end + 1U] == '\n') {
					offset = line_end + 2U;
				} else if (line_end + 1U < bytes.size() && bytes[line_end + 1U] == '\r') {
					offset = line_end + 1U;
				} else {
					result.uncertain = true;
					offset = line_end + 1U;
				}
			} else if (line_end + 1U < bytes.size() && bytes[line_end + 1U] == '\r') {
				offset = line_end + 2U;
			} else {
				offset = line_end + 1U;
			}
		}
	} catch (...) {
		result.error = MakeError(ErrorCode::Json);
	}
	return result;
}

IniScanResult ScanIni(std::string_view bytes)
{
	return IniScan::Scan(bytes);
}

} // namespace Jeiobs::Workspace
