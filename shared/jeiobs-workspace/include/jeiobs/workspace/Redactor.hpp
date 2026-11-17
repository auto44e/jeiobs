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

enum class RedactOutcome {
	Stored,
	Omitted,
};

struct Redaction {
	std::string pointer;
	std::string kind;
	std::size_t start = 0U;
	std::size_t end = 0U;
	std::size_t replacement_size = 0U;
	bool has_was = false;
	bool was = false;
};

struct RedactReport {
	RedactOutcome outcome = RedactOutcome::Omitted;
	std::string reason;
	std::string fidelity;
	bool parse_invalid = false;
	std::vector<Redaction> redactions;
	std::size_t unreviewed_count = 0U;
};

struct RedactOptions {
	std::string ws_placeholder;
	bool has_ws_placeholder = false;
};

struct RedactResult {
	std::string bytes;
	RedactReport report;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

[[nodiscard]] bool IsRedactRole(std::string_view role) noexcept;

[[nodiscard]] RedactResult Redact(std::string_view role, std::string_view bytes, const RedactOptions &options = {});

class Redactor {
public:
	[[nodiscard]] static RedactResult Run(std::string_view role, std::string_view bytes,
					      const RedactOptions &options = {})
	{
		return Redact(role, bytes, options);
	}
};

} // namespace Jeiobs::Workspace
