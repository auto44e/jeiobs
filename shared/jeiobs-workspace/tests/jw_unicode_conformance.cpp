/*
 * JeiOBS workspace snapshot support: Unicode conformance check.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * jw-unicode-conformance UCD_DIR checks the generated tables against the data files they came from
 * (fetched by tools/jeiobs/unicode/fetch-ucd.sh):
 *   - NormalizationTest.txt: ToNfd gives column 3 for columns 1 to 3 and column 5 for columns 4 and 5.
 *   - Every code point that NormalizationTest.txt part 1 does not list is unchanged by ToNfd, except
 *     the Hangul syllables, which are checked against the algorithm of Unicode section 3.12.
 *   - UnicodeData.txt: the combining class of every code point without a decomposition, by pairwise
 *     reordering against one mark of each class.
 *   - CaseFolding.txt: ToCaseFold gives each C and F mapping and leaves every other code point alone.
 *   - CollisionKey is NFD(fold(NFD(x))) for every single code point.
 */
#include "jeiobs/workspace/Unicode.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using Jeiobs::Workspace::CollisionKey;
using Jeiobs::Workspace::ToCaseFold;
using Jeiobs::Workspace::ToNfd;

namespace {

int g_failures = 0;

void Fail(const std::string &message)
{
	if (++g_failures <= 50) {
		std::fprintf(stderr, "FAIL %s\n", message.c_str());
	}
}

std::string Utf8(const std::vector<std::uint32_t> &points)
{
	std::string out;
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
	return out;
}

std::vector<std::uint32_t> Points(const std::string &field)
{
	std::vector<std::uint32_t> points;
	std::istringstream stream(field);
	std::string token;
	while (stream >> token) {
		points.push_back(static_cast<std::uint32_t>(std::stoul(token, nullptr, 16)));
	}
	return points;
}

std::string Hex(const std::string &bytes)
{
	std::string out;
	char buffer[4];
	for (const char byte : bytes) {
		std::snprintf(buffer, sizeof(buffer), "%02x", static_cast<unsigned>(static_cast<unsigned char>(byte)));
		out += buffer;
	}
	return out;
}

void ExpectNfd(const std::string &what, const std::string &input, const std::string &expected)
{
	std::string actual;
	if (!ToNfd(input, actual) || actual != expected) {
		Fail(what + ": NFD(" + Hex(input) + ") = " + Hex(actual) + ", expected " + Hex(expected));
	}
}

bool IsSurrogate(std::uint32_t code)
{
	return code >= 0xD800U && code <= 0xDFFFU;
}

bool IsHangulSyllable(std::uint32_t code)
{
	return code >= 0xAC00U && code <= 0xD7A3U;
}

void CheckNormalizationTest(const std::string &dir, std::set<std::uint32_t> &part1_singles)
{
	std::ifstream file(dir + "/NormalizationTest.txt");
	if (!file) {
		Fail("cannot open NormalizationTest.txt");
		return;
	}
	std::string line;
	bool part1 = false;
	std::size_t lines = 0U;
	while (std::getline(file, line)) {
		if (line.rfind("@Part", 0) == 0) {
			part1 = line.rfind("@Part1", 0) == 0;
			continue;
		}
		const std::size_t comment = line.find('#');
		if (comment != std::string::npos) {
			line.resize(comment);
		}
		if (line.find_first_not_of(" \t\r") == std::string::npos) {
			continue;
		}
		std::vector<std::string> columns;
		std::istringstream stream(line);
		std::string column;
		while (std::getline(stream, column, ';')) {
			columns.push_back(column);
		}
		if (columns.size() < 5U) {
			Fail("bad line: " + line);
			continue;
		}
		++lines;
		const std::string c1 = Utf8(Points(columns[0]));
		const std::string c2 = Utf8(Points(columns[1]));
		const std::string c3 = Utf8(Points(columns[2]));
		const std::string c4 = Utf8(Points(columns[3]));
		const std::string c5 = Utf8(Points(columns[4]));
		const std::string what = "line " + std::to_string(lines);
		ExpectNfd(what + " c1", c1, c3);
		ExpectNfd(what + " c2", c2, c3);
		ExpectNfd(what + " c3", c3, c3);
		ExpectNfd(what + " c4", c4, c5);
		ExpectNfd(what + " c5", c5, c5);
		const std::vector<std::uint32_t> single = Points(columns[0]);
		if (part1 && single.size() == 1U) {
			part1_singles.insert(single[0]);
		}
	}
	std::printf("NormalizationTest.txt: %zu lines\n", lines);
}

void CheckHangul()
{
	for (std::uint32_t code = 0xAC00U; code <= 0xD7A3U; ++code) {
		const std::uint32_t index = code - 0xAC00U;
		std::vector<std::uint32_t> expected = {0x1100U + index / 588U, 0x1161U + (index % 588U) / 28U};
		if (index % 28U != 0U) {
			expected.push_back(0x11A7U + index % 28U);
		}
		ExpectNfd("hangul " + std::to_string(code), Utf8({code}), Utf8(expected));
	}
}

// Canonical combining classes, from UnicodeData.txt: for every code point without a decomposition and
// every class that occurs, "a" + code point + a mark of that class is reordered exactly when the code
// point's class is greater than the mark's (a starter never moves). This tests each class value of the
// table against an independent reading of UnicodeData.txt, which single-code-point checks cannot.
void CheckCombiningClasses(const std::string &dir)
{
	std::ifstream file(dir + "/UnicodeData.txt");
	if (!file) {
		Fail("cannot open UnicodeData.txt");
		return;
	}
	struct Row {
		std::uint32_t code;
		unsigned value;
	};
	std::vector<Row> rows;
	std::map<unsigned, std::uint32_t> representative;
	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> fields;
		std::istringstream stream(line);
		std::string field;
		while (std::getline(stream, field, ';')) {
			fields.push_back(field);
		}
		// Range markers (Hangul syllables decompose algorithmically) are not single characters.
		if (fields.size() < 6U || !fields[5].empty() || fields[1].find(", First>") != std::string::npos ||
		    fields[1].find(", Last>") != std::string::npos) {
			continue;
		}
		const std::uint32_t code = static_cast<std::uint32_t>(std::stoul(fields[0], nullptr, 16));
		const unsigned value = static_cast<unsigned>(std::stoul(fields[3]));
		rows.push_back({code, value});
		if (value != 0U && representative.count(value) == 0U) {
			representative[value] = code;
		}
	}
	std::size_t checks = 0U;
	for (const Row &row : rows) {
		for (const auto &[value, mark] : representative) {
			if (row.code == mark) {
				continue;
			}
			const bool swapped = row.value > value;
			const std::string expected = swapped ? Utf8({'a', mark, row.code})
							     : Utf8({'a', row.code, mark});
			ExpectNfd("class U+" + std::to_string(row.code) + " vs " + std::to_string(value),
				  Utf8({'a', row.code, mark}), expected);
			++checks;
		}
	}
	std::printf("UnicodeData.txt: %zu combining class checks, %zu classes\n", checks, representative.size());
}

void CheckUnlisted(const std::set<std::uint32_t> &listed)
{
	for (std::uint32_t code = 0U; code <= 0x10FFFFU; ++code) {
		if (IsSurrogate(code) || IsHangulSyllable(code) || listed.count(code) != 0U) {
			continue;
		}
		ExpectNfd("unlisted U+" + std::to_string(code), Utf8({code}), Utf8({code}));
	}
}

void CheckCaseFolding(const std::string &dir)
{
	std::ifstream file(dir + "/CaseFolding.txt");
	if (!file) {
		Fail("cannot open CaseFolding.txt");
		return;
	}
	std::map<std::uint32_t, std::vector<std::uint32_t>> folds;
	std::string line;
	while (std::getline(file, line)) {
		const std::size_t comment = line.find('#');
		if (comment != std::string::npos) {
			line.resize(comment);
		}
		std::vector<std::string> columns;
		std::istringstream stream(line);
		std::string column;
		while (std::getline(stream, column, ';')) {
			columns.push_back(column);
		}
		if (columns.size() < 3U) {
			continue;
		}
		const std::string status = columns[1].substr(columns[1].find_first_not_of(' '), 1U);
		if (status == "C" || status == "F") {
			folds[Points(columns[0])[0]] = Points(columns[2]);
		}
	}
	for (std::uint32_t code = 0U; code <= 0x10FFFFU; ++code) {
		if (IsSurrogate(code)) {
			continue;
		}
		const auto found = folds.find(code);
		const std::string expected =
			Utf8(found == folds.end() ? std::vector<std::uint32_t>{code} : found->second);
		std::string actual;
		if (!ToCaseFold(Utf8({code}), actual) || actual != expected) {
			Fail("fold U+" + std::to_string(code) + " = " + Hex(actual) + ", expected " + Hex(expected));
		}
		std::string nfd1;
		std::string fold;
		std::string nfd2;
		std::string key;
		if (!ToNfd(Utf8({code}), nfd1) || !ToCaseFold(nfd1, fold) || !ToNfd(fold, nfd2) ||
		    !CollisionKey(Utf8({code}), key) || key != nfd2) {
			Fail("key U+" + std::to_string(code));
		}
	}
	std::printf("CaseFolding.txt: %zu C/F mappings\n", folds.size());
}

} // namespace

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::fprintf(stderr, "usage: jw-unicode-conformance UCD_DIR\n");
		return 2;
	}
	std::printf("tables: Unicode %s\n", Jeiobs::Workspace::UnicodeVersion());
	std::set<std::uint32_t> part1_singles;
	CheckNormalizationTest(argv[1], part1_singles);
	CheckHangul();
	CheckCombiningClasses(argv[1]);
	CheckUnlisted(part1_singles);
	CheckCaseFolding(argv[1]);
	if (g_failures != 0) {
		std::fprintf(stderr, "%d failures\n", g_failures);
		return 1;
	}
	std::printf("OK\n");
	return 0;
}
