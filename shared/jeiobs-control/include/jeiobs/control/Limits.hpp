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

#include <cstddef>

namespace Jeiobs::Control::Limits {

inline constexpr std::size_t kMappingBytes = 4U * 1024U * 1024U;
inline constexpr std::size_t kWireBytes = 64U * 1024U;
inline constexpr std::size_t kBlobBytes = 64U * 1024U;
inline constexpr std::size_t kTextBytes = 4U * 1024U;
inline constexpr std::size_t kTupleElements = 256U;
inline constexpr std::size_t kTupleDepth = 4U;
inline constexpr std::size_t kAddressSegments = 16U;
inline constexpr std::size_t kSegmentBytes = 256U;
inline constexpr std::size_t kWildcardFanout = 64U;
inline constexpr std::size_t kEndpointQueue = 4096U;
inline constexpr std::size_t kEndpointQuota = 256U;
inline constexpr std::size_t kPrivilegedLane = 64U;
inline constexpr std::size_t kJsonDepth = 32U;
inline constexpr std::size_t kJsonStringBytes = 64U * 1024U;
inline constexpr std::size_t kJsonValues = 1'000'000U;
inline constexpr std::size_t kMappingSlots = 256U;
inline constexpr std::size_t kMappingRoutes = 10'000U;
inline constexpr std::size_t kTransforms = 16U;

} // namespace Jeiobs::Control::Limits
