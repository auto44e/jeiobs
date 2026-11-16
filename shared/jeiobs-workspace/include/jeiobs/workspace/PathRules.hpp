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

#include <string>
#include <string_view>

namespace Jeiobs::Workspace {

enum class PathStatus {
	Ok,
	Unchecked,
	Invalid,
};

// Length rules ("path", "components", "component") are reported as E_LIMIT, the rest as E_PATH.
[[nodiscard]] inline bool IsPathLimitRule(std::string_view rule) noexcept
{
	return rule == "path" || rule == "components" || rule == "component";
}

struct PathResult {
	PathStatus status = PathStatus::Invalid;
	std::string rule;

	[[nodiscard]] bool ok() const noexcept { return status != PathStatus::Invalid; }
};

[[nodiscard]] PathResult ValidatePath(std::string_view path);

[[nodiscard]] std::string AsciiFold(std::string_view value);

} // namespace Jeiobs::Workspace
