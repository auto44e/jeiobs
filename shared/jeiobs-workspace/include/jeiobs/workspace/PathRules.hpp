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

// The key two entries of one manifest must not share: `root`, a NUL byte, then the Unicode collision
// key of `path` (section 12 of WORKSPACE_FORMAT.md). A path that is not UTF-8 gets the byte 0xFF and
// its raw bytes instead, which no valid key can equal; ValidatePath rejects such a path anyway.
[[nodiscard]] std::string EntryCollisionKey(std::string_view root, std::string_view path);

} // namespace Jeiobs::Workspace
