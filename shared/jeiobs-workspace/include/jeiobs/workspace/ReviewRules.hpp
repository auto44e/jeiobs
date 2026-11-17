/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 or later.
 */
#pragma once

#include "jeiobs/workspace/Error.hpp"

#include <cstddef>
#include <string_view>

namespace Jeiobs::Workspace {

struct ReviewResult {
	std::size_t unreviewed_count = 0U;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

class ReviewRules {
public:
	[[nodiscard]] static bool IsFirstPartySource(std::string_view source_type) noexcept;
	[[nodiscard]] static ReviewResult AnalyzeCollection(std::string_view bytes);
};

[[nodiscard]] bool IsFirstPartySourceType(std::string_view source_type) noexcept;

} // namespace Jeiobs::Workspace
