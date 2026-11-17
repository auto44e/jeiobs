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
#include "jeiobs/control/Value.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <utility>

namespace Jeiobs::Control {
namespace {

constexpr std::uint64_t kMaxUint64 = std::numeric_limits<std::uint64_t>::max();
constexpr double kTwoTo64 = 18446744073709551616.0;
constexpr double kTwoTo128 = 340282366920938463463374607431768211456.0;

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

bool IsPositiveDecimal(std::string_view value)
{
	if (value.empty()) {
		return false;
	}
	bool has_digit = false;
	bool has_nonzero = false;
	bool has_dot = false;
	for (const char character : value) {
		if (character == '.' && !has_dot) {
			has_dot = true;
			continue;
		}
		if (character < '0' || character > '9') {
			return false;
		}
		has_digit = true;
		has_nonzero = has_nonzero || character != '0';
	}
	return has_digit && has_nonzero;
}

bool IsIntegerDecimal(std::string_view value)
{
	if (value.empty()) {
		return false;
	}
	std::size_t start = 0U;
	if (value[0] == '+' || value[0] == '-') {
		start = 1U;
	}
	if (start == value.size()) {
		return false;
	}
	for (std::size_t index = start; index < value.size(); ++index) {
		if (value[index] < '0' || value[index] > '9') {
			return false;
		}
	}
	return true;
}

bool IsLess(UInt128 left, UInt128 right) noexcept
{
	return left.high < right.high || (left.high == right.high && left.low < right.low);
}

unsigned int BitLength(UInt128 value) noexcept
{
	if (value.high != 0U) {
		unsigned int length = 64U;
		std::uint64_t high = value.high;
		while (high != 0U) {
			++length;
			high >>= 1U;
		}
		return length;
	}
	unsigned int length = 0U;
	std::uint64_t low = value.low;
	while (low != 0U) {
		++length;
		low >>= 1U;
	}
	return length;
}

bool BitAt(UInt128 value, unsigned int bit) noexcept
{
	if (bit >= 64U) {
		return ((value.high >> (bit - 64U)) & 1U) != 0U;
	}
	return ((value.low >> bit) & 1U) != 0U;
}

bool HasLowerBits(UInt128 value, unsigned int count) noexcept
{
	if (count == 0U) {
		return false;
	}
	if (count >= 64U && value.low != 0U) {
		return true;
	}
	if (count <= 64U) {
		const std::uint64_t mask = count == 64U ? kMaxUint64 : (std::uint64_t(1U) << count) - 1U;
		return (value.low & mask) != 0U;
	}
	const unsigned int high_count = count - 64U;
	const std::uint64_t mask = high_count == 64U ? kMaxUint64 : (std::uint64_t(1U) << high_count) - 1U;
	return (value.high & mask) != 0U;
}

double UInt128ToDouble(UInt128 value) noexcept
{
	const unsigned int bit_length = BitLength(value);
	if (bit_length == 0U) {
		return 0.0;
	}
	if (bit_length <= 53U) {
		return static_cast<double>(value.low);
	}

	const unsigned int shift = bit_length - 53U;
	std::uint64_t significand = 0U;
	if (shift >= 64U) {
		significand = value.high >> (shift - 64U);
	} else {
		significand = (value.high << (64U - shift)) | (value.low >> shift);
	}

	const bool halfway = BitAt(value, shift - 1U);
	const bool sticky = HasLowerBits(value, shift - 1U);
	if (halfway && (sticky || (significand & 1U) != 0U)) {
		++significand;
	}
	return std::ldexp(static_cast<double>(significand), static_cast<int>(shift));
}

UInt128 BitMask(unsigned int bits) noexcept
{
	if (bits >= 128U) {
		return {kMaxUint64, kMaxUint64};
	}
	if (bits == 64U) {
		return {0U, kMaxUint64};
	}
	if (bits > 64U) {
		return {(std::uint64_t(1U) << (bits - 64U)) - 1U, kMaxUint64};
	}
	if (bits == 0U) {
		return {};
	}
	return {0U, (std::uint64_t(1U) << bits) - 1U};
}

bool Fits(UInt128 value, unsigned int bits) noexcept
{
	const UInt128 mask = BitMask(bits);
	return (value.high & ~mask.high) == 0U && (value.low & ~mask.low) == 0U;
}

UInt128 Masked(UInt128 value, unsigned int bits) noexcept
{
	const UInt128 mask = BitMask(bits);
	return {value.high & mask.high, value.low & mask.low};
}

bool AddOne(UInt128 &value) noexcept
{
	++value.low;
	if (value.low != 0U) {
		return false;
	}
	++value.high;
	return value.high == 0U;
}

UInt128 TwosComplement(UInt128 value) noexcept
{
	value.high = ~value.high;
	value.low = ~value.low;
	(void)AddOne(value);
	return value;
}

UInt128 SubtractOne(UInt128 value) noexcept
{
	if (value.low != 0U) {
		--value.low;
	} else {
		value.low = kMaxUint64;
		--value.high;
	}
	return value;
}

bool HasSignBit(Int128 raw, unsigned int bits) noexcept
{
	if (bits == 0U) {
		return false;
	}
	if (bits <= 64U) {
		return (raw.low & (std::uint64_t(1U) << (bits - 1U))) != 0U;
	}
	return (raw.high & (std::uint64_t(1U) << (bits - 65U))) != 0U;
}

Int128 SignExtended(Int128 raw, unsigned int bits) noexcept
{
	const UInt128 mask = BitMask(bits);
	UInt128 value{raw.high & mask.high, raw.low & mask.low};
	if (HasSignBit({value.high, value.low}, bits)) {
		value.high |= ~mask.high;
		value.low |= ~mask.low;
	}
	return {value.high, value.low};
}

UInt128 IntMagnitude(Int128 raw, unsigned int bits) noexcept
{
	UInt128 value{raw.high, raw.low};
	value = Masked(value, bits);
	if (HasSignBit(raw, bits)) {
		value = TwosComplement(value);
		value = Masked(value, bits);
	}
	return value;
}

bool MultiplyByTenAdd(UInt128 &value, unsigned int digit) noexcept
{
	const std::uint64_t low_low = static_cast<std::uint32_t>(value.low);
	const std::uint64_t low_high = value.low >> 32U;
	const std::uint64_t product_low = low_low * 10U + digit;
	const std::uint64_t product_high = low_high * 10U + (product_low >> 32U);
	const std::uint64_t new_low = (product_high << 32U) | (product_low & 0xffffffffU);
	const std::uint64_t carry = product_high >> 32U;

	const std::uint64_t high_low = static_cast<std::uint32_t>(value.high);
	const std::uint64_t high_high = value.high >> 32U;
	const std::uint64_t high_product_low = high_low * 10U + carry;
	const std::uint64_t high_product_high = high_high * 10U + (high_product_low >> 32U);
	const std::uint64_t new_high = (high_product_high << 32U) | (high_product_low & 0xffffffffU);
	const bool overflow = (high_product_high >> 32U) != 0U;
	value.low = new_low;
	value.high = new_high;
	return overflow;
}

Result<UInt128> ParseUnsignedDecimal(const std::string &decimal)
{
	try {
		if (decimal.empty()) {
			return {{}, MakeError(ErrorCode::Value)};
		}
		UInt128 result;
		for (const char character : decimal) {
			if (character < '0' || character > '9') {
				return {{}, MakeError(ErrorCode::Value)};
			}
			if (MultiplyByTenAdd(result, static_cast<unsigned int>(character - '0'))) {
				return {{}, MakeError(ErrorCode::Limit, "value")};
			}
		}
		return {result, {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

bool FitsSigned(Int128 raw, unsigned int bits) noexcept
{
	if (bits == 128U) {
		return true;
	}

	const UInt128 value{raw.high, raw.low};
	const UInt128 mask = BitMask(bits);
	const bool negative = (raw.high & (std::uint64_t(1U) << 63U)) != 0U;
	const bool sign_bit = HasSignBit(raw, bits);
	if (negative != sign_bit) {
		return false;
	}
	if (negative) {
		return (value.high & ~mask.high) == ~mask.high && (value.low & ~mask.low) == ~mask.low;
	}
	return (value.high & ~mask.high) == 0U && (value.low & ~mask.low) == 0U;
}

UInt128 RoundedDoubleToUInt(double number) noexcept
{
	if (number <= 0.0) {
		return {};
	}
	if (number >= kTwoTo128) {
		return {kMaxUint64, kMaxUint64};
	}
	const double rounded = std::floor(number);
	const double fraction = number - rounded;
	double nearest = rounded;
	if (fraction > 0.5 || (fraction == 0.5 && std::fmod(rounded, 2.0) != 0.0)) {
		nearest += 1.0;
	}
	if (nearest >= kTwoTo128) {
		return {kMaxUint64, kMaxUint64};
	}
	const double high_double = std::floor(nearest / kTwoTo64);
	const double low_double = nearest - high_double * kTwoTo64;
	return {static_cast<std::uint64_t>(high_double), static_cast<std::uint64_t>(low_double)};
}

UInt128 SignedLimit(unsigned int bits) noexcept
{
	if (bits == 128U) {
		return {std::uint64_t(1U) << 63U, 0U};
	}
	if (bits > 64U) {
		return {std::uint64_t(1U) << (bits - 65U), 0U};
	}
	return {0U, std::uint64_t(1U) << (bits - 1U)};
}

Result<Value> PreserveRelative(Result<Value> result, const Value &template_value)
{
	if (!result.ok() || !template_value.relative().has_value()) {
		return result;
	}
	return result.value->WithRelative(*template_value.relative());
}

std::uint32_t Float32Bits(float value) noexcept
{
	std::uint32_t bits = 0U;
	std::memcpy(&bits, &value, sizeof(bits));
	return bits;
}

float BitsFloat32(std::uint32_t bits) noexcept
{
	float value = 0.0F;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

std::uint64_t DoubleBits(double value) noexcept
{
	std::uint64_t bits = 0U;
	std::memcpy(&bits, &value, sizeof(bits));
	return bits;
}

double BitsDouble(std::uint64_t bits) noexcept
{
	double value = 0.0;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

} // namespace

Result<Value> Value::Finish(Value value, std::string unit)
{
	try {
		if (unit.size() > Limits::kSegmentBytes) {
			return {{}, MakeError(ErrorCode::Limit, "unit")};
		}
		if (unit.find('\0') != std::string::npos) {
			return {{}, MakeError(ErrorCode::Nul)};
		}
		if (!IsValidUtf8(unit)) {
			return {{}, MakeError(ErrorCode::Utf8)};
		}
		value.unit_ = std::move(unit);
		return {std::move(value), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Value> Value::MakeTrigger(std::string unit)
{
	Value value;
	value.kind_ = ValueKind::Trigger;
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeBool(bool value_data, std::string unit)
{
	Value value;
	value.kind_ = ValueKind::Bool;
	value.bool_value_ = value_data;
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeUInt(unsigned int width_bits, UInt128 raw, std::string unit)
{
	if (width_bits == 0U || width_bits > 128U) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	if (!Fits(raw, width_bits)) {
		return {{}, MakeError(ErrorCode::Limit, "value")};
	}
	Value value;
	value.kind_ = ValueKind::UInt;
	value.width_bits_ = width_bits;
	value.uint_raw_ = raw;
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeInt(unsigned int width_bits, Int128 raw, std::string unit)
{
	if (width_bits == 0U || width_bits > 128U) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	if (!FitsSigned(raw, width_bits)) {
		return {{}, MakeError(ErrorCode::Limit, "value")};
	}
	Value value;
	value.kind_ = ValueKind::Int;
	value.width_bits_ = width_bits;
	value.int_raw_ = SignExtended(raw, width_bits);
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeFloatBits(FloatFormat format, std::uint64_t raw, std::string unit)
{
	if (format != FloatFormat::Ieee32 && format != FloatFormat::Ieee64) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	if (format == FloatFormat::Ieee32 && raw > std::numeric_limits<std::uint32_t>::max()) {
		return {{}, MakeError(ErrorCode::Limit, "value")};
	}
	Value value;
	value.kind_ = ValueKind::FloatBits;
	value.float_format_ = format;
	value.float_raw_ = raw;
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeText(std::string text, std::string unit)
{
	if (text.size() > Limits::kTextBytes) {
		return {{}, MakeError(ErrorCode::Limit, "text")};
	}
	if (text.find('\0') != std::string::npos) {
		return {{}, MakeError(ErrorCode::Nul)};
	}
	if (!IsValidUtf8(text)) {
		return {{}, MakeError(ErrorCode::Utf8)};
	}
	Value value;
	value.kind_ = ValueKind::Text;
	value.text_ = std::move(text);
	return Finish(std::move(value), std::move(unit));
}

Result<Value> Value::MakeBlob(std::vector<std::uint8_t> bytes, std::string unit)
{
	try {
		if (bytes.size() > Limits::kBlobBytes) {
			return {{}, MakeError(ErrorCode::Limit, "blob")};
		}
		Value value;
		value.kind_ = ValueKind::Blob;
		value.blob_ = std::move(bytes);
		return Finish(std::move(value), std::move(unit));
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Value> Value::MakeTuple(std::vector<Value> values, std::string unit)
{
	try {
		if (values.size() > Limits::kTupleElements) {
			return {{}, MakeError(ErrorCode::Limit, "tuple")};
		}
		std::function<bool(const Value &, std::size_t)> within_depth = [&within_depth](const Value &value,
											       std::size_t depth) {
			if (value.kind() != ValueKind::Tuple) {
				return true;
			}
			if (depth > Limits::kTupleDepth) {
				return false;
			}
			for (const Value &child : value.tuple()) {
				if (!within_depth(child, depth + 1U)) {
					return false;
				}
			}
			return true;
		};
		for (const Value &value : values) {
			if (!within_depth(value, 2U)) {
				return {{}, MakeError(ErrorCode::Limit, "tuple-depth")};
			}
			if (value.kind() == ValueKind::Invalid) {
				return {{}, MakeError(ErrorCode::Value)};
			}
		}
		Value result;
		result.kind_ = ValueKind::Tuple;
		result.tuple_ = std::move(values);
		return Finish(std::move(result), std::move(unit));
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Value> Value::MakeUIntDecimal(unsigned int width_bits, const std::string &decimal, std::string unit)
{
	if (width_bits == 0U || width_bits > 128U) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	if (!decimal.empty() && decimal[0] == '-' && IsIntegerDecimal(decimal)) {
		return {{}, MakeError(ErrorCode::Limit, "value")};
	}
	auto raw = ParseUnsignedDecimal(decimal);
	if (!raw.ok()) {
		return {{}, raw.error};
	}
	return MakeUInt(width_bits, *raw.value, std::move(unit));
}

Result<Value> Value::MakeIntDecimal(unsigned int width_bits, const std::string &decimal, std::string unit)
{
	if (width_bits == 0U || width_bits > 128U || decimal.empty()) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	bool negative = false;
	std::size_t start = 0U;
	if (decimal[0] == '-' || decimal[0] == '+') {
		negative = decimal[0] == '-';
		start = 1U;
	}
	if (start == decimal.size()) {
		return {{}, MakeError(ErrorCode::Value)};
	}
	auto magnitude_result = ParseUnsignedDecimal(decimal.substr(start));
	if (!magnitude_result.ok()) {
		return {{}, magnitude_result.error};
	}
	const UInt128 magnitude = *magnitude_result.value;
	const UInt128 limit = SignedLimit(width_bits);
	if (negative) {
		if (IsLess(limit, magnitude)) {
			return {{}, MakeError(ErrorCode::Limit, "value")};
		}
	} else if (!IsLess(magnitude, limit)) {
		return {{}, MakeError(ErrorCode::Limit, "value")};
	}
	const UInt128 raw = negative ? TwosComplement(magnitude) : magnitude;
	return MakeInt(width_bits, {raw.high, raw.low}, std::move(unit));
}

Result<Value> Value::FromDoubleInCodeSpace(const Value &value, double number)
{
	try {
		if (std::isnan(number)) {
			return {{}, MakeError(ErrorCode::Value)};
		}
		switch (value.kind_) {
		case ValueKind::Trigger:
			return PreserveRelative(MakeTrigger(value.unit_), value);
		case ValueKind::Bool:
			return PreserveRelative(MakeBool(number != 0.0, value.unit_), value);
		case ValueKind::UInt: {
			const UInt128 maximum = BitMask(value.width_bits_);
			UInt128 rounded;
			if (number > 0.0) {
				rounded = RoundedDoubleToUInt(number);
			}
			if (IsLess(maximum, rounded)) {
				rounded = maximum;
			}
			return PreserveRelative(MakeUInt(value.width_bits_, rounded, value.unit_), value);
		}
		case ValueKind::Int: {
			const double limit_double = std::ldexp(1.0, static_cast<int>(value.width_bits_ - 1U));
			if (number >= limit_double) {
				const UInt128 maximum = SubtractOne(SignedLimit(value.width_bits_));
				return PreserveRelative(
					MakeInt(value.width_bits_, {maximum.high, maximum.low}, value.unit_), value);
			}
			if (number <= -limit_double) {
				const UInt128 magnitude = SignedLimit(value.width_bits_);
				const UInt128 raw = TwosComplement(magnitude);
				return PreserveRelative(MakeInt(value.width_bits_, {raw.high, raw.low}, value.unit_),
							value);
			}
			const bool negative = number < 0.0;
			const UInt128 magnitude = RoundedDoubleToUInt(std::abs(number));
			const UInt128 signed_limit = SignedLimit(value.width_bits_);
			if (magnitude == signed_limit || IsLess(signed_limit, magnitude)) {
				if (negative) {
					const UInt128 raw = TwosComplement(signed_limit);
					return PreserveRelative(
						MakeInt(value.width_bits_, {raw.high, raw.low}, value.unit_), value);
				}
				const UInt128 maximum = SubtractOne(signed_limit);
				return PreserveRelative(
					MakeInt(value.width_bits_, {maximum.high, maximum.low}, value.unit_), value);
			}
			const UInt128 raw = negative ? TwosComplement(magnitude) : magnitude;
			return PreserveRelative(MakeInt(value.width_bits_, {raw.high, raw.low}, value.unit_), value);
		}
		case ValueKind::FloatBits: {
			if (value.float_format_ == FloatFormat::Ieee32) {
				float converted = static_cast<float>(number);
				if (!std::isfinite(converted)) {
					converted = std::signbit(number) ? -std::numeric_limits<float>::max()
									 : std::numeric_limits<float>::max();
				}
				return PreserveRelative(
					MakeFloatBits(value.float_format_, Float32Bits(converted), value.unit_), value);
			}
			const double converted =
				std::isinf(number) ? std::copysign(std::numeric_limits<double>::max(), number) : number;
			return PreserveRelative(MakeFloatBits(value.float_format_, DoubleBits(converted), value.unit_),
						value);
		}
		case ValueKind::Text:
			return MakeText(value.text_, value.unit_);
		case ValueKind::Blob:
			return MakeBlob(value.blob_, value.unit_);
		case ValueKind::Tuple:
			return MakeTuple(value.tuple_, value.unit_);
		case ValueKind::Invalid:
			break;
		}
		return {{}, MakeError(ErrorCode::Value)};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Value> Value::WithRelative(Relative relative) const
{
	try {
		if ((kind_ != ValueKind::UInt && kind_ != ValueKind::Int && kind_ != ValueKind::FloatBits) ||
		    (relative.encoding != RelativeEncoding::TwosComplement &&
		     relative.encoding != RelativeEncoding::SignMagnitude &&
		     relative.encoding != RelativeEncoding::BinaryOffset) ||
		    !IsPositiveDecimal(relative.step) || relative.step.size() > Limits::kSegmentBytes ||
		    relative.step.find('\0') != std::string::npos) {
			return {{}, MakeError(ErrorCode::Value)};
		}
		Value result = *this;
		result.relative_ = std::move(relative);
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

double Value::AsDouble() const noexcept
{
	switch (kind_) {
	case ValueKind::Trigger:
		return 1.0;
	case ValueKind::Bool:
		return bool_value_ ? 1.0 : 0.0;
	case ValueKind::UInt:
		return UInt128ToDouble(uint_raw_);
	case ValueKind::Int: {
		const bool negative = HasSignBit(int_raw_, width_bits_);
		const UInt128 magnitude = IntMagnitude({int_raw_.high, int_raw_.low}, width_bits_);
		const double value = UInt128ToDouble(magnitude);
		return negative ? -value : value;
	}
	case ValueKind::FloatBits:
		return float_format_ == FloatFormat::Ieee32
			       ? static_cast<double>(BitsFloat32(static_cast<std::uint32_t>(float_raw_)))
			       : BitsDouble(float_raw_);
	case ValueKind::Text:
	case ValueKind::Blob:
	case ValueKind::Tuple:
	case ValueKind::Invalid:
		return 0.0;
	}
	return 0.0;
}

bool Value::operator==(const Value &other) const noexcept
{
	return kind_ == other.kind_ && width_bits_ == other.width_bits_ && float_format_ == other.float_format_ &&
	       bool_value_ == other.bool_value_ && uint_raw_ == other.uint_raw_ && int_raw_ == other.int_raw_ &&
	       float_raw_ == other.float_raw_ && text_ == other.text_ && blob_ == other.blob_ &&
	       tuple_ == other.tuple_ && unit_ == other.unit_ && relative_ == other.relative_;
}

} // namespace Jeiobs::Control
