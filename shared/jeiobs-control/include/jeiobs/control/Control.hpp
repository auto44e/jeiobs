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

#include "jeiobs/control/Error.hpp"
#include "jeiobs/control/Limits.hpp"
#include "jeiobs/control/Value.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Jeiobs::Control {

enum class AddressSegmentKind {
	Symbol,
	Int,
	Text,
};

struct AddressSegment {
	AddressSegmentKind kind = AddressSegmentKind::Symbol;
	std::uint64_t integer = 0U;
	std::string text;

	[[nodiscard]] static Result<AddressSegment> Symbol(std::string value);
	[[nodiscard]] static Result<AddressSegment> Int(std::uint64_t value);
	[[nodiscard]] static Result<AddressSegment> Text(std::string value);
	[[nodiscard]] bool operator==(const AddressSegment &other) const noexcept;
};

enum class PatternSegmentKind {
	Symbol,
	Int,
	Text,
	Any,
	Range,
	Set,
};

struct PatternSegment {
	PatternSegmentKind kind = PatternSegmentKind::Any;
	std::uint64_t low = 0U;
	std::uint64_t high = 0U;
	std::vector<std::uint64_t> set;
	std::string text;

	[[nodiscard]] static Result<PatternSegment> Symbol(std::string value);
	[[nodiscard]] static Result<PatternSegment> Int(std::uint64_t value);
	[[nodiscard]] static Result<PatternSegment> Text(std::string value);
	[[nodiscard]] static PatternSegment Any() noexcept;
	[[nodiscard]] static Result<PatternSegment> Range(std::uint64_t low, std::uint64_t high);
	[[nodiscard]] static Result<PatternSegment> Set(std::vector<std::uint64_t> values);
};

struct ControlAddress {
	std::vector<AddressSegment> segments;

	[[nodiscard]] static Result<ControlAddress> Create(std::vector<AddressSegment> segments);
};

struct ControlPattern {
	std::vector<PatternSegment> segments;

	[[nodiscard]] static Result<ControlPattern> Create(std::vector<PatternSegment> segments);
};

[[nodiscard]] bool Matches(const ControlPattern &pattern, const ControlAddress &address) noexcept;
[[nodiscard]] bool Matches(const std::vector<PatternSegment> &pattern,
			   const std::vector<AddressSegment> &address) noexcept;

enum class TrustClass {
	Hw,
	Virtual,
	LoopbackNet,
	RemoteNet,
	LocalUi,
};

struct Principal {
	TrustClass trust = TrustClass::Hw;
	std::string endpoint_id;
	std::string peer_id;

	[[nodiscard]] static Result<Principal> Create(TrustClass trust, std::string endpoint_id,
						      std::string peer_id = {});
};

struct WireEnvelope {
	std::string protocol;
	std::vector<std::uint8_t> bytes;
	std::size_t packet_count = 0U;

	[[nodiscard]] static Result<WireEnvelope> Create(std::string protocol, std::vector<std::uint8_t> bytes,
							 std::size_t packet_count);
};

struct Event {
	std::optional<WireEnvelope> envelope;
	ControlAddress address;
	Value value;
	Principal principal;

	[[nodiscard]] static Result<Event> Create(std::optional<WireEnvelope> envelope, ControlAddress address,
						  Value value, Principal principal);
};

} // namespace Jeiobs::Control
