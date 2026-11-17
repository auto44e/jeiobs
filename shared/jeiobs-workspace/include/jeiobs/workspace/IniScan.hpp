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
#pragma once

#include "jeiobs/workspace/Error.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace Jeiobs::Workspace {

struct IniSectionOccurrence {
	std::string name;
	std::size_t start = 0U;
	std::size_t end = 0U;
};

struct IniKeyOccurrence {
	// Compatibility fields remain available but are intentionally empty. Use
	// byte offsets and section_index to avoid per-key string materialization.
	std::string section;
	std::string key;
	std::string pointer;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t value_start = 0U;
	std::size_t value_end = 0U;
	std::size_t section_index = static_cast<std::size_t>(-1);
	std::size_t key_start = 0U;
	std::size_t key_end = 0U;
};

struct IniScanResult {
	bool has_bom = false;
	bool uncertain = false;
	std::vector<IniSectionOccurrence> sections;
	std::vector<IniKeyOccurrence> keys;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

class IniScan {
public:
	[[nodiscard]] static IniScanResult Scan(std::string_view bytes);
};

[[nodiscard]] IniScanResult ScanIni(std::string_view bytes);

} // namespace Jeiobs::Workspace
