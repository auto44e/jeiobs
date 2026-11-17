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
#include "jeiobs/control/Mapping.hpp"
#include "jeiobs/control/Transform.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using Jeiobs::Control::CompileMapping;
using Jeiobs::Control::CompiledMapping;
using Jeiobs::Control::Error;
using Jeiobs::Control::ErrorCode;
using Jeiobs::Control::ErrorCodeName;
using Jeiobs::Control::Limits::kMappingBytes;
using Jeiobs::Control::Limits::kTupleDepth;
using Jeiobs::Control::Value;
using Json = nlohmann::json;
using JsonSax = nlohmann::json_sax<Json>;

void PrintError(const Error &error)
{
	std::cout << "ERR " << ErrorCodeName(error.code);
	if (!error.detail.empty()) {
		std::cout << ' ' << error.detail;
	}
	std::cout << '\n';
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

class StrictJsonSax final : public JsonSax {
	enum class FrameKind {
		Object,
		Array,
	};

	struct Frame {
		FrameKind kind = FrameKind::Object;
		Json value;
		std::optional<std::string> pending_key;
		std::set<std::string> keys;
	};

	Json root_;
	bool has_root_ = false;
	std::vector<Frame> stack_;
	std::size_t values_ = 0U;
	Error error_;

	bool Fail(Error error)
	{
		if (error_.ok()) {
			error_ = std::move(error);
		}
		return false;
	}

	bool BeforeValue()
	{
		++values_;
		if (values_ > Jeiobs::Control::Limits::kJsonValues) {
			return Fail({ErrorCode::Limit, "values"});
		}
		return true;
	}

	bool CheckString(const std::string &value)
	{
		if (value.size() > Jeiobs::Control::Limits::kJsonStringBytes) {
			return Fail({ErrorCode::Limit, "string"});
		}
		if (value.find('\0') != std::string::npos) {
			return Fail({ErrorCode::Nul, {}});
		}
		return true;
	}

	bool Attach(Json value)
	{
		if (stack_.empty()) {
			if (has_root_) {
				return Fail({ErrorCode::Json, {}});
			}
			root_ = std::move(value);
			has_root_ = true;
			return true;
		}
		Frame &frame = stack_.back();
		if (frame.kind == FrameKind::Array) {
			frame.value.push_back(std::move(value));
			return true;
		}
		if (!frame.pending_key.has_value()) {
			return Fail({ErrorCode::Json, {}});
		}
		frame.value[*frame.pending_key] = std::move(value);
		frame.pending_key.reset();
		return true;
	}

public:
	[[nodiscard]] const Json &root() const noexcept { return root_; }
	[[nodiscard]] bool hasRoot() const noexcept { return has_root_; }
	[[nodiscard]] const Error &error() const noexcept { return error_; }

	bool null() override { return BeforeValue() && Attach(nullptr); }
	bool boolean(bool value) override { return BeforeValue() && Attach(value); }
	bool number_integer(number_integer_t value) override { return BeforeValue() && Attach(value); }
	bool number_unsigned(number_unsigned_t value) override { return BeforeValue() && Attach(value); }

	bool number_float(number_float_t value, const string_t &) override
	{
		if (!std::isfinite(value)) {
			return Fail({ErrorCode::Nonfinite, {}});
		}
		return BeforeValue() && Attach(value);
	}

	bool string(string_t &value) override
	{
		return CheckString(value) && BeforeValue() && Attach(std::move(value));
	}

	bool binary(binary_t &) override { return Fail({ErrorCode::Json, {}}); }

	bool start_object(std::size_t) override
	{
		if (!BeforeValue()) {
			return false;
		}
		if (stack_.size() + 1U > Jeiobs::Control::Limits::kJsonDepth) {
			return Fail({ErrorCode::Limit, "depth"});
		}
		Frame frame;
		frame.kind = FrameKind::Object;
		frame.value = Json::object();
		stack_.push_back(std::move(frame));
		return true;
	}

	bool key(string_t &value) override
	{
		if (stack_.empty() || stack_.back().kind != FrameKind::Object || !CheckString(value)) {
			return false;
		}
		Frame &frame = stack_.back();
		if (!frame.keys.insert(value).second) {
			return Fail({ErrorCode::DuplicateKey, {}});
		}
		frame.pending_key = std::move(value);
		return true;
	}

	bool end_object() override
	{
		if (stack_.empty() || stack_.back().kind != FrameKind::Object ||
		    stack_.back().pending_key.has_value()) {
			return Fail({ErrorCode::Json, {}});
		}
		Json value = std::move(stack_.back().value);
		stack_.pop_back();
		return Attach(std::move(value));
	}

	bool start_array(std::size_t) override
	{
		if (!BeforeValue()) {
			return false;
		}
		if (stack_.size() + 1U > Jeiobs::Control::Limits::kJsonDepth) {
			return Fail({ErrorCode::Limit, "depth"});
		}
		Frame frame;
		frame.kind = FrameKind::Array;
		frame.value = Json::array();
		stack_.push_back(std::move(frame));
		return true;
	}

	bool end_array() override
	{
		if (stack_.empty() || stack_.back().kind != FrameKind::Array) {
			return Fail({ErrorCode::Json, {}});
		}
		Json value = std::move(stack_.back().value);
		stack_.pop_back();
		return Attach(std::move(value));
	}

	bool parse_error(std::size_t, const std::string &, const nlohmann::detail::exception &exception) override
	{
		if (error_.ok()) {
			error_ = {exception.id == 406 ? ErrorCode::Nonfinite : ErrorCode::Json, {}};
		}
		return false;
	}
};

Jeiobs::Control::Result<Json> ParseStrictJson(std::string_view bytes)
{
	try {
		if (bytes.size() > kMappingBytes) {
			return {{}, {ErrorCode::Limit, "inputs"}};
		}
		if (bytes.size() >= 3U && static_cast<unsigned char>(bytes[0]) == 0xefU &&
		    static_cast<unsigned char>(bytes[1]) == 0xbbU && static_cast<unsigned char>(bytes[2]) == 0xbfU) {
			return {{}, {ErrorCode::Bom, {}}};
		}
		if (bytes.find('\0') != std::string_view::npos) {
			return {{}, {ErrorCode::Nul, {}}};
		}
		if (!IsValidUtf8(bytes)) {
			return {{}, {ErrorCode::Utf8, {}}};
		}
		StrictJsonSax sax;
		bool parsed = false;
		try {
			parsed = Json::sax_parse(bytes.begin(), bytes.end(), &sax, Json::input_format_t::json, true);
		} catch (...) {
			return {{}, {ErrorCode::Json, {}}};
		}
		if (!parsed || !sax.hasRoot() || !sax.error().ok()) {
			return {{}, sax.error().ok() ? Error{ErrorCode::Json, {}} : sax.error()};
		}
		return {sax.root(), {}};
	} catch (...) {
		return {{}, {ErrorCode::Json, {}}};
	}
}

Jeiobs::Control::Result<std::string> ReadFile(const std::string &path, std::string_view error_detail)
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(path);
		std::error_code status_error;
		const std::filesystem::file_status status = std::filesystem::status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return {{}, {ErrorCode::Io, {}}};
		}
		std::error_code size_error;
		const std::uintmax_t size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error) {
			return {{}, {ErrorCode::Io, {}}};
		}
		if (size > kMappingBytes) {
			return {{}, {ErrorCode::Limit, std::string(error_detail)}};
		}
		if (size > std::numeric_limits<std::size_t>::max()) {
			return {{}, {ErrorCode::Limit, std::string(error_detail)}};
		}
		std::ifstream input;
		input.open(filesystem_path, std::ios::in | std::ios::binary);
		if (!input.is_open()) {
			return {{}, {ErrorCode::Io, {}}};
		}
		std::string bytes(static_cast<std::size_t>(size), '\0');
		if (!bytes.empty()) {
			input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
				return {{}, {ErrorCode::Io, {}}};
			}
		}
		return {std::move(bytes), {}};
	} catch (...) {
		return {{}, {ErrorCode::Io, {}}};
	}
}

int CheckValidate(const std::string &path)
{
	const auto result = Jeiobs::Control::LoadMapping(path);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const auto validated = Jeiobs::Control::ValidateMapping(*result.value);
	if (!validated.ok()) {
		PrintError(validated.error);
		return 1;
	}
	std::cout << "OK\n";
	return 0;
}

Json CompiledJson(const CompiledMapping &compiled)
{
	Json result = Json::object();
	result["routes"] = Json::array();
	for (const auto &route : compiled.routes) {
		Json entry = Json::object();
		entry["id"] = route.id;
		entry["enabled"] = route.enabled;
		entry["class"] = Jeiobs::Control::CapabilityClassName(route.capability);
		entry["target_key"] = route.target_key;
		entry["merge"] = {{"policy", Jeiobs::Control::MergePolicyName(route.merge.policy)},
				  {"priority", route.merge.priority}};
		entry["fanout"] = route.fanout;
		result["routes"].push_back(std::move(entry));
	}
	result["slots"] = compiled.slots;
	return result;
}

int CheckCompile(const std::string &path)
{
	const auto mapping = Jeiobs::Control::LoadMapping(path);
	if (!mapping.ok()) {
		PrintError(mapping.error);
		return 1;
	}
	const auto compiled = CompileMapping(*mapping.value);
	if (!compiled.ok()) {
		PrintError(compiled.error);
		return 1;
	}
	std::cout << CompiledJson(*compiled.value).dump() << '\n';
	return 0;
}

bool GetUInt(const Json &value, std::uint64_t &result)
{
	if (value.is_number_unsigned()) {
		result = value.get<std::uint64_t>();
		return true;
	}
	if (value.is_number_integer()) {
		const std::int64_t signed_value = value.get<std::int64_t>();
		if (signed_value < 0) {
			return false;
		}
		result = static_cast<std::uint64_t>(signed_value);
		return true;
	}
	return false;
}

int CheckMatch(const std::string &mapping_path, const std::string &event_path)
{
	const auto mapping = Jeiobs::Control::LoadMapping(mapping_path);
	if (!mapping.ok()) {
		PrintError(mapping.error);
		return 1;
	}
	const auto compiled = CompileMapping(*mapping.value);
	if (!compiled.ok()) {
		PrintError(compiled.error);
		return 1;
	}
	auto event_bytes = ReadFile(event_path, "event");
	if (!event_bytes.ok()) {
		PrintError(event_bytes.error);
		return 1;
	}
	const auto event = Jeiobs::Control::ParseEvent(*event_bytes.value);
	if (!event.ok()) {
		PrintError(event.error);
		return 1;
	}
	Json result = Json::object();
	result["matches"] = Json::array();
	for (std::size_t index = 0U; index < mapping.value->routes.size(); ++index) {
		const auto &route = mapping.value->routes[index];
		if (!route.enabled || (route.match.slot.has_value() && route.match.slot != event.value->slot) ||
		    !Jeiobs::Control::Matches(route.match.address, event.value->address)) {
			continue;
		}
		result["matches"].push_back(route.id);
	}
	std::cout << result.dump() << '\n';
	return 0;
}

bool ParseHex(std::string_view value, std::vector<std::uint8_t> &bytes)
{
	if ((value.size() & 1U) != 0U) {
		return false;
	}
	bytes.clear();
	bytes.reserve(value.size() / 2U);
	for (std::size_t index = 0U; index < value.size(); index += 2U) {
		auto nibble = [](char character) -> int {
			if (character >= '0' && character <= '9') {
				return character - '0';
			}
			if (character >= 'a' && character <= 'f') {
				return character - 'a' + 10;
			}
			if (character >= 'A' && character <= 'F') {
				return character - 'A' + 10;
			}
			return -1;
		};
		const int high = nibble(value[index]);
		const int low = nibble(value[index + 1U]);
		if (high < 0 || low < 0) {
			return false;
		}
		bytes.push_back(static_cast<std::uint8_t>((high << 4) | low));
	}
	return true;
}

bool ParseFloatRaw(const Json &document, Jeiobs::Control::FloatFormat &format, std::uint64_t &raw)
{
	const auto format_iterator = document.find("format");
	const auto bits_iterator = document.find("bits_hex");
	if (format_iterator == document.end() || bits_iterator == document.end() || !format_iterator->is_string() ||
	    !bits_iterator->is_string()) {
		return false;
	}
	const std::string format_name = format_iterator->get<std::string>();
	const std::string bits = bits_iterator->get<std::string>();
	const std::size_t expected = format_name == "ieee32" ? 8U : format_name == "ieee64" ? 16U : 0U;
	if (expected == 0U || bits.size() != expected) {
		return false;
	}
	raw = 0U;
	for (const char character : bits) {
		int nibble = -1;
		if (character >= '0' && character <= '9') {
			nibble = character - '0';
		} else if (character >= 'a' && character <= 'f') {
			nibble = character - 'a' + 10;
		} else if (character >= 'A' && character <= 'F') {
			nibble = character - 'A' + 10;
		}
		if (nibble < 0) {
			return false;
		}
		raw = (raw << 4U) | static_cast<std::uint64_t>(nibble);
	}
	format = format_name == "ieee32" ? Jeiobs::Control::FloatFormat::Ieee32 : Jeiobs::Control::FloatFormat::Ieee64;
	return true;
}

bool IsIntegerLiteral(std::string_view value)
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

Jeiobs::Control::Result<Value> ParseValueAt(const Json &document, std::size_t depth)
{
	try {
		if (!document.is_object()) {
			return {{}, {ErrorCode::Value, {}}};
		}
		const auto type = document.find("type");
		if (type == document.end() || !type->is_string()) {
			return {{}, {ErrorCode::Value, {}}};
		}
		const std::string name = type->get<std::string>();
		if (name == "trigger") {
			return Value::MakeTrigger();
		}
		if (name == "bool") {
			const auto value = document.find("value");
			return value != document.end() && value->is_boolean()
				       ? Value::MakeBool(value->get<bool>())
				       : Jeiobs::Control::Result<Value>{{}, {ErrorCode::Value, {}}};
		}
		if (name == "uint") {
			const auto bits = document.find("bits");
			const auto value = document.find("value");
			std::uint64_t width = 0U;
			if (bits == document.end() || value == document.end() || !GetUInt(*bits, width) ||
			    width == 0U || width > 128U || !value->is_string()) {
				return {{}, {ErrorCode::Value, {}}};
			}
			return Value::MakeUIntDecimal(static_cast<unsigned int>(width), value->get<std::string>());
		}
		if (name == "int") {
			const auto bits = document.find("bits");
			const auto value = document.find("value");
			std::uint64_t width = 0U;
			if (bits == document.end() || value == document.end() || !GetUInt(*bits, width) ||
			    width == 0U || width > 128U || !value->is_string()) {
				return {{}, {ErrorCode::Value, {}}};
			}
			return Value::MakeIntDecimal(static_cast<unsigned int>(width), value->get<std::string>());
		}
		if (name == "float") {
			Jeiobs::Control::FloatFormat format = Jeiobs::Control::FloatFormat::Ieee32;
			std::uint64_t raw = 0U;
			if (!ParseFloatRaw(document, format, raw)) {
				return {{}, {ErrorCode::Value, {}}};
			}
			return Value::MakeFloatBits(format, raw);
		}
		if (name == "text") {
			const auto value = document.find("value");
			return value != document.end() && value->is_string()
				       ? Value::MakeText(value->get<std::string>())
				       : Jeiobs::Control::Result<Value>{{}, {ErrorCode::Value, {}}};
		}
		if (name == "blob") {
			const auto value = document.find("hex");
			std::vector<std::uint8_t> bytes;
			if (value == document.end() || !value->is_string() ||
			    !ParseHex(value->get<std::string>(), bytes)) {
				return {{}, {ErrorCode::Value, {}}};
			}
			return Value::MakeBlob(std::move(bytes));
		}
		if (name == "tuple") {
			const auto values = document.find("values");
			if (values == document.end() || !values->is_array()) {
				return {{}, {ErrorCode::Value, {}}};
			}
			if (depth > kTupleDepth) {
				return {{}, {ErrorCode::Limit, "tuple-depth"}};
			}
			std::vector<Value> parsed;
			parsed.reserve(values->size());
			for (const Json &entry : *values) {
				auto child = ParseValueAt(entry, depth + 1U);
				if (!child.ok()) {
					return {{}, child.error};
				}
				parsed.push_back(std::move(*child.value));
			}
			return Value::MakeTuple(std::move(parsed));
		}
		return {{}, {ErrorCode::Value, {}}};
	} catch (...) {
		return {{}, {ErrorCode::Value, {}}};
	}
}

Jeiobs::Control::Result<Value> ParseValue(const Json &document)
{
	return ParseValueAt(document, 1U);
}

Jeiobs::Control::Result<std::vector<Value>> ParseTransformInputs(std::string_view bytes)
{
	auto document = ParseStrictJson(bytes);
	if (!document.ok()) {
		return {{}, document.error};
	}
	if (!document.value->is_array() || document.value->empty() || document.value->size() > 1000U) {
		return {{}, {ErrorCode::Schema, "inputs"}};
	}
	try {
		std::vector<Value> inputs;
		inputs.reserve(document.value->size());
		for (const Json &entry : *document.value) {
			auto value = ParseValue(entry);
			if (!value.ok()) {
				return {{}, {ErrorCode::Schema, "inputs"}};
			}
			inputs.push_back(std::move(*value.value));
		}
		return {std::move(inputs), {}};
	} catch (...) {
		return {{}, {ErrorCode::Schema, "inputs"}};
	}
}

bool ParseDoubleArgument(std::string_view argument, double &result)
{
	if (argument == "inf" || argument == "+inf") {
		result = std::numeric_limits<double>::infinity();
		return true;
	}
	if (argument == "-inf") {
		result = -std::numeric_limits<double>::infinity();
		return true;
	}
	if (argument == "nan" || argument == "+nan" || argument == "-nan") {
		result = std::numeric_limits<double>::quiet_NaN();
		return true;
	}
	std::string text(argument);
	char *end = nullptr;
	errno = 0;
	result = std::strtod(text.c_str(), &end);
	return end != text.c_str() && end == text.c_str() + text.size();
}

// A plain decimal number: digits, one optional sign, a decimal point and an exponent. No hexadecimal
// floats, "inf" or "nan" (strtod accepts those).
bool IsPlainDecimal(std::string_view text)
{
	if (text.empty()) {
		return false;
	}
	for (const char character : text) {
		const bool digit = character >= '0' && character <= '9';
		if (!digit && character != '.' && character != '+' && character != '-' && character != 'e' &&
		    character != 'E') {
			return false;
		}
	}
	return true;
}

std::string UInt128Decimal(Jeiobs::Control::UInt128 value)
{
	if (value.high == 0U && value.low == 0U) {
		return "0";
	}
	std::string result;
	while (value.high != 0U || value.low != 0U) {
		Jeiobs::Control::UInt128 quotient;
		unsigned int remainder = 0U;
		for (int bit = 127; bit >= 0; --bit) {
			const bool set = bit >= 64 ? ((value.high >> (bit - 64)) & 1U) != 0U
						   : ((value.low >> bit) & 1U) != 0U;
			remainder = remainder * 2U + (set ? 1U : 0U);
			if (remainder >= 10U) {
				remainder -= 10U;
				if (bit >= 64) {
					quotient.high |= std::uint64_t(1U) << (bit - 64);
				} else {
					quotient.low |= std::uint64_t(1U) << bit;
				}
			}
		}
		result.push_back(static_cast<char>('0' + remainder));
		value = quotient;
	}
	std::reverse(result.begin(), result.end());
	return result;
}

Jeiobs::Control::UInt128 IntMagnitude(const Value &value)
{
	Jeiobs::Control::UInt128 magnitude{value.intRaw().high, value.intRaw().low};
	if (value.widthBits() <= 64U) {
		magnitude.high = 0U;
		if (value.widthBits() < 64U) {
			magnitude.low &= (std::uint64_t(1U) << value.widthBits()) - 1U;
		}
	} else if (value.widthBits() < 128U) {
		magnitude.high &= (std::uint64_t(1U) << (value.widthBits() - 64U)) - 1U;
	}
	const bool negative = value.widthBits() <= 64U
				      ? (value.intRaw().low & (std::uint64_t(1U) << (value.widthBits() - 1U))) != 0U
				      : (value.intRaw().high & (std::uint64_t(1U) << (value.widthBits() - 65U))) != 0U;
	if (!negative) {
		return magnitude;
	}
	magnitude.high = ~magnitude.high;
	magnitude.low = ~magnitude.low;
	++magnitude.low;
	if (magnitude.low == 0U) {
		++magnitude.high;
	}
	if (value.widthBits() <= 64U) {
		magnitude.high = 0U;
		if (value.widthBits() < 64U) {
			magnitude.low &= (std::uint64_t(1U) << value.widthBits()) - 1U;
		}
	} else if (value.widthBits() < 128U) {
		magnitude.high &= (std::uint64_t(1U) << (value.widthBits() - 64U)) - 1U;
	}
	return magnitude;
}

std::string Int128Decimal(const Value &value)
{
	const bool negative = value.widthBits() <= 64U
				      ? (value.intRaw().low & (std::uint64_t(1U) << (value.widthBits() - 1U))) != 0U
				      : (value.intRaw().high & (std::uint64_t(1U) << (value.widthBits() - 65U))) != 0U;
	std::string result = UInt128Decimal(
		negative ? IntMagnitude(value) : Jeiobs::Control::UInt128{value.intRaw().high, value.intRaw().low});
	if (negative && result != "0") {
		result.insert(result.begin(), '-');
	}
	return result;
}

std::string HexValue(std::uint64_t value, std::size_t digits)
{
	std::string result(digits, '0');
	const char *hex = "0123456789abcdef";
	for (std::size_t index = digits; index > 0U; --index) {
		result[index - 1U] = hex[value & 0xfU];
		value >>= 4U;
	}
	return result;
}

Json ValueJson(const Value &value)
{
	Json result = Json::object();
	switch (value.kind()) {
	case Jeiobs::Control::ValueKind::Trigger:
		result["type"] = "trigger";
		break;
	case Jeiobs::Control::ValueKind::Bool:
		result["type"] = "bool";
		result["value"] = value.boolValue();
		break;
	case Jeiobs::Control::ValueKind::UInt:
		result["bits"] = value.widthBits();
		result["type"] = "uint";
		result["value"] = UInt128Decimal(value.uintRaw());
		break;
	case Jeiobs::Control::ValueKind::Int:
		result["bits"] = value.widthBits();
		result["type"] = "int";
		result["value"] = Int128Decimal(value);
		break;
	case Jeiobs::Control::ValueKind::FloatBits:
		result["bits_hex"] = HexValue(value.floatRaw(),
					      value.floatFormat() == Jeiobs::Control::FloatFormat::Ieee32 ? 8U : 16U);
		result["format"] = value.floatFormat() == Jeiobs::Control::FloatFormat::Ieee32 ? "ieee32" : "ieee64";
		result["type"] = "float";
		break;
	case Jeiobs::Control::ValueKind::Text:
		result["type"] = "text";
		result["value"] = value.text();
		break;
	case Jeiobs::Control::ValueKind::Blob: {
		std::string hex;
		hex.reserve(value.blob().size() * 2U);
		for (const std::uint8_t byte : value.blob()) {
			hex += HexValue(byte, 2U);
		}
		result["hex"] = std::move(hex);
		result["type"] = "blob";
		break;
	}
	case Jeiobs::Control::ValueKind::Tuple:
		result["type"] = "tuple";
		result["values"] = Json::array();
		for (const Value &child : value.tuple()) {
			result["values"].push_back(ValueJson(child));
		}
		break;
	case Jeiobs::Control::ValueKind::Invalid:
		break;
	}
	return result;
}

Json TransformValueJson(const Value &value)
{
	if (value.kind() != Jeiobs::Control::ValueKind::FloatBits) {
		return ValueJson(value);
	}
	double number = value.AsDouble();
	if (number == 0.0) {
		number = 0.0;
	}
	char formatted[64] = {};
	const int length = std::snprintf(formatted, sizeof(formatted), "%.9g", number);
	const std::string text = length > 0 && static_cast<std::size_t>(length) < sizeof(formatted)
					 ? std::string(formatted, static_cast<std::size_t>(length))
					 : std::string("0");
	Json result = Json::object();
	result["format"] = value.floatFormat() == Jeiobs::Control::FloatFormat::Ieee32 ? "ieee32" : "ieee64";
	result["number"] = text;
	result["type"] = "float";
	return result;
}

int CheckValue(const std::string &operation, const std::string &value_bytes, std::string_view double_argument = {})
{
	try {
		Json document = Json::parse(value_bytes, nullptr, false);
		if (document.is_discarded()) {
			PrintError({ErrorCode::Value, {}});
			return 1;
		}
		auto value = ParseValue(document);
		if (!value.ok()) {
			if (operation == "limits" && document.is_object()) {
				const auto type = document.find("type");
				const auto bits = document.find("bits");
				const auto decimal = document.find("value");
				std::uint64_t width = 0U;
				if (type != document.end() && bits != document.end() && decimal != document.end() &&
				    type->is_string() &&
				    (type->get<std::string>() == "uint" || type->get<std::string>() == "int") &&
				    GetUInt(*bits, width) && width >= 1U && width <= 128U && decimal->is_string() &&
				    IsIntegerLiteral(decimal->get<std::string>())) {
					PrintError({ErrorCode::Limit, "value"});
					return 1;
				}
			}
			PrintError(value.error);
			return 1;
		}
		if (operation == "roundtrip") {
			const auto roundtrip = Value::FromDoubleInCodeSpace(*value.value, value.value->AsDouble());
			if (!roundtrip.ok() || *roundtrip.value != *value.value) {
				PrintError({ErrorCode::Value, {}});
				return 1;
			}
			std::cout << "OK\n";
			return 0;
		}
		if (operation == "double") {
			std::cout << std::setprecision(17) << std::defaultfloat << value.value->AsDouble() << '\n';
			return 0;
		}
		if (operation == "fromdouble") {
			double number = 0.0;
			if (!ParseDoubleArgument(double_argument, number)) {
				PrintError({ErrorCode::Value, {}});
				return 1;
			}
			const auto converted = Value::FromDoubleInCodeSpace(*value.value, number);
			if (!converted.ok()) {
				PrintError(converted.error);
				return 1;
			}
			std::cout << ValueJson(*converted.value).dump() << '\n';
			return 0;
		}
		if (operation == "limits") {
			std::cout << "OK\n";
			return 0;
		}
		PrintError({ErrorCode::Usage, {}});
		return 2;
	} catch (...) {
		PrintError({ErrorCode::Value, {}});
		return 1;
	}
}

int CheckTransform(const std::string &mapping_path, const std::string &route_id, const std::string &inputs_path,
		   bool has_position, double position, bool toggled)
{
	const auto mapping = Jeiobs::Control::LoadMapping(mapping_path);
	if (!mapping.ok()) {
		PrintError(mapping.error);
		return 1;
	}
	const auto compiled = CompileMapping(*mapping.value);
	if (!compiled.ok()) {
		PrintError(compiled.error);
		return 1;
	}
	const auto route =
		std::find_if(mapping.value->routes.begin(), mapping.value->routes.end(),
			     [&route_id](const Jeiobs::Control::Route &candidate) { return candidate.id == route_id; });
	if (route == mapping.value->routes.end()) {
		PrintError({ErrorCode::Reference, "route"});
		return 1;
	}
	const auto pipeline = Jeiobs::Control::TransformPipeline::Compile(route->transforms);
	if (!pipeline.ok()) {
		PrintError(pipeline.error);
		return 1;
	}
	const auto input_bytes = ReadFile(inputs_path, "inputs");
	if (!input_bytes.ok()) {
		PrintError(input_bytes.error);
		return 1;
	}
	const auto inputs = ParseTransformInputs(*input_bytes.value);
	if (!inputs.ok()) {
		PrintError(inputs.error);
		return 1;
	}

	Jeiobs::Control::TransformState state;
	state.position = has_position ? position : 0.0;
	state.toggled = toggled;
	Json result = Json::object();
	result["outputs"] = Json::array();
	for (const Value &input : *inputs.value) {
		const auto applied = pipeline.value->Apply(input, state);
		if (!applied.ok()) {
			Json error = Json::object();
			error["error"] = ErrorCodeName(applied.error.code);
			if (!applied.error.detail.empty()) {
				error["error"] =
					std::string(ErrorCodeName(applied.error.code)) + ' ' + applied.error.detail;
			}
			result["outputs"].push_back(std::move(error));
			continue;
		}
		if (applied.value->outcome == Jeiobs::Control::TransformPipeline::Outcome::NoOutput) {
			result["outputs"].push_back(nullptr);
			continue;
		}
		if (!applied.value->value.has_value()) {
			Json error = Json::object();
			error["error"] = "E_VALUE";
			result["outputs"].push_back(std::move(error));
			continue;
		}
		result["outputs"].push_back(TransformValueJson(*applied.value->value));
	}
	std::cout << result.dump() << '\n';
	return 0;
}

void PrintUsage()
{
	std::cerr << "usage: jc-check validate FILE | compile FILE | match FILE EVENT.json | value OP VALUE_JSON"
		     " [DOUBLE] | transform [--position P] [--toggled] FILE ROUTE_ID INPUTS.json\n";
}

} // namespace

int Run(int argc, char **argv)
{
	try {
		if (argc == 3 && std::string_view(argv[1]) == "validate") {
			return CheckValidate(argv[2]);
		}
		if (argc == 3 && std::string_view(argv[1]) == "compile") {
			return CheckCompile(argv[2]);
		}
		if (argc == 4 && std::string_view(argv[1]) == "match") {
			return CheckMatch(argv[2], argv[3]);
		}
		if (argc >= 3 && std::string_view(argv[1]) == "transform") {
			int argument = 2;
			bool has_position = false;
			double position = 0.0;
			bool toggled = false;
			while (argument < argc && std::string_view(argv[argument]).size() >= 2U &&
			       std::string_view(argv[argument])[0] == '-' &&
			       std::string_view(argv[argument])[1] == '-') {
				const std::string_view option = argv[argument];
				if (option == "--toggled") {
					toggled = true;
					++argument;
					continue;
				}
				if (option == "--position" && argument + 1 < argc) {
					if (!IsPlainDecimal(argv[argument + 1]) ||
					    !ParseDoubleArgument(argv[argument + 1], position) ||
					    !std::isfinite(position) || position < 0.0 || position > 1.0) {
						PrintUsage();
						return 2;
					}
					has_position = true;
					argument += 2;
					continue;
				}
				PrintUsage();
				return 2;
			}
			if (argc - argument != 3) {
				PrintUsage();
				return 2;
			}
			return CheckTransform(argv[argument], argv[argument + 1], argv[argument + 2], has_position,
					      position, toggled);
		}
		if (argc == 4 && std::string_view(argv[1]) == "value" && std::string_view(argv[2]) != "fromdouble") {
			return CheckValue(argv[2], argv[3]);
		}
		if (argc == 5 && std::string_view(argv[1]) == "value" && std::string_view(argv[2]) == "fromdouble") {
			return CheckValue(argv[2], argv[3], argv[4]);
		}
		PrintUsage();
		return 2;
	} catch (...) {
		PrintError({ErrorCode::Value, {}});
		return 1;
	}
}

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

// Windows passes the command line in UTF-16; convert it so that every path and argument reaches the
// library as UTF-8, as on the other platforms.
int wmain(int argc, wchar_t **wargv)
{
	try {
		std::vector<std::string> args;
		for (int i = 0; i < argc; ++i) {
			const int bytes = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
			std::string arg(bytes > 0 ? static_cast<std::size_t>(bytes) : 1U, '\0');
			if (bytes > 0) {
				WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, arg.data(), bytes, nullptr, nullptr);
			}
			arg.resize(std::char_traits<char>::length(arg.c_str()));
			args.push_back(std::move(arg));
		}
		std::vector<char *> argv;
		for (std::string &arg : args) {
			argv.push_back(arg.data());
		}
		argv.push_back(nullptr);
		return Run(argc, argv.data());
	} catch (...) {
		return 1;
	}
}
#else
int main(int argc, char **argv)
{
	return Run(argc, argv);
}
#endif
