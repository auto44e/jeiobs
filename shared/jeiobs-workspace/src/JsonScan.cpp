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
#include "jeiobs/workspace/JsonScan.hpp"

#include "jeiobs/workspace/Limits.hpp"

#include <charconv>
#include <locale>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

// Called only for a grammar-valid token that the parser reported out of range, which is either
// huge (overflow) or tiny (underflow). Decide from the decimal position of the first non-zero digit:
// positive means a magnitude of at least 1, so the out-of-range result was an overflow.
long long DecimalMagnitude(std::string_view token, bool &zero)
{
	std::size_t i = 0U;
	if (i < token.size() && token[i] == '-') {
		++i;
	}
	long long first_digit_position = 0; // position of the first non-zero digit, 1 = units
	bool seen_nonzero = false;
	long long integer_digits = 0;
	while (i < token.size() && token[i] >= '0' && token[i] <= '9') {
		if (token[i] != '0' && !seen_nonzero) {
			seen_nonzero = true;
			first_digit_position = 1;
		} else if (seen_nonzero) {
			++first_digit_position;
		}
		++integer_digits;
		++i;
	}
	if (i < token.size() && token[i] == '.') {
		++i;
		long long fraction_index = 0;
		while (i < token.size() && token[i] >= '0' && token[i] <= '9') {
			++fraction_index;
			if (!seen_nonzero && token[i] != '0') {
				seen_nonzero = true;
				first_digit_position = 1 - fraction_index;
			}
			++i;
		}
	}
	zero = !seen_nonzero;
	if (zero) {
		return 0;
	}
	long long exponent = 0;
	if (i < token.size() && (token[i] == 'e' || token[i] == 'E')) {
		++i;
		bool negative = false;
		if (i < token.size() && (token[i] == '+' || token[i] == '-')) {
			negative = token[i] == '-';
			++i;
		}
		while (i < token.size() && token[i] >= '0' && token[i] <= '9') {
			if (exponent < 1000000) {
				exponent = exponent * 10 + (token[i] - '0');
			}
			++i;
		}
		if (negative) {
			exponent = -exponent;
		}
	}
	(void)integer_digits;
	return first_digit_position + exponent;
}

// The token already matched the JSON number grammar; only overflow to infinity is rejected
// (an underflow to zero or a denormal is a valid number). Locale independent on every platform:
// std::from_chars for double where the standard library has it, otherwise a classic-locale
// stream (Apple's libc++ lacks floating-point from_chars for older deployment targets).
bool IsOverflowingExponent(std::string_view token)
{
	bool zero = false;
	const long long magnitude = DecimalMagnitude(token, zero);
	return !zero && magnitude > 0;
}

bool IsFiniteJsonNumber(std::string_view token)
{
	if (token.size() > 4096U) {
		return false;
	}
	// Fast path: a grammar-valid number whose first significant digit sits well inside the double
	// range (10^-300 .. 10^300) is finite; no conversion needed. Only numbers near the limits are
	// converted, so a document with a million numbers stays linear and cheap on every platform.
	bool zero = false;
	const long long magnitude = DecimalMagnitude(token, zero);
	if (zero || (magnitude > -300 && magnitude < 300)) {
		return true;
	}
#if !defined(JW_NO_FROM_CHARS) && defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
	double number = 0.0;
	const std::from_chars_result parsed =
		std::from_chars(token.data(), token.data() + token.size(), number, std::chars_format::general);
	if (parsed.ptr != token.data() + token.size()) {
		return false;
	}
	if (parsed.ec == std::errc::result_out_of_range) {
		return !IsOverflowingExponent(token);
	}
	return parsed.ec == std::errc() && std::isfinite(number);
#else
	std::istringstream stream{std::string(token)};
	stream.imbue(std::locale::classic());
	double number = 0.0;
	stream >> number;
	if (stream.fail()) {
		return !IsOverflowingExponent(token);
	}
	return std::isfinite(number);
#endif
}

constexpr std::size_t kNoIndex = std::numeric_limits<std::size_t>::max();

Error MakeError(ErrorCode code, std::string detail = {})
{
	return {code, std::move(detail)};
}

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

bool IsHex(char character)
{
	return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') ||
	       (character >= 'A' && character <= 'F');
}

unsigned int HexValue(char character)
{
	if (character >= '0' && character <= '9') {
		return static_cast<unsigned int>(character - '0');
	}
	if (character >= 'a' && character <= 'f') {
		return static_cast<unsigned int>(character - 'a' + 10);
	}
	return static_cast<unsigned int>(character - 'A' + 10);
}

std::size_t CodePointWidth(std::uint32_t value)
{
	if (value <= 0x7fU) {
		return 1U;
	}
	if (value <= 0x7ffU) {
		return 2U;
	}
	if (value <= 0xffffU) {
		return 3U;
	}
	return 4U;
}

void AppendCodePoint(std::string &output, std::uint32_t value)
{
	if (value <= 0x7fU) {
		output.push_back(static_cast<char>(value));
	} else if (value <= 0x7ffU) {
		output.push_back(static_cast<char>(0xc0U | (value >> 6U)));
		output.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
	} else if (value <= 0xffffU) {
		output.push_back(static_cast<char>(0xe0U | (value >> 12U)));
		output.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
		output.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
	} else {
		output.push_back(static_cast<char>(0xf0U | (value >> 18U)));
		output.push_back(static_cast<char>(0x80U | ((value >> 12U) & 0x3fU)));
		output.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
		output.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
	}
}

class Parser final {
	enum class FrameKind {
		Object,
		Array,
	};

	enum class FrameState {
		ObjectKeyOrEnd,
		ObjectColon,
		ObjectValue,
		ObjectCommaOrEnd,
		ArrayValueOrEnd,
		ArrayCommaOrEnd,
	};

	struct Frame {
		FrameKind kind = FrameKind::Object;
		FrameState state = FrameState::ObjectKeyOrEnd;
		std::size_t value_index = kNoIndex;
		std::size_t pending_member = kNoIndex;
		std::size_t next_array_index = 0U;
		bool after_comma = false; // a "," was just read: a closing bracket here is a trailing comma
		std::unordered_set<std::string> keys;
	};

	std::string_view bytes_;
	std::size_t position_ = 0U;
	std::size_t value_count_ = 0U;
	bool root_done_ = false;
	bool expect_value_ = true;
	std::size_t next_parent_index_ = kNoIndex;
	std::size_t next_member_index_ = kNoIndex;
	std::size_t next_array_index_ = kNoIndex;
	std::vector<Frame> stack_;
	JsonScanResult result_;

	bool Fail(Error error)
	{
		if (result_.error.ok()) {
			result_.error = std::move(error);
		}
		return false;
	}

	void SkipWhitespace()
	{
		while (position_ < bytes_.size() && (bytes_[position_] == ' ' || bytes_[position_] == '\t' ||
						     bytes_[position_] == '\r' || bytes_[position_] == '\n')) {
			++position_;
		}
	}

	bool ParseString(std::string &value, std::size_t &content_start, std::size_t &content_end)
	{
		if (position_ >= bytes_.size() || bytes_[position_] != '"') {
			return Fail(MakeError(ErrorCode::Json));
		}
		++position_;
		content_start = position_;
		value.clear();
		const auto append_bytes = [&](std::string_view bytes) {
			if (bytes.size() > Limits::kDocumentStringBytes - value.size()) {
				return Fail(MakeError(ErrorCode::Limit, "string"));
			}
			value.append(bytes);
			return true;
		};
		const auto append_character = [&](char character) {
			if (value.size() >= Limits::kDocumentStringBytes) {
				return Fail(MakeError(ErrorCode::Limit, "string"));
			}
			value.push_back(character);
			return true;
		};
		while (position_ < bytes_.size()) {
			const unsigned char character = static_cast<unsigned char>(bytes_[position_]);
			if (character == '"') {
				content_end = position_;
				++position_;
				return true;
			}
			if (character < 0x20U) {
				return Fail(MakeError(ErrorCode::Json));
			}
			if (character != '\\') {
				std::size_t width = 1U;
				if (character >= 0x80U) {
					if (character >= 0xc2U && character <= 0xdfU) {
						width = 2U;
					} else if (character >= 0xe0U && character <= 0xefU) {
						width = 3U;
					} else if (character >= 0xf0U && character <= 0xf4U) {
						width = 4U;
					} else {
						return Fail(MakeError(ErrorCode::Utf8));
					}
					if (position_ + width > bytes_.size()) {
						return Fail(MakeError(ErrorCode::Utf8));
					}
					for (std::size_t offset = 1U; offset < width; ++offset) {
						const unsigned char continuation =
							static_cast<unsigned char>(bytes_[position_ + offset]);
						if ((continuation & 0xc0U) != 0x80U) {
							return Fail(MakeError(ErrorCode::Utf8));
						}
					}
				}
				if (!append_bytes(bytes_.substr(position_, width))) {
					return false;
				}
				position_ += width;
			} else {
				++position_;
				if (position_ >= bytes_.size()) {
					return Fail(MakeError(ErrorCode::Json));
				}
				const char escape = bytes_[position_++];
				switch (escape) {
				case '"':
				case '\\':
				case '/':
					if (!append_character(escape)) {
						return false;
					}
					break;
				case 'b':
					if (!append_character('\b')) {
						return false;
					}
					break;
				case 'f':
					if (!append_character('\f')) {
						return false;
					}
					break;
				case 'n':
					if (!append_character('\n')) {
						return false;
					}
					break;
				case 'r':
					if (!append_character('\r')) {
						return false;
					}
					break;
				case 't':
					if (!append_character('\t')) {
						return false;
					}
					break;
				case 'u': {
					if (position_ + 4U > bytes_.size()) {
						return Fail(MakeError(ErrorCode::Json));
					}
					unsigned int code_unit = 0U;
					for (std::size_t offset = 0U; offset < 4U; ++offset) {
						if (!IsHex(bytes_[position_ + offset])) {
							return Fail(MakeError(ErrorCode::Json));
						}
						code_unit = (code_unit << 4U) | HexValue(bytes_[position_ + offset]);
					}
					position_ += 4U;
					std::uint32_t code_point = code_unit;
					if (code_unit >= 0xd800U && code_unit <= 0xdbffU) {
						if (position_ + 6U > bytes_.size() || bytes_[position_] != '\\' ||
						    bytes_[position_ + 1U] != 'u') {
							return Fail(MakeError(ErrorCode::Json));
						}
						unsigned int low = 0U;
						for (std::size_t offset = 0U; offset < 4U; ++offset) {
							if (!IsHex(bytes_[position_ + 2U + offset])) {
								return Fail(MakeError(ErrorCode::Json));
							}
							low = (low << 4U) | HexValue(bytes_[position_ + 2U + offset]);
						}
						if (low < 0xdc00U || low > 0xdfffU) {
							return Fail(MakeError(ErrorCode::Json));
						}
						position_ += 6U;
						code_point =
							0x10000U + ((code_unit - 0xd800U) << 10U) + (low - 0xdc00U);
					} else if (code_unit >= 0xdc00U && code_unit <= 0xdfffU) {
						return Fail(MakeError(ErrorCode::Json));
					}
					if (code_point == 0U) {
						return Fail(MakeError(ErrorCode::Nul));
					}
					const std::size_t width = CodePointWidth(code_point);
					if (width > Limits::kDocumentStringBytes - value.size()) {
						return Fail(MakeError(ErrorCode::Limit, "string"));
					}
					AppendCodePoint(value, code_point);
					break;
				}
				default:
					return Fail(MakeError(ErrorCode::Json));
				}
			}
		}
		return Fail(MakeError(ErrorCode::Json));
	}

	bool BeginValue(JsonValueKind kind, std::size_t &value_index)
	{
		++value_count_;
		if (value_count_ > Limits::kDocumentValues) {
			return Fail(MakeError(ErrorCode::Limit, "values"));
		}
		JsonValueOccurrence value;
		value.kind = kind;
		value.start = position_;
		value.parent_index = next_parent_index_;
		value.member_index = next_member_index_;
		value.array_index = next_array_index_;
		next_parent_index_ = kNoIndex;
		next_member_index_ = kNoIndex;
		next_array_index_ = kNoIndex;
		value_index = result_.values.size();
		result_.values.push_back(std::move(value));
		return true;
	}

	bool CompleteValue(std::size_t value_index)
	{
		if (stack_.empty()) {
			root_done_ = true;
			expect_value_ = false;
			return true;
		}
		Frame &parent = stack_.back();
		if (parent.kind == FrameKind::Object && parent.pending_member != kNoIndex) {
			JsonMemberOccurrence &member = result_.members[parent.pending_member];
			member.end = result_.values[value_index].end;
			member.value_index = value_index;
			parent.pending_member = kNoIndex;
		}
		expect_value_ = false;
		return true;
	}

	bool CloseContainer()
	{
		if (stack_.empty()) {
			return Fail(MakeError(ErrorCode::Json));
		}
		Frame frame = std::move(stack_.back());
		stack_.pop_back();
		result_.values[frame.value_index].end = position_ + 1U;
		++position_;
		return CompleteValue(frame.value_index);
	}

	bool ParseNumber(std::size_t &end)
	{
		const std::size_t start = position_;
		if (bytes_[position_] == '-') {
			++position_;
		}
		if (position_ >= bytes_.size()) {
			return Fail(MakeError(ErrorCode::Json));
		}
		if (bytes_[position_] == '0') {
			++position_;
			if (position_ < bytes_.size() && bytes_[position_] >= '0' && bytes_[position_] <= '9') {
				return Fail(MakeError(ErrorCode::Json));
			}
		} else if (bytes_[position_] >= '1' && bytes_[position_] <= '9') {
			while (position_ < bytes_.size() && bytes_[position_] >= '0' && bytes_[position_] <= '9') {
				++position_;
			}
		} else {
			return Fail(MakeError(ErrorCode::Json));
		}
		if (position_ < bytes_.size() && bytes_[position_] == '.') {
			++position_;
			const std::size_t fraction_start = position_;
			while (position_ < bytes_.size() && bytes_[position_] >= '0' && bytes_[position_] <= '9') {
				++position_;
			}
			if (position_ == fraction_start) {
				return Fail(MakeError(ErrorCode::Json));
			}
		}
		if (position_ < bytes_.size() && (bytes_[position_] == 'e' || bytes_[position_] == 'E')) {
			++position_;
			if (position_ < bytes_.size() && (bytes_[position_] == '+' || bytes_[position_] == '-')) {
				++position_;
			}
			const std::size_t exponent_start = position_;
			while (position_ < bytes_.size() && bytes_[position_] >= '0' && bytes_[position_] <= '9') {
				++position_;
			}
			if (position_ == exponent_start) {
				return Fail(MakeError(ErrorCode::Json));
			}
		}

		const std::string_view token = bytes_.substr(start, position_ - start);
		if (token.size() > 4096U) {
			return Fail(MakeError(ErrorCode::Json));
		}
		if (!IsFiniteJsonNumber(token)) {
			return Fail(MakeError(ErrorCode::Nonfinite));
		}
		end = position_;
		return true;
	}

	bool ParsePrimitive()
	{
		const std::size_t start = position_;
		JsonValueKind kind = JsonValueKind::Null;
		bool boolean = false;
		if (bytes_.substr(position_, 4U) == "null") {
			position_ += 4U;
			kind = JsonValueKind::Null;
		} else if (bytes_.substr(position_, 4U) == "true") {
			position_ += 4U;
			kind = JsonValueKind::Boolean;
			boolean = true;
		} else if (bytes_.substr(position_, 5U) == "false") {
			position_ += 5U;
			kind = JsonValueKind::Boolean;
		} else {
			kind = JsonValueKind::Number;
			std::size_t end = 0U;
			if (!ParseNumber(end)) {
				return false;
			}
		}
		std::size_t value_index = kNoIndex;
		if (!BeginValue(kind, value_index)) {
			return false;
		}
		JsonValueOccurrence &value = result_.values[value_index];
		value.start = start;
		value.end = position_;
		value.boolean_value = boolean;
		return CompleteValue(value_index);
	}

	bool ParseNextValue()
	{
		SkipWhitespace();
		if (position_ >= bytes_.size()) {
			return Fail(MakeError(ErrorCode::Json));
		}
		const char character = bytes_[position_];
		if (character == '{' || character == '[') {
			const JsonValueKind kind = character == '{' ? JsonValueKind::Object : JsonValueKind::Array;
			if (stack_.size() + 1U > Limits::kDepth) {
				return Fail(MakeError(ErrorCode::Limit, "depth"));
			}
			std::size_t value_index = kNoIndex;
			if (!BeginValue(kind, value_index)) {
				return false;
			}
			Frame frame;
			frame.kind = character == '{' ? FrameKind::Object : FrameKind::Array;
			frame.state = character == '{' ? FrameState::ObjectKeyOrEnd : FrameState::ArrayValueOrEnd;
			frame.value_index = value_index;
			stack_.push_back(std::move(frame));
			++position_;
			expect_value_ = false;
			return true;
		}
		if (character == '"') {
			std::string value_string;
			std::size_t content_start = 0U;
			std::size_t content_end = 0U;
			const std::size_t token_start = position_;
			if (!ParseString(value_string, content_start, content_end)) {
				return false;
			}
			std::size_t value_index = kNoIndex;
			if (!BeginValue(JsonValueKind::String, value_index)) {
				return false;
			}
			JsonValueOccurrence &value = result_.values[value_index];
			value.start = token_start;
			value.end = position_;
			value.content_start = content_start;
			value.content_end = content_end;
			value.string_value = std::move(value_string);
			result_.strings.push_back({{}, content_start, content_end, value.string_value, value_index});
			return CompleteValue(value_index);
		}
		return ParsePrimitive();
	}

	bool ProcessObject(Frame &frame)
	{
		switch (frame.state) {
		case FrameState::ObjectKeyOrEnd: {
			SkipWhitespace();
			if (position_ >= bytes_.size()) {
				return Fail(MakeError(ErrorCode::Json));
			}
			if (bytes_[position_] == '}') {
				if (frame.after_comma) {
					return Fail(MakeError(ErrorCode::Json));
				}
				return CloseContainer();
			}
			if (bytes_[position_] != '"') {
				return Fail(MakeError(ErrorCode::Json));
			}
			frame.after_comma = false;
			std::string key;
			std::size_t key_start = 0U;
			std::size_t key_end = 0U;
			if (!ParseString(key, key_start, key_end)) {
				return false;
			}
			if (!frame.keys.insert(key).second) {
				result_.duplicate_key = true;
			}
			JsonMemberOccurrence member;
			member.key = key;
			member.key_start = key_start;
			member.key_end = key_end;
			member.parent_index = frame.value_index;
			const std::size_t member_index = result_.members.size();
			result_.members.push_back(std::move(member));
			frame.pending_member = member_index;
			frame.state = FrameState::ObjectColon;
			return true;
		}
		case FrameState::ObjectColon:
			SkipWhitespace();
			if (position_ >= bytes_.size() || bytes_[position_] != ':') {
				return Fail(MakeError(ErrorCode::Json));
			}
			++position_;
			frame.state = FrameState::ObjectValue;
			return true;
		case FrameState::ObjectValue:
			SkipWhitespace();
			if (position_ >= bytes_.size() || frame.pending_member == kNoIndex) {
				return Fail(MakeError(ErrorCode::Json));
			}
			frame.state = FrameState::ObjectCommaOrEnd;
			result_.members[frame.pending_member].start = position_;
			next_parent_index_ = frame.value_index;
			next_member_index_ = frame.pending_member;
			next_array_index_ = kNoIndex;
			expect_value_ = true;
			return true;
		case FrameState::ObjectCommaOrEnd:
			SkipWhitespace();
			if (position_ >= bytes_.size()) {
				return Fail(MakeError(ErrorCode::Json));
			}
			if (bytes_[position_] == ',') {
				++position_;
				frame.after_comma = true;
				frame.state = FrameState::ObjectKeyOrEnd;
				return true;
			}
			if (bytes_[position_] == '}') {
				return CloseContainer();
			}
			return Fail(MakeError(ErrorCode::Json));
		default:
			return Fail(MakeError(ErrorCode::Json));
		}
	}

	bool ProcessArray(Frame &frame)
	{
		switch (frame.state) {
		case FrameState::ArrayValueOrEnd:
			SkipWhitespace();
			if (position_ >= bytes_.size()) {
				return Fail(MakeError(ErrorCode::Json));
			}
			if (bytes_[position_] == ']') {
				if (frame.after_comma) {
					return Fail(MakeError(ErrorCode::Json));
				}
				return CloseContainer();
			}
			frame.after_comma = false;
			frame.state = FrameState::ArrayCommaOrEnd;
			next_parent_index_ = frame.value_index;
			next_member_index_ = kNoIndex;
			next_array_index_ = frame.next_array_index;
			++frame.next_array_index;
			expect_value_ = true;
			return true;
		case FrameState::ArrayCommaOrEnd:
			SkipWhitespace();
			if (position_ >= bytes_.size()) {
				return Fail(MakeError(ErrorCode::Json));
			}
			if (bytes_[position_] == ',') {
				++position_;
				frame.after_comma = true;
				frame.state = FrameState::ArrayValueOrEnd;
				return true;
			}
			if (bytes_[position_] == ']') {
				return CloseContainer();
			}
			return Fail(MakeError(ErrorCode::Json));
		default:
			return Fail(MakeError(ErrorCode::Json));
		}
	}

public:
	explicit Parser(std::string_view bytes) : bytes_(bytes) {}

	JsonScanResult Run()
	{
		if (bytes_.size() > Limits::kDocumentBytes) {
			result_.error = MakeError(ErrorCode::Limit, "blob");
			return std::move(result_);
		}
		if (bytes_.size() >= 3U && static_cast<unsigned char>(bytes_[0]) == 0xefU &&
		    static_cast<unsigned char>(bytes_[1]) == 0xbbU && static_cast<unsigned char>(bytes_[2]) == 0xbfU) {
			position_ = 3U;
		}
		if (bytes_.find('\0') != std::string_view::npos) {
			result_.error = MakeError(ErrorCode::Nul);
			return std::move(result_);
		}
		if (!IsValidUtf8(bytes_)) {
			result_.error = MakeError(ErrorCode::Utf8);
			return std::move(result_);
		}

		while (result_.error.ok()) {
			if (expect_value_) {
				if (!ParseNextValue()) {
					break;
				}
				continue;
			}
			if (stack_.empty()) {
				SkipWhitespace();
				if (!root_done_ || position_ != bytes_.size()) {
					Fail(MakeError(ErrorCode::Json));
				}
				break;
			}
			Frame &frame = stack_.back();
			if (frame.kind == FrameKind::Object) {
				if (!ProcessObject(frame)) {
					break;
				}
			} else if (!ProcessArray(frame)) {
				break;
			}
		}

		if (result_.error.ok() && !stack_.empty()) {
			result_.error = MakeError(ErrorCode::Json);
		}
		if (result_.error.ok() && result_.duplicate_key) {
			result_.error = MakeError(ErrorCode::DuplicateKey);
		}
		return std::move(result_);
	}
};

} // namespace

JsonScanResult JsonScan::Scan(std::string_view bytes)
{
	try {
		return Parser(bytes).Run();
	} catch (...) {
		JsonScanResult result;
		result.error = {ErrorCode::Json, {}};
		return result;
	}
}

JsonScanResult ScanJson(std::string_view bytes)
{
	return JsonScan::Scan(bytes);
}

} // namespace Jeiobs::Workspace
