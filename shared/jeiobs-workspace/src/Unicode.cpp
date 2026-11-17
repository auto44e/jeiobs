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
#include "jeiobs/workspace/Unicode.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

struct MapRow {
	std::uint32_t code;
	std::uint32_t offset;
	std::uint32_t count;
};

struct ClassRow {
	std::uint32_t code;
	std::uint32_t value;
};

#include "UnicodeTables.inc"

constexpr std::uint32_t kHangulBase = 0xAC00U;
constexpr std::uint32_t kLeadBase = 0x1100U;
constexpr std::uint32_t kVowelBase = 0x1161U;
constexpr std::uint32_t kTrailBase = 0x11A7U;
constexpr std::uint32_t kVowelCount = 21U;
constexpr std::uint32_t kTrailCount = 28U;
constexpr std::uint32_t kSyllableCount = 19U * kVowelCount * kTrailCount;

bool Decode(std::string_view text, std::vector<std::uint32_t> &out)
{
	out.reserve(text.size());
	for (std::size_t index = 0U; index < text.size();) {
		const unsigned char first = static_cast<unsigned char>(text[index]);
		std::uint32_t value;
		std::size_t length;
		std::uint32_t minimum;
		if (first <= 0x7FU) {
			value = first;
			length = 1U;
			minimum = 0U;
		} else if (first >= 0xC2U && first <= 0xDFU) {
			value = first & 0x1FU;
			length = 2U;
			minimum = 0x80U;
		} else if (first >= 0xE0U && first <= 0xEFU) {
			value = first & 0x0FU;
			length = 3U;
			minimum = 0x800U;
		} else if (first >= 0xF0U && first <= 0xF4U) {
			value = first & 0x07U;
			length = 4U;
			minimum = 0x10000U;
		} else {
			return false;
		}
		if (text.size() - index < length) {
			return false;
		}
		for (std::size_t offset = 1U; offset < length; ++offset) {
			const unsigned char byte = static_cast<unsigned char>(text[index + offset]);
			if ((byte & 0xC0U) != 0x80U) {
				return false;
			}
			value = (value << 6U) | (byte & 0x3FU);
		}
		if (value < minimum || (value >= 0xD800U && value <= 0xDFFFU) || value > 0x10FFFFU) {
			return false;
		}
		out.push_back(value);
		index += length;
	}
	return true;
}

void Encode(const std::vector<std::uint32_t> &points, std::string &out)
{
	out.clear();
	out.reserve(points.size());
	for (const std::uint32_t value : points) {
		if (value <= 0x7FU) {
			out.push_back(static_cast<char>(value));
		} else if (value <= 0x7FFU) {
			out.push_back(static_cast<char>(0xC0U | (value >> 6U)));
			out.push_back(static_cast<char>(0x80U | (value & 0x3FU)));
		} else if (value <= 0xFFFFU) {
			out.push_back(static_cast<char>(0xE0U | (value >> 12U)));
			out.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3FU)));
			out.push_back(static_cast<char>(0x80U | (value & 0x3FU)));
		} else {
			out.push_back(static_cast<char>(0xF0U | (value >> 18U)));
			out.push_back(static_cast<char>(0x80U | ((value >> 12U) & 0x3FU)));
			out.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3FU)));
			out.push_back(static_cast<char>(0x80U | (value & 0x3FU)));
		}
	}
}

template<typename Row, std::size_t Size> const Row *Find(const Row (&rows)[Size], std::uint32_t code)
{
	const Row *end = std::end(rows);
	const Row *found = std::lower_bound(std::begin(rows), end, code,
					    [](const Row &row, std::uint32_t value) { return row.code < value; });
	return found != end && found->code == code ? found : nullptr;
}

std::uint32_t CombiningClass(std::uint32_t code)
{
	if (code < 0x300U) {
		return 0U;
	}
	const ClassRow *row = Find(kCombiningClassRows, code);
	return row ? row->value : 0U;
}

// Canonical decomposition of every code point (decomposition tables are already fully recursive),
// then canonical ordering: a stable sort of each run of nonzero combining classes.
void Nfd(const std::vector<std::uint32_t> &in, std::vector<std::uint32_t> &out)
{
	out.clear();
	out.reserve(in.size());
	for (const std::uint32_t code : in) {
		if (code < 0xC0U) {
			out.push_back(code);
		} else if (code >= kHangulBase && code < kHangulBase + kSyllableCount) {
			const std::uint32_t index = code - kHangulBase;
			out.push_back(kLeadBase + index / (kVowelCount * kTrailCount));
			out.push_back(kVowelBase + (index % (kVowelCount * kTrailCount)) / kTrailCount);
			if (index % kTrailCount != 0U) {
				out.push_back(kTrailBase + index % kTrailCount);
			}
		} else if (const MapRow *row = Find(kDecompositionRows, code)) {
			out.insert(out.end(), kDecompositionData + row->offset,
				   kDecompositionData + row->offset + row->count);
		} else {
			out.push_back(code);
		}
	}

	for (std::size_t start = 0U; start < out.size();) {
		if (CombiningClass(out[start]) == 0U) {
			++start;
			continue;
		}
		std::size_t end = start;
		while (end < out.size() && CombiningClass(out[end]) != 0U) {
			++end;
		}
		std::stable_sort(out.begin() + static_cast<std::ptrdiff_t>(start),
				 out.begin() + static_cast<std::ptrdiff_t>(end),
				 [](std::uint32_t left, std::uint32_t right) {
					 return CombiningClass(left) < CombiningClass(right);
				 });
		start = end;
	}
}

void Fold(const std::vector<std::uint32_t> &in, std::vector<std::uint32_t> &out)
{
	out.clear();
	out.reserve(in.size());
	for (const std::uint32_t code : in) {
		if (code < 0x80U) {
			out.push_back(code >= 'A' && code <= 'Z' ? code + ('a' - 'A') : code);
		} else if (const MapRow *row = Find(kCaseFoldRows, code)) {
			out.insert(out.end(), kCaseFoldData + row->offset, kCaseFoldData + row->offset + row->count);
		} else {
			out.push_back(code);
		}
	}
}

} // namespace

bool ToNfd(std::string_view utf8, std::string &out)
{
	out.clear();
	std::vector<std::uint32_t> points;
	std::vector<std::uint32_t> result;
	if (!Decode(utf8, points)) {
		return false;
	}
	Nfd(points, result);
	Encode(result, out);
	return true;
}

bool ToCaseFold(std::string_view utf8, std::string &out)
{
	out.clear();
	std::vector<std::uint32_t> points;
	std::vector<std::uint32_t> result;
	if (!Decode(utf8, points)) {
		return false;
	}
	Fold(points, result);
	Encode(result, out);
	return true;
}

bool CollisionKey(std::string_view utf8, std::string &out)
{
	out.clear();
	std::vector<std::uint32_t> points;
	std::vector<std::uint32_t> first;
	std::vector<std::uint32_t> folded;
	if (!Decode(utf8, points)) {
		return false;
	}
	Nfd(points, first);
	Fold(first, folded);
	Nfd(folded, first);
	Encode(first, out);
	return true;
}

const char *UnicodeVersion() noexcept
{
	return kUnicodeVersion;
}

} // namespace Jeiobs::Workspace
