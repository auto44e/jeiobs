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

		for (std::size_t line_start = 0U; line_start < bytes.size();) {
			std::size_t line_end = line_start;
			while (line_end < bytes.size() && !IsNewline(bytes[line_end])) {
				++line_end;
			}
			if (line_end - line_start > Limits::kIniLineBytes) {
				result.error = MakeError(ErrorCode::Limit, "line");
				return result;
			}
			if (line_end == bytes.size()) {
				break;
			}
			if (bytes[line_end] == '\r' && line_end + 1U < bytes.size() && bytes[line_end + 1U] == '\n') {
				line_start = line_end + 2U;
			} else if (bytes[line_end] == '\n' && line_end + 1U < bytes.size() &&
				   bytes[line_end + 1U] == '\r') {
				result.uncertain = true;
				line_start = line_end + 2U;
			} else {
				if (bytes[line_end] == '\r') {
					result.uncertain = true;
				}
				line_start = line_end + 1U;
			}
		}

		std::size_t offset = 0U;
		if (bytes.size() >= 3U && static_cast<unsigned char>(bytes[0]) == 0xefU &&
		    static_cast<unsigned char>(bytes[1]) == 0xbbU && static_cast<unsigned char>(bytes[2]) == 0xbfU) {
			result.has_bom = true;
			offset = 3U;
		}
		std::size_t section_index = kNoIndex;
		const auto consume_newline = [&bytes](std::size_t &position) {
			if (position >= bytes.size() || !IsNewline(bytes[position])) {
				return;
			}
			if (position + 1U < bytes.size() &&
			    ((bytes[position] == '\r' && bytes[position + 1U] == '\n') ||
			     (bytes[position] == '\n' && bytes[position + 1U] == '\r'))) {
				position += 2U;
			} else {
				++position;
			}
		};
		const auto skip_whitespace = [&bytes, &consume_newline](std::size_t &position) {
			while (position < bytes.size()) {
				if (bytes[position] == ' ' || bytes[position] == '\t') {
					++position;
				} else if (IsNewline(bytes[position])) {
					consume_newline(position);
				} else {
					break;
				}
			}
		};
		const auto skip_to_newline = [&bytes, &consume_newline](std::size_t &position) {
			while (position < bytes.size() && !IsNewline(bytes[position])) {
				++position;
			}
			consume_newline(position);
		};
		const auto append_key = [&](std::size_t &position, std::size_t current_section) {
			if (position >= bytes.size()) {
				return;
			}
			if (bytes[position] == '#') {
				skip_to_newline(position);
				return;
			}

			const std::size_t key_start = position;
			while (position < bytes.size() && bytes[position] != '=' && !IsNewline(bytes[position])) {
				++position;
			}
			if (position >= bytes.size() || IsNewline(bytes[position])) {
				consume_newline(position);
				return;
			}

			const std::size_t equals = position++;
			const std::size_t value_start = position;
			while (position < bytes.size() && !IsNewline(bytes[position])) {
				++position;
			}
			const std::size_t value_end = position;
			if (key_start < equals) {
				if (result.keys.size() >= Limits::kIniKeys) {
					result.error = MakeError(ErrorCode::Limit, "values");
					return;
				}
				IniKeyOccurrence key;
				key.start = value_start;
				key.end = value_end;
				key.value_start = value_start;
				key.value_end = value_end;
				key.section_index = current_section;
				key.key_start = key_start;
				key.key_end = equals;
				result.keys.push_back(std::move(key));
			}
			consume_newline(position);
		};

		while (offset < bytes.size()) {
			skip_whitespace(offset);
			if (offset >= bytes.size()) {
				break;
			}
			if (bytes[offset] == '[') {
				++offset;
				const std::size_t name_start = offset;
				while (offset < bytes.size() && bytes[offset] != ']' && !IsNewline(bytes[offset])) {
					++offset;
				}
				const std::size_t name_end = offset;
				const bool has_close = offset < bytes.size() && bytes[offset] == ']';
				bool has_nonblank_name = false;
				for (std::size_t index = name_start; index < name_end; ++index) {
					if (bytes[index] != ' ' && bytes[index] != '\t') {
						has_nonblank_name = true;
						break;
					}
				}

				bool exact_header = has_close && name_start < name_end && has_nonblank_name;
				if (has_close) {
					++offset;
				}
				for (std::size_t trailing = offset;
				     trailing < bytes.size() && !IsNewline(bytes[trailing]); ++trailing) {
					if (bytes[trailing] != ' ' && bytes[trailing] != '\t') {
						exact_header = false;
						break;
					}
				}
				if (!exact_header || name_end - name_start > 1024U) {
					// A malformed header, or a section name too long to copy into pointers.
					result.uncertain = true;
				}

				if (name_start < name_end) {
					if (result.sections.size() >= Limits::kIniSections) {
						result.error = MakeError(ErrorCode::Limit, "values");
						return result;
					}
					IniSectionOccurrence section;
					section.name.assign(bytes.substr(name_start, name_end - name_start));
					section.start = name_start;
					section.end = name_end;
					result.sections.push_back(std::move(section));
					section_index = result.sections.size() - 1U;
				}
			} else {
				append_key(offset, section_index);
			}

			while (offset < bytes.size()) {
				skip_whitespace(offset);
				if (offset >= bytes.size() || bytes[offset] == '[') {
					break;
				}
				append_key(offset, section_index);
				if (!result.error.ok()) {
					return result;
				}
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
