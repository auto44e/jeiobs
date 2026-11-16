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
#include <string>
#include <string_view>
#include <utility>

namespace Jeiobs::Workspace {
namespace {

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

std::size_t TrimRight(std::string_view line, std::size_t begin, std::size_t end)
{
	while (end > begin && (line[end - 1U] == ' ' || line[end - 1U] == '\t')) {
		--end;
	}
	return end;
}

std::string EscapePointerToken(std::string_view token)
{
	std::string escaped;
	escaped.reserve(token.size());
	for (const char character : token) {
		if (character == '~') {
			escaped += "~0";
		} else if (character == '/') {
			escaped += "~1";
		} else {
			escaped.push_back(character);
		}
	}
	return escaped;
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
		std::string current_section;
		while (offset <= bytes.size()) {
			const std::size_t line_start = offset;
			const std::size_t newline = bytes.find('\n', offset);
			const std::size_t line_end = newline == std::string_view::npos ? bytes.size() : newline;
			std::size_t content_end = line_end;
			if (content_end > line_start && bytes[content_end - 1U] == '\r') {
				--content_end;
			}
			if (content_end - line_start > Limits::kIniLineBytes) {
				result.error = MakeError(ErrorCode::Limit, "line");
				return result;
			}

			const std::string_view line = bytes.substr(line_start, content_end - line_start);
			const std::size_t first = TrimLeft(line);
			if (first < line.size() && line[first] != ';' && line[first] != '#') {
				const std::size_t trimmed_end = TrimRight(line, first, line.size());
				if (trimmed_end > first + 1U && line[first] == '[' && line[trimmed_end - 1U] == ']') {
					const std::size_t name_start = first + 1U;
					const std::size_t name_end = TrimRight(line, name_start, trimmed_end - 1U);
					const std::size_t name_offset = line_start + name_start;
					const std::size_t name_length = name_end - name_start;
					current_section.assign(line.substr(name_start, name_length));
					result.sections.push_back(
						{current_section, name_offset, name_offset + name_length});
				} else {
					const std::size_t equals = line.find('=', first);
					if (equals != std::string_view::npos) {
						const std::size_t key_end = TrimRight(line, first, equals);
						const std::string key(line.substr(first, key_end - first));
						const std::size_t value_start = line_start + equals + 1U;
						const std::size_t value_end = line_start + line.size();
						std::string pointer;
						pointer.reserve(current_section.size() + key.size() + 2U);
						pointer.push_back('/');
						pointer += EscapePointerToken(current_section);
						pointer.push_back('/');
						pointer += EscapePointerToken(key);
						result.keys.push_back({current_section, key, std::move(pointer),
								       value_start, value_end, value_start, value_end});
					}
				}
			}

			if (newline == std::string_view::npos) {
				break;
			}
			offset = newline + 1U;
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
