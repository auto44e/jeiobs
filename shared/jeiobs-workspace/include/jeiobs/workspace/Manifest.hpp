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

#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace Jeiobs::Workspace {

enum class ManifestMode {
	Local,
	Export,
};

using Json = nlohmann::json;

struct Manifest {
	Json document;
	ManifestMode mode = ManifestMode::Local;
	bool readonly = false;
	bool roCompat = false;
	bool uncheckedPaths = false;
};

struct ManifestResult {
	std::optional<Manifest> value;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

struct JsonResult {
	std::optional<Json> value;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

struct BytesResult {
	std::string bytes;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

[[nodiscard]] BytesResult ReadFile(const std::string &path);

[[nodiscard]] JsonResult ParseJson(std::string_view bytes);

[[nodiscard]] ManifestResult ParseManifest(std::string_view bytes, ManifestMode mode);

[[nodiscard]] ManifestResult LoadManifest(const std::string &path, ManifestMode mode);

[[nodiscard]] JsonResult CarryOver(const Manifest &parent, const Manifest &draft);

[[nodiscard]] JsonResult ProjectForExport(const Manifest &manifest, std::string_view bytesHash, const Json &decisions,
					  std::string_view now);

} // namespace Jeiobs::Workspace
