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

#include <string>
#include <string_view>

namespace Jeiobs::Workspace {

struct HashResult {
	std::string value;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

[[nodiscard]] HashResult HashBytes(std::string_view bytes);

[[nodiscard]] HashResult HashFile(const std::string &path);

} // namespace Jeiobs::Workspace
