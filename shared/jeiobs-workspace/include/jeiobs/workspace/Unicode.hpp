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

// The functions below take UTF-8 and write UTF-8. They return false, leaving `out` empty, when the
// input is not strict UTF-8 (no overlong forms, no surrogates, nothing above U+10FFFF). Unassigned
// code points pass through unchanged. They use the tables generated from the Unicode version that
// UnicodeVersion() reports.

// Canonical decomposition (NFD, Unicode section 3.11), including algorithmic Hangul.
[[nodiscard]] bool ToNfd(std::string_view utf8, std::string &out);

// Full case folding (CaseFolding.txt statuses C and F; no Turkic mappings), without normalization.
[[nodiscard]] bool ToCaseFold(std::string_view utf8, std::string &out);

// The path collision key, NFD(toCasefold(NFD(x))) (Unicode D145). Two paths collide when their keys
// are equal bytes.
[[nodiscard]] bool CollisionKey(std::string_view utf8, std::string &out);

[[nodiscard]] const char *UnicodeVersion() noexcept;

} // namespace Jeiobs::Workspace
