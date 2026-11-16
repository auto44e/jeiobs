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

namespace Jeiobs::Workspace {

enum class ErrorCode {
	None,
	Io,
	Limit,
	Json,
	DuplicateKey,
	Utf8,
	Nul,
	Nonfinite,
	Bom,
	Schema,
	Path,
	Collision,
	Incompat,
	Major,
	RoCompat,
	Undecided,
	Name,
	Revid,
	Seq,
};

struct Error {
	ErrorCode code = ErrorCode::None;
	std::string detail;

	[[nodiscard]] bool ok() const noexcept { return code == ErrorCode::None; }
};

[[nodiscard]] const char *ErrorCodeName(ErrorCode code) noexcept;

} // namespace Jeiobs::Workspace
