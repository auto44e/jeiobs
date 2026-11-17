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
#include "jeiobs/workspace/Redactor.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::uint32_t NextRandom(std::uint32_t &state)
{
	state = state * 1664525U + 1013904223U;
	return state;
}

bool ReadFile(const std::filesystem::path &path, std::string &bytes)
{
	std::ifstream input(path, std::ios::in | std::ios::binary);
	if (!input.is_open()) {
		return false;
	}
	input.seekg(0, std::ios::end);
	const std::streamoff size = input.tellg();
	if (size < 0) {
		return false;
	}
	input.seekg(0, std::ios::beg);
	bytes.assign(static_cast<std::size_t>(size), '\0');
	if (!bytes.empty()) {
		input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
			return false;
		}
	}
	return true;
}

bool CheckResult(std::string_view input, const Jeiobs::Workspace::RedactResult &result)
{
	if (!result.ok()) {
		return false;
	}
	if (result.report.outcome != Jeiobs::Workspace::RedactOutcome::Stored) {
		return result.bytes.empty();
	}
	std::size_t expected = input.size();
	std::size_t previous_end = 0U;
	bool first = true;
	for (const Jeiobs::Workspace::Redaction &redaction : result.report.redactions) {
		if (redaction.start > redaction.end || redaction.end > input.size() ||
		    (!first && previous_end > redaction.start)) {
			return false;
		}
		expected -= redaction.end - redaction.start;
		expected += redaction.replacement_size;
		previous_end = redaction.end;
		first = false;
	}
	return expected == result.bytes.size();
}

} // namespace

int main()
{
	try {
		const std::filesystem::path fixture_root = std::filesystem::u8path(JW_CASES_DIR) / "fixtures";
		std::vector<std::filesystem::path> files;
		if (std::filesystem::exists(fixture_root)) {
			for (const std::filesystem::directory_entry &entry :
			     std::filesystem::recursive_directory_iterator(fixture_root)) {
				if (entry.is_regular_file()) {
					files.push_back(entry.path());
				}
			}
		}

		std::uint32_t state = 0x4a574632U;
		const char *roles[] = {"profile.service",
				       "profile.service.bak",
				       "profile.basic",
				       "module.obs-websocket",
				       "collection",
				       "collection.bak",
				       "profile.encoder.stream",
				       "profile.encoder.stream.bak",
				       "profile.encoder.record",
				       "profile.encoder.record.bak",
				       "app.modules",
				       "app.user"};
		for (const std::filesystem::path &path : files) {
			std::string seed;
			if (!ReadFile(path, seed)) {
				std::cerr << "cannot read " << path.u8string() << '\n';
				return 1;
			}
			for (int iteration = 0; iteration < 32; ++iteration) {
				std::string mutated = seed;
				if (!mutated.empty()) {
					const std::size_t index = NextRandom(state) % mutated.size();
					mutated[index] = static_cast<char>(NextRandom(state) & 0xffU);
					if ((NextRandom(state) & 3U) == 0U) {
						mutated.insert(mutated.begin() + static_cast<std::ptrdiff_t>(index),
							       static_cast<char>(NextRandom(state) & 0xffU));
					}
				} else {
					mutated.push_back(static_cast<char>(NextRandom(state) & 0xffU));
				}
				for (const char *role : roles) {
					const Jeiobs::Workspace::RedactResult result =
						Jeiobs::Workspace::Redact(role, mutated);
					if (!CheckResult(mutated, result)) {
						std::cerr << "invalid redactor result for " << path.u8string() << " as "
							  << role << '\n';
						return 1;
					}
				}
			}
		}
		return 0;
	} catch (...) {
		return 1;
	}
}
