/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License or (at your
 * option) any later version.
 */
#include "jeiobs/workspace/PathRules.hpp"

#include "jeiobs/workspace/Limits.hpp"
#include "jeiobs/workspace/Unicode.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

struct CodePoint {
	std::uint32_t value = 0;
	std::size_t bytes = 0;
};

bool DecodeUtf8(std::string_view text, std::vector<CodePoint> &code_points)
{
	for (std::size_t index = 0; index < text.size();) {
		const unsigned char first = static_cast<unsigned char>(text[index]);
		CodePoint code_point;
		if (first <= 0x7fU) {
			code_point = {first, 1U};
		} else if (first >= 0xc2U && first <= 0xdfU) {
			code_point = {static_cast<std::uint32_t>(first & 0x1fU), 2U};
		} else if (first >= 0xe0U && first <= 0xefU) {
			code_point = {static_cast<std::uint32_t>(first & 0x0fU), 3U};
		} else if (first >= 0xf0U && first <= 0xf4U) {
			code_point = {static_cast<std::uint32_t>(first & 0x07U), 4U};
		} else {
			return false;
		}

		if (index + code_point.bytes > text.size()) {
			return false;
		}

		for (std::size_t continuation = 1U; continuation < code_point.bytes; ++continuation) {
			const unsigned char byte = static_cast<unsigned char>(text[index + continuation]);
			if ((byte & 0xc0U) != 0x80U) {
				return false;
			}
			code_point.value = (code_point.value << 6U) | (byte & 0x3fU);
		}

		if ((code_point.bytes == 2U && code_point.value < 0x80U) ||
		    (code_point.bytes == 3U && code_point.value < 0x800U) ||
		    (code_point.bytes == 4U && code_point.value < 0x10000U) ||
		    (code_point.value >= 0xd800U && code_point.value <= 0xdfffU) || code_point.value > 0x10ffffU) {
			return false;
		}

		code_points.push_back(code_point);
		index += code_point.bytes;
	}

	return true;
}

bool IsControl(std::uint32_t value)
{
	return value <= 0x1fU || value == 0x7fU;
}

bool IsReserved(std::uint32_t value)
{
	switch (value) {
	case '<':
	case '>':
	case '"':
	case '|':
	case '?':
	case '*':
		return true;
	default:
		return false;
	}
}

std::string AsciiFold(std::string_view value)
{
	std::string folded;
	folded.reserve(value.size());
	for (const char character : value) {
		if (character >= 'A' && character <= 'Z') {
			folded.push_back(static_cast<char>(character - 'A' + 'a'));
		} else {
			folded.push_back(character);
		}
	}
	return folded;
}

bool IsAsciiEqual(std::string_view value, std::string_view expected)
{
	return AsciiFold(value) == AsciiFold(expected);
}

bool IsDeviceName(std::string_view component)
{
	const std::size_t dot = component.find('.');
	std::string_view stem = component.substr(0U, dot);
	if (dot != std::string_view::npos) {
		while (!stem.empty() && stem.back() == ' ') {
			stem.remove_suffix(1U);
		}
	}
	if (IsAsciiEqual(stem, "CON") || IsAsciiEqual(stem, "PRN") || IsAsciiEqual(stem, "AUX") ||
	    IsAsciiEqual(stem, "NUL") || IsAsciiEqual(stem, "CONIN$") || IsAsciiEqual(stem, "CONOUT$")) {
		return true;
	}

	std::vector<CodePoint> code_points;
	if (!DecodeUtf8(stem, code_points)) {
		return false;
	}

	if (code_points.size() == 4U && (code_points[0].value == 'C' || code_points[0].value == 'c') &&
	    (code_points[1].value == 'O' || code_points[1].value == 'o') &&
	    (code_points[2].value == 'M' || code_points[2].value == 'm')) {
		return (code_points[3].value >= '0' && code_points[3].value <= '9') ||
		       (code_points[3].value >= 0x00b9U && code_points[3].value <= 0x00b9U) ||
		       code_points[3].value == 0x00b2U || code_points[3].value == 0x00b3U;
	}

	if (code_points.size() == 4U && (code_points[0].value == 'L' || code_points[0].value == 'l') &&
	    (code_points[1].value == 'P' || code_points[1].value == 'p') &&
	    (code_points[2].value == 'T' || code_points[2].value == 't')) {
		return (code_points[3].value >= '0' && code_points[3].value <= '9') ||
		       code_points[3].value == 0x00b9U || code_points[3].value == 0x00b2U ||
		       code_points[3].value == 0x00b3U;
	}

	return false;
}

} // namespace

std::string EntryCollisionKey(std::string_view root, std::string_view path)
{
	std::string key(root);
	key.push_back('\0');
	std::string folded;
	if (CollisionKey(path, folded)) {
		key += folded;
	} else {
		key.push_back('\xFF');
		key.append(path);
	}
	return key;
}

PathResult ValidatePath(std::string_view path)
{
	if (path.empty()) {
		return {PathStatus::Invalid, "empty"};
	}
	if (path.size() > Limits::kPathBytes) {
		return {PathStatus::Invalid, "path"};
	}

	std::vector<CodePoint> code_points;
	if (!DecodeUtf8(path, code_points)) {
		return {PathStatus::Invalid, "not-utf8"};
	}

	if (path.front() == '/') {
		return {PathStatus::Invalid, "absolute"};
	}

	for (const CodePoint &code_point : code_points) {
		if (IsControl(code_point.value)) {
			return {PathStatus::Invalid, "control"};
		}
		if (IsReserved(code_point.value)) {
			return {PathStatus::Invalid, "reserved-char"};
		}
	}

	std::size_t component_start = 0U;
	std::size_t component_count = 0U;
	while (component_start <= path.size()) {
		const std::size_t slash = path.find('/', component_start);
		const std::size_t component_end = slash == std::string_view::npos ? path.size() : slash;
		const std::string_view component = path.substr(component_start, component_end - component_start);

		++component_count;
		if (component_count > Limits::kPathComponents) {
			return {PathStatus::Invalid, "components"};
		}
		if (component.empty() || component == ".") {
			return {PathStatus::Invalid, "dot"};
		}
		if (component == "..") {
			return {PathStatus::Invalid, "dotdot"};
		}
		if (component.size() > Limits::kComponentBytes) {
			return {PathStatus::Invalid, "component"};
		}
		if (component.find('.') != std::string_view::npos && IsDeviceName(component)) {
			return {PathStatus::Invalid, "device-name"};
		}
		if (component.back() == '.' || component.back() == ' ') {
			return {PathStatus::Invalid, "trailing-dot-space"};
		}
		if (component.find('\\') != std::string_view::npos) {
			return {PathStatus::Invalid, "backslash"};
		}
		if (component.find(':') != std::string_view::npos) {
			return {PathStatus::Invalid, "colon"};
		}
		if (IsDeviceName(component)) {
			return {PathStatus::Invalid, "device-name"};
		}

		if (slash == std::string_view::npos) {
			break;
		}
		component_start = slash + 1U;
	}

	return {PathStatus::Ok, {}};
}

} // namespace Jeiobs::Workspace
