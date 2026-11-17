/*
 * JeiOBS control graph support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>

namespace Jeiobs::Control {

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
	Major,
	Schema,
	Duplicate,
	Reference,
	Denied,
	WildcardPrivileged,
	Fanout,
	AmbiguousMerge,
	Cycle,
	Value,
	Usage,
};

struct Error {
	ErrorCode code = ErrorCode::None;
	std::string detail;

	[[nodiscard]] bool ok() const noexcept { return code == ErrorCode::None; }
};

[[nodiscard]] const char *ErrorCodeName(ErrorCode code) noexcept;

template<typename T> struct Result {
	std::optional<T> value;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok() && value.has_value(); }
};

template<> struct Result<void> {
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

[[nodiscard]] inline Error MakeError(ErrorCode code, std::string detail = {})
{
	return {code, std::move(detail)};
}

} // namespace Jeiobs::Control
