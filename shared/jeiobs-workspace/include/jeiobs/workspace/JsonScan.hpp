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

inline constexpr std::size_t kJsonNoIndex = static_cast<std::size_t>(-1);

enum class JsonValueKind {
	Null,
	Boolean,
	Number,
	String,
	Object,
	Array,
};

struct JsonValueOccurrence {
	// Compatibility field; scanners keep parent/index metadata and do not
	// materialize pointers for every value.
	std::string pointer;
	JsonValueKind kind = JsonValueKind::Null;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t content_start = 0U;
	std::size_t content_end = 0U;
	std::string string_value;
	bool boolean_value = false;
	std::size_t parent_index = kJsonNoIndex;
	std::size_t member_index = kJsonNoIndex;
	std::size_t array_index = kJsonNoIndex;
};

struct JsonStringOccurrence {
	// Compatibility field; report pointers are built only when needed.
	std::string pointer;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::string value;
	std::size_t value_index = kJsonNoIndex;
};

struct JsonMemberOccurrence {
	// Compatibility field; report pointers are built only when needed.
	std::string pointer;
	std::string key;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t key_start = 0U;
	std::size_t key_end = 0U;
	std::size_t value_index = kJsonNoIndex;
	std::size_t parent_index = kJsonNoIndex;
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
