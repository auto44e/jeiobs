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

#include <cstddef>

namespace Jeiobs::Workspace::Limits {

inline constexpr std::size_t kManifestBytes = 4U * 1024U * 1024U;
inline constexpr std::size_t kEntries = 10000U;
inline constexpr std::size_t kStringBytes = 64U * 1024U;
inline constexpr std::size_t kValues = 250000U;
inline constexpr std::size_t kDepth = 64U;
inline constexpr std::size_t kBlobBytes = 64U * 1024U * 1024U;
inline constexpr std::size_t kRevisionBytes = 256U * 1024U * 1024U;
inline constexpr std::size_t kDocumentBytes = 64U * 1024U * 1024U;
inline constexpr std::size_t kDocumentStringBytes = 16U * 1024U * 1024U;
inline constexpr std::size_t kDocumentValues = 1000000U;
inline constexpr std::size_t kIniLineBytes = 1024U * 1024U;
inline constexpr std::size_t kIniSections = 100000U;
inline constexpr std::size_t kIniKeys = 1000000U;
inline constexpr std::size_t kPathBytes = 512U;
inline constexpr std::size_t kPathComponents = 8U;
inline constexpr std::size_t kComponentBytes = 255U;

} // namespace Jeiobs::Workspace::Limits
