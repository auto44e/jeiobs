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
#include "jeiobs/control/Control.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace Jeiobs::Control {
namespace {

bool IsValidUtf8(std::string_view bytes)
{
	for (std::size_t index = 0U; index < bytes.size();) {
		const unsigned char first = static_cast<unsigned char>(bytes[index]);
		std::size_t width = 0U;
		std::uint32_t value = 0U;
		if (first <= 0x7fU) {
			width = 1U;
			value = first;
		} else if (first >= 0xc2U && first <= 0xdfU) {
			width = 2U;
			value = first & 0x1fU;
		} else if (first >= 0xe0U && first <= 0xefU) {
			width = 3U;
			value = first & 0x0fU;
		} else if (first >= 0xf0U && first <= 0xf4U) {
			width = 4U;
			value = first & 0x07U;
		} else {
			return false;
		}
		if (index + width > bytes.size()) {
			return false;
		}
		for (std::size_t offset = 1U; offset < width; ++offset) {
			const unsigned char continuation = static_cast<unsigned char>(bytes[index + offset]);
			if ((continuation & 0xc0U) != 0x80U) {
				return false;
			}
			value = (value << 6U) | (continuation & 0x3fU);
		}
		if ((width == 2U && value < 0x80U) || (width == 3U && value < 0x800U) ||
		    (width == 4U && value < 0x10000U) || (value >= 0xd800U && value <= 0xdfffU) || value > 0x10ffffU) {
			return false;
		}
		index += width;
	}
	return true;
}

Result<std::string> ValidateSegmentText(std::string value)
{
	try {
		if (value.size() > Limits::kSegmentBytes) {
			return {{}, MakeError(ErrorCode::Limit, "segment")};
		}
		if (value.find('\0') != std::string::npos) {
			return {{}, MakeError(ErrorCode::Nul)};
		}
		if (!IsValidUtf8(value)) {
			return {{}, MakeError(ErrorCode::Utf8)};
		}
		return {std::move(value), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

} // namespace

const char *ErrorCodeName(ErrorCode code) noexcept
{
	switch (code) {
	case ErrorCode::None:
		return "OK";
	case ErrorCode::Io:
		return "E_IO";
	case ErrorCode::Limit:
		return "E_LIMIT";
	case ErrorCode::Json:
		return "E_JSON";
	case ErrorCode::DuplicateKey:
		return "E_DUPKEY";
	case ErrorCode::Utf8:
		return "E_UTF8";
	case ErrorCode::Nul:
		return "E_NUL";
	case ErrorCode::Nonfinite:
		return "E_NONFINITE";
	case ErrorCode::Bom:
		return "E_BOM";
	case ErrorCode::Major:
		return "E_MAJOR";
	case ErrorCode::Schema:
		return "E_SCHEMA";
	case ErrorCode::Duplicate:
		return "E_DUP";
	case ErrorCode::Reference:
		return "E_REF";
	case ErrorCode::Denied:
		return "E_DENIED";
	case ErrorCode::WildcardPrivileged:
		return "E_WILDCARD_PRIVILEGED";
	case ErrorCode::Fanout:
		return "E_FANOUT";
	case ErrorCode::AmbiguousMerge:
		return "E_AMBIGUOUS_MERGE";
	case ErrorCode::Cycle:
		return "E_CYCLE";
	case ErrorCode::Value:
		return "E_VALUE";
	case ErrorCode::Usage:
		return "E_USAGE";
	}
	return "E_UNKNOWN";
}

Result<AddressSegment> AddressSegment::Symbol(std::string value)
{
	auto checked = ValidateSegmentText(std::move(value));
	if (!checked.ok()) {
		return {{}, checked.error};
	}
	AddressSegment result;
	result.kind = AddressSegmentKind::Symbol;
	result.text = std::move(*checked.value);
	return {std::move(result), {}};
}

Result<AddressSegment> AddressSegment::Int(std::uint64_t value)
{
	AddressSegment result;
	result.kind = AddressSegmentKind::Int;
	result.integer = value;
	return {std::move(result), {}};
}

Result<AddressSegment> AddressSegment::Text(std::string value)
{
	auto checked = ValidateSegmentText(std::move(value));
	if (!checked.ok()) {
		return {{}, checked.error};
	}
	AddressSegment result;
	result.kind = AddressSegmentKind::Text;
	result.text = std::move(*checked.value);
	return {std::move(result), {}};
}

bool AddressSegment::operator==(const AddressSegment &other) const noexcept
{
	return kind == other.kind && integer == other.integer && text == other.text;
}

Result<PatternSegment> PatternSegment::Symbol(std::string value)
{
	auto checked = ValidateSegmentText(std::move(value));
	if (!checked.ok()) {
		return {{}, checked.error};
	}
	PatternSegment result;
	result.kind = PatternSegmentKind::Symbol;
	result.text = std::move(*checked.value);
	return {std::move(result), {}};
}

Result<PatternSegment> PatternSegment::Int(std::uint64_t value)
{
	PatternSegment result;
	result.kind = PatternSegmentKind::Int;
	result.low = value;
	result.high = value;
	return {std::move(result), {}};
}

Result<PatternSegment> PatternSegment::Text(std::string value)
{
	auto checked = ValidateSegmentText(std::move(value));
	if (!checked.ok()) {
		return {{}, checked.error};
	}
	PatternSegment result;
	result.kind = PatternSegmentKind::Text;
	result.text = std::move(*checked.value);
	return {std::move(result), {}};
}

PatternSegment PatternSegment::Any() noexcept
{
	PatternSegment result;
	result.kind = PatternSegmentKind::Any;
	return result;
}

Result<PatternSegment> PatternSegment::Range(std::uint64_t low, std::uint64_t high)
{
	if (low > high) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	PatternSegment result;
	result.kind = PatternSegmentKind::Range;
	result.low = low;
	result.high = high;
	return {std::move(result), {}};
}

Result<PatternSegment> PatternSegment::Set(std::vector<std::uint64_t> values)
{
	try {
		if (values.empty() || values.size() > 64U) {
			return {{}, MakeError(ErrorCode::Limit, "set")};
		}
		std::sort(values.begin(), values.end());
		if (std::adjacent_find(values.begin(), values.end()) != values.end()) {
			return {{}, MakeError(ErrorCode::Value)};
		}
		PatternSegment result;
		result.kind = PatternSegmentKind::Set;
		result.set = std::move(values);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<ControlAddress> ControlAddress::Create(std::vector<AddressSegment> values)
{
	try {
		if (values.size() > Limits::kAddressSegments) {
			return {{}, MakeError(ErrorCode::Limit, "address")};
		}
		ControlAddress result;
		result.segments = std::move(values);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<ControlPattern> ControlPattern::Create(std::vector<PatternSegment> values)
{
	try {
		if (values.size() > Limits::kAddressSegments) {
			return {{}, MakeError(ErrorCode::Limit, "address")};
		}
		ControlPattern result;
		result.segments = std::move(values);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

bool Matches(const ControlPattern &pattern, const ControlAddress &address) noexcept
{
	return Matches(pattern.segments, address.segments);
}

bool Matches(const std::vector<PatternSegment> &pattern, const std::vector<AddressSegment> &address) noexcept
{
	if (pattern.size() != address.size()) {
		return false;
	}
	for (std::size_t index = 0U; index < pattern.size(); ++index) {
		const PatternSegment &candidate = pattern[index];
		const AddressSegment &actual = address[index];
		switch (candidate.kind) {
		case PatternSegmentKind::Symbol:
			if (actual.kind != AddressSegmentKind::Symbol || candidate.text != actual.text) {
				return false;
			}
			break;
		case PatternSegmentKind::Int:
			if (actual.kind != AddressSegmentKind::Int || candidate.low != actual.integer) {
				return false;
			}
			break;
		case PatternSegmentKind::Text:
			if (actual.kind != AddressSegmentKind::Text || candidate.text != actual.text) {
				return false;
			}
			break;
		case PatternSegmentKind::Any:
			break;
		case PatternSegmentKind::Range:
			if (actual.kind != AddressSegmentKind::Int || actual.integer < candidate.low ||
			    actual.integer > candidate.high) {
				return false;
			}
			break;
		case PatternSegmentKind::Set:
			if (actual.kind != AddressSegmentKind::Int ||
			    std::find(candidate.set.begin(), candidate.set.end(), actual.integer) ==
				    candidate.set.end()) {
				return false;
			}
			break;
		}
	}
	return true;
}

Result<Principal> Principal::Create(TrustClass trust, std::string endpoint_id, std::string peer_id)
{
	try {
		if (endpoint_id.size() > Limits::kSegmentBytes || peer_id.size() > Limits::kSegmentBytes) {
			return {{}, MakeError(ErrorCode::Limit, "principal")};
		}
		if (endpoint_id.find('\0') != std::string::npos || peer_id.find('\0') != std::string::npos) {
			return {{}, MakeError(ErrorCode::Nul)};
		}
		if (!IsValidUtf8(endpoint_id) || !IsValidUtf8(peer_id)) {
			return {{}, MakeError(ErrorCode::Utf8)};
		}
		Principal result;
		result.trust = trust;
		result.endpoint_id = std::move(endpoint_id);
		result.peer_id = std::move(peer_id);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<WireEnvelope> WireEnvelope::Create(std::string protocol, std::vector<std::uint8_t> data,
					  std::size_t packet_total)
{
	try {
		if (protocol.size() > Limits::kSegmentBytes) {
			return {{}, MakeError(ErrorCode::Limit, "protocol")};
		}
		if (data.size() > Limits::kWireBytes) {
			return {{}, MakeError(ErrorCode::Limit, "wire")};
		}
		if (protocol.find('\0') != std::string::npos) {
			return {{}, MakeError(ErrorCode::Nul)};
		}
		if (!IsValidUtf8(protocol)) {
			return {{}, MakeError(ErrorCode::Utf8)};
		}
		WireEnvelope result;
		result.protocol = std::move(protocol);
		result.bytes = std::move(data);
		result.packet_count = packet_total;
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Event> Event::Create(std::optional<WireEnvelope> wire, ControlAddress control_address, Value control_value,
			    Principal control_principal)
{
	try {
		if (control_value.kind() == ValueKind::Invalid) {
			return {{}, MakeError(ErrorCode::Value)};
		}
		Event result;
		result.envelope = std::move(wire);
		result.address = std::move(control_address);
		result.value = std::move(control_value);
		result.principal = std::move(control_principal);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

} // namespace Jeiobs::Control
