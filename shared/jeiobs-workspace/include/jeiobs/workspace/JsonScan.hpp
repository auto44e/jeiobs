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

enum class JsonValueKind {
	Null,
	Boolean,
	Number,
	String,
	Object,
	Array,
};

struct JsonValueOccurrence {
	std::string pointer;
	JsonValueKind kind = JsonValueKind::Null;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t content_start = 0U;
	std::size_t content_end = 0U;
	std::string string_value;
	bool boolean_value = false;
};

struct JsonStringOccurrence {
	std::string pointer;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::string value;
	std::size_t value_index = 0U;
};

struct JsonMemberOccurrence {
	std::string pointer;
	std::string key;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t key_start = 0U;
	std::size_t key_end = 0U;
	std::size_t value_index = 0U;
};

struct JsonScanResult {
	std::vector<JsonValueOccurrence> values;
	std::vector<JsonStringOccurrence> strings;
	std::vector<JsonMemberOccurrence> members;
	bool duplicate_key = false;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

class JsonScan {
public:
	[[nodiscard]] static JsonScanResult Scan(std::string_view bytes);
};

[[nodiscard]] JsonScanResult ScanJson(std::string_view bytes);

} // namespace Jeiobs::Workspace
