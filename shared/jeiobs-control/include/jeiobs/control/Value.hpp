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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Jeiobs::Control {

struct UInt128 {
	std::uint64_t high = 0U;
	std::uint64_t low = 0U;

	[[nodiscard]] bool operator==(const UInt128 &other) const noexcept
	{
		return high == other.high && low == other.low;
	}
	[[nodiscard]] bool operator!=(const UInt128 &other) const noexcept { return !(*this == other); }
};

struct Int128 {
	std::uint64_t high = 0U;
	std::uint64_t low = 0U;

	[[nodiscard]] bool operator==(const Int128 &other) const noexcept
	{
		return high == other.high && low == other.low;
	}
	[[nodiscard]] bool operator!=(const Int128 &other) const noexcept { return !(*this == other); }
};

enum class ValueKind {
	Invalid,
	Trigger,
	Bool,
	UInt,
	Int,
	FloatBits,
	Text,
	Blob,
	Tuple,
};

enum class FloatFormat {
	Ieee32,
	Ieee64,
};

enum class RelativeEncoding {
	TwosComplement,
	SignMagnitude,
	BinaryOffset,
};

struct Relative {
	RelativeEncoding encoding = RelativeEncoding::TwosComplement;
	std::string step;

	[[nodiscard]] bool operator==(const Relative &other) const noexcept
	{
		return encoding == other.encoding && step == other.step;
	}
};

class Value final {
public:
	Value() = default;

	[[nodiscard]] static Result<Value> MakeTrigger(std::string unit = {});
	[[nodiscard]] static Result<Value> MakeBool(bool value, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeUInt(unsigned int width_bits, UInt128 raw, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeInt(unsigned int width_bits, Int128 raw, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeFloatBits(FloatFormat format, std::uint64_t raw, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeText(std::string text, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeBlob(std::vector<std::uint8_t> bytes, std::string unit = {});
	[[nodiscard]] static Result<Value> MakeTuple(std::vector<Value> values, std::string unit = {});

	[[nodiscard]] static Result<Value> MakeUIntDecimal(unsigned int width_bits, const std::string &decimal,
							   std::string unit = {});
	[[nodiscard]] static Result<Value> MakeIntDecimal(unsigned int width_bits, const std::string &decimal,
							  std::string unit = {});
	[[nodiscard]] Result<Value> WithRelative(Relative relative) const;

	[[nodiscard]] static Result<Value> FromDoubleInCodeSpace(const Value &value, double number);

	[[nodiscard]] ValueKind kind() const noexcept { return kind_; }
	[[nodiscard]] unsigned int widthBits() const noexcept { return width_bits_; }
	[[nodiscard]] FloatFormat floatFormat() const noexcept { return float_format_; }
	[[nodiscard]] bool boolValue() const noexcept { return bool_value_; }
	[[nodiscard]] const UInt128 &uintRaw() const noexcept { return uint_raw_; }
	[[nodiscard]] const Int128 &intRaw() const noexcept { return int_raw_; }
	[[nodiscard]] std::uint64_t floatRaw() const noexcept { return float_raw_; }
	[[nodiscard]] const std::string &text() const noexcept { return text_; }
	[[nodiscard]] const std::vector<std::uint8_t> &blob() const noexcept { return blob_; }
	[[nodiscard]] const std::vector<Value> &tuple() const noexcept { return tuple_; }
	[[nodiscard]] const std::string &unit() const noexcept { return unit_; }
	[[nodiscard]] const std::optional<Relative> &relative() const noexcept { return relative_; }

	[[nodiscard]] double AsDouble() const noexcept;
	[[nodiscard]] bool operator==(const Value &other) const noexcept;
	[[nodiscard]] bool operator!=(const Value &other) const noexcept { return !(*this == other); }

private:
	ValueKind kind_ = ValueKind::Invalid;
	unsigned int width_bits_ = 0U;
	FloatFormat float_format_ = FloatFormat::Ieee32;
	bool bool_value_ = false;
	UInt128 uint_raw_;
	Int128 int_raw_;
	std::uint64_t float_raw_ = 0U;
	std::string text_;
	std::vector<std::uint8_t> blob_;
	std::vector<Value> tuple_;
	std::string unit_;
	std::optional<Relative> relative_;

	[[nodiscard]] static Result<Value> Finish(Value value, std::string unit);
};

} // namespace Jeiobs::Control
