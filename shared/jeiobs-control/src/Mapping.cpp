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

#include "jeiobs/control/Limits.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Jeiobs::Control {
namespace {

using Json = nlohmann::ordered_json;
using JsonSax = nlohmann::json_sax<Json>;

Error Schema(std::string_view field)
{
	return MakeError(ErrorCode::Schema, std::string(field));
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

class BoundedSax final : public JsonSax {
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
		if (values_ > Limits::kJsonValues) {
			return Fail(MakeError(ErrorCode::Limit, "values"));
		}
		return true;
	}

	bool CheckString(const std::string &value)
	{
		if (value.size() > Limits::kJsonStringBytes) {
			return Fail(MakeError(ErrorCode::Limit, "string"));
		}
		if (value.find('\0') != std::string::npos) {
			return Fail(MakeError(ErrorCode::Nul));
		}
		return true;
	}

	bool Attach(Json value)
	{
		if (stack_.empty()) {
			if (has_root_) {
				return Fail(MakeError(ErrorCode::Json));
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
			return Fail(MakeError(ErrorCode::Json));
		}
		frame.value[*frame.pending_key] = std::move(value);
		frame.pending_key.reset();
		return true;
	}

public:
	BoundedSax() = default;

	[[nodiscard]] const Json &root() const noexcept { return root_; }
	[[nodiscard]] const Error &error() const noexcept { return error_; }

	bool null() override { return BeforeValue() && Attach(nullptr); }
	bool boolean(bool value) override { return BeforeValue() && Attach(value); }
	bool number_integer(number_integer_t value) override { return BeforeValue() && Attach(value); }
	bool number_unsigned(number_unsigned_t value) override { return BeforeValue() && Attach(value); }

	bool number_float(number_float_t value, const string_t &) override
	{
		if (!std::isfinite(value)) {
			return Fail(MakeError(ErrorCode::Nonfinite));
		}
		return BeforeValue() && Attach(value);
	}

	bool string(string_t &value) override
	{
		return CheckString(value) && BeforeValue() && Attach(std::move(value));
	}

	bool binary(binary_t &) override { return Fail(MakeError(ErrorCode::Json)); }

	bool start_object(std::size_t) override
	{
		if (!BeforeValue()) {
			return false;
		}
		if (stack_.size() + 1U > Limits::kJsonDepth) {
			return Fail(MakeError(ErrorCode::Limit, "depth"));
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
			return Fail(MakeError(ErrorCode::DuplicateKey));
		}
		frame.pending_key = std::move(value);
		return true;
	}

	bool end_object() override
	{
		if (stack_.empty() || stack_.back().kind != FrameKind::Object ||
		    stack_.back().pending_key.has_value()) {
			return Fail(MakeError(ErrorCode::Json));
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
		if (stack_.size() + 1U > Limits::kJsonDepth) {
			return Fail(MakeError(ErrorCode::Limit, "depth"));
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
			return Fail(MakeError(ErrorCode::Json));
		}
		Json value = std::move(stack_.back().value);
		stack_.pop_back();
		return Attach(std::move(value));
	}

	bool parse_error(std::size_t, const std::string &, const nlohmann::detail::exception &exception) override
	{
		if (error_.ok()) {
			error_ = MakeError(exception.id == 406 ? ErrorCode::Nonfinite : ErrorCode::Json);
		}
		return false;
	}
};

Result<Json> ParseJson(std::string_view bytes)
{
	try {
		if (bytes.size() > Limits::kMappingBytes) {
			return {{}, MakeError(ErrorCode::Limit, "mapping")};
		}
		if (bytes.size() >= 3U && static_cast<unsigned char>(bytes[0]) == 0xefU &&
		    static_cast<unsigned char>(bytes[1]) == 0xbbU && static_cast<unsigned char>(bytes[2]) == 0xbfU) {
			return {{}, MakeError(ErrorCode::Bom)};
		}
		if (bytes.find('\0') != std::string_view::npos) {
			return {{}, MakeError(ErrorCode::Nul)};
		}
		if (!IsValidUtf8(bytes)) {
			return {{}, MakeError(ErrorCode::Utf8)};
		}
		BoundedSax sax;
		bool parsed = false;
		try {
			parsed = Json::sax_parse(bytes.begin(), bytes.end(), &sax, Json::input_format_t::json, true);
		} catch (...) {
			return {{}, MakeError(ErrorCode::Json)};
		}
		if (!parsed || !sax.error().ok()) {
			return {{}, sax.error().ok() ? MakeError(ErrorCode::Json) : sax.error()};
		}
		return {sax.root(), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Json)};
	}
}

Result<std::string> ReadUtf8File(const std::string &path)
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(path);
		std::error_code status_error;
		const std::filesystem::file_status status = std::filesystem::status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return {{}, MakeError(ErrorCode::Io)};
		}
		std::error_code size_error;
		const std::uintmax_t size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error) {
			return {{}, MakeError(ErrorCode::Io)};
		}
		if (size > Limits::kMappingBytes || size > std::numeric_limits<std::size_t>::max()) {
			return {{}, MakeError(ErrorCode::Limit, "mapping")};
		}
		std::ifstream input;
		input.open(filesystem_path, std::ios::in | std::ios::binary);
		if (!input.is_open()) {
			return {{}, MakeError(ErrorCode::Io)};
		}
		std::string bytes(static_cast<std::size_t>(size), '\0');
		if (!bytes.empty()) {
			input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
				return {{}, MakeError(ErrorCode::Io)};
			}
		}
		return {std::move(bytes), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Io)};
	}
}

bool IsId(std::string_view value)
{
	if (value.empty() || value.size() > 64U) {
		return false;
	}
	for (const char character : value) {
		if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
		      character == '_')) {
			return false;
		}
	}
	return true;
}

bool IsRequirement(std::string_view value)
{
	if (value.empty() || value.size() > 64U) {
		return false;
	}
	bool has_non_ascii = false;
	for (const unsigned char character : value) {
		if (character >= 0x80U) {
			has_non_ascii = true;
			continue;
		}
		if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
		      character == '_' || character == '.')) {
			return false;
		}
	}
	return has_non_ascii ||
	       value.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_.") == std::string_view::npos;
}

bool GetString(const Json &object, const char *key, std::string &result)
{
	const auto iterator = object.find(key);
	if (iterator == object.end() || !iterator->is_string()) {
		return false;
	}
	result = iterator->get<std::string>();
	return true;
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

bool GetInt(const Json &value, std::int64_t &result)
{
	if (value.is_number_integer()) {
		result = value.get<std::int64_t>();
		return true;
	}
	if (value.is_number_unsigned()) {
		const std::uint64_t unsigned_value = value.get<std::uint64_t>();
		if (unsigned_value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
			return false;
		}
		result = static_cast<std::int64_t>(unsigned_value);
		return true;
	}
	return false;
}

bool GetNumber(const Json &value, double &result)
{
	if (!value.is_number()) {
		return false;
	}
	result = value.get<double>();
	return std::isfinite(result);
}

bool IsField(const Json &object, const char *key)
{
	return object.find(key) != object.end();
}

Result<PatternSegment> ParsePatternSegment(const Json &value, std::string_view error_field)
{
	if (!value.is_object() || value.size() != 1U) {
		return {{}, Schema(error_field)};
	}
	const auto iterator = value.begin();
	const std::string &key = iterator.key();
	const Json &data = iterator.value();
	if (key == "s" || key == "t") {
		if (!data.is_string()) {
			return {{}, Schema(error_field)};
		}
		return key == "s" ? PatternSegment::Symbol(data.get<std::string>())
				  : PatternSegment::Text(data.get<std::string>());
	}
	if (key == "i") {
		std::uint64_t integer = 0U;
		if (!GetUInt(data, integer) || integer > (std::uint64_t(1U) << 53U)) {
			return {{}, Schema(error_field)};
		}
		return PatternSegment::Int(integer);
	}
	if (key == "any") {
		if (!data.is_boolean() || !data.get<bool>()) {
			return {{}, Schema(error_field)};
		}
		return {PatternSegment::Any(), {}};
	}
	if (key == "r") {
		if (!data.is_array() || data.size() != 2U) {
			return {{}, Schema(error_field)};
		}
		std::uint64_t low = 0U;
		std::uint64_t high = 0U;
		if (!GetUInt(data[0], low) || !GetUInt(data[1], high) || low > high) {
			return {{}, Schema(error_field)};
		}
		return PatternSegment::Range(low, high);
	}
	if (key == "in") {
		if (!data.is_array() || data.empty() || data.size() > 64U) {
			return {{}, Schema(error_field)};
		}
		std::vector<std::uint64_t> values;
		values.reserve(data.size());
		for (const Json &entry : data) {
			std::uint64_t integer = 0U;
			if (!GetUInt(entry, integer) ||
			    std::find(values.begin(), values.end(), integer) != values.end()) {
				return {{}, Schema(error_field)};
			}
			values.push_back(integer);
		}
		return PatternSegment::Set(std::move(values));
	}
	return {{}, Schema(error_field)};
}

Result<ControlPattern> ParsePattern(const Json &value, std::string_view error_field, bool allow_empty)
{
	if (!value.is_array()) {
		return {{}, Schema(error_field)};
	}
	if (value.empty() && !allow_empty) {
		return {{}, Schema(error_field)};
	}
	if (value.size() > Limits::kAddressSegments) {
		return {{}, MakeError(ErrorCode::Limit, "address")};
	}
	std::vector<PatternSegment> segments;
	segments.reserve(value.size());
	for (const Json &entry : value) {
		auto segment = ParsePatternSegment(entry, error_field);
		if (!segment.ok()) {
			return {{}, segment.error};
		}
		segments.push_back(std::move(*segment.value));
	}
	return ControlPattern::Create(std::move(segments));
}

Result<ControlAddress> ParseConcreteAddress(const Json &value, std::string_view error_field, bool allow_empty)
{
	if (!value.is_array()) {
		return {{}, Schema(error_field)};
	}
	if (value.empty() && !allow_empty) {
		return {{}, Schema(error_field)};
	}
	if (value.size() > Limits::kAddressSegments) {
		return {{}, MakeError(ErrorCode::Limit, "address")};
	}
	std::vector<AddressSegment> segments;
	segments.reserve(value.size());
	for (const Json &entry : value) {
		if (!entry.is_object() || entry.size() != 1U) {
			return {{}, Schema(error_field)};
		}
		const auto iterator = entry.begin();
		if (iterator.key() == "s" && iterator.value().is_string()) {
			auto segment = AddressSegment::Symbol(iterator.value().get<std::string>());
			if (!segment.ok()) {
				return {{}, segment.error};
			}
			segments.push_back(std::move(*segment.value));
		} else if (iterator.key() == "t" && iterator.value().is_string()) {
			auto segment = AddressSegment::Text(iterator.value().get<std::string>());
			if (!segment.ok()) {
				return {{}, segment.error};
			}
			segments.push_back(std::move(*segment.value));
		} else if (iterator.key() == "i") {
			std::uint64_t integer = 0U;
			if (!GetUInt(iterator.value(), integer) || integer > (std::uint64_t(1U) << 53U)) {
				return {{}, Schema(error_field)};
			}
			auto segment = AddressSegment::Int(integer);
			segments.push_back(std::move(*segment.value));
		} else {
			return {{}, Schema(error_field)};
		}
	}
	return ControlAddress::Create(std::move(segments));
}

Result<Predicate> ParsePredicate(const Json &value)
{
	if (!value.is_object() || value.size() != 1U) {
		return {{}, Schema("guard")};
	}
	const auto iterator = value.begin();
	if (iterator.key() == "value") {
		if (!iterator.value().is_object()) {
			return {{}, Schema("guard")};
		}
		double minimum = 0.0;
		double maximum = 0.0;
		if (!GetNumber(iterator.value().value("min", Json()), minimum) ||
		    !GetNumber(iterator.value().value("max", Json()), maximum) || minimum > maximum) {
			return {{}, Schema("guard")};
		}
		Predicate result;
		result.kind = PredicateKind::Value;
		result.minimum = minimum;
		result.maximum = maximum;
		return {std::move(result), {}};
	}
	if (iterator.key() == "modifier" || iterator.key() == "layer") {
		if (!iterator.value().is_string()) {
			return {{}, Schema("guard")};
		}
		Predicate result;
		result.kind = iterator.key() == "modifier" ? PredicateKind::Modifier : PredicateKind::Layer;
		result.name = iterator.value().get<std::string>();
		return {std::move(result), {}};
	}
	if (iterator.key() == "bank") {
		std::uint64_t bank = 0U;
		if (!GetUInt(iterator.value(), bank)) {
			return {{}, Schema("guard")};
		}
		Predicate result;
		result.kind = PredicateKind::Bank;
		result.bank = bank;
		return {std::move(result), {}};
	}
	return {{}, Schema("guard")};
}

Result<Guard> ParseGuard(const Json &value)
{
	if (!value.is_object()) {
		return {{}, Schema("guard")};
	}
	const auto iterator = value.find("all");
	if (iterator == value.end() || !iterator->is_array()) {
		return {{}, Schema("guard")};
	}
	Guard result;
	result.all.reserve(iterator->size());
	for (const Json &entry : *iterator) {
		auto predicate = ParsePredicate(entry);
		if (!predicate.ok()) {
			return {{}, predicate.error};
		}
		result.all.push_back(std::move(*predicate.value));
	}
	return {std::move(result), {}};
}

bool IsFiniteParamArray(const Json &value, std::size_t count, std::vector<double> &result)
{
	if (!value.is_array() || value.size() != count) {
		return false;
	}
	result.clear();
	result.reserve(count);
	for (const Json &entry : value) {
		double number = 0.0;
		if (!GetNumber(entry, number)) {
			return false;
		}
		result.push_back(number);
	}
	return true;
}

Result<TransformSpec> ParseTransform(const Json &value)
{
	if (!value.is_object()) {
		return {{}, Schema("transforms")};
	}
	std::string operation;
	if (!GetString(value, "op", operation)) {
		return {{}, Schema("transforms")};
	}
	TransformSpec result;
	result.op = operation;
	if (operation == "scale") {
		if (!IsFiniteParamArray(value.value("in", Json()), 2U, result.numbers) ||
		    !IsFiniteParamArray(value.value("out", Json()), 2U, result.numbers)) {
			return {{}, Schema("transforms")};
		}
		std::vector<double> input;
		std::vector<double> output;
		if (!IsFiniteParamArray(value.at("in"), 2U, input) ||
		    !IsFiniteParamArray(value.at("out"), 2U, output) || input[0] == input[1]) {
			return {{}, Schema("transforms")};
		}
		result.numbers = {input[0], input[1], output[0], output[1]};
	} else if (operation == "curve") {
		if (!GetString(value, "kind", result.text) ||
		    (result.text != "linear" && result.text != "log" && result.text != "exp")) {
			return {{}, Schema("transforms")};
		}
	} else if (operation == "invert" || operation == "toggle" || operation == "momentary" ||
		   operation == "join14" || operation == "split14") {
		// The operation has no required parameters in K1.
	} else if (operation == "clamp") {
		double minimum = 0.0;
		double maximum = 0.0;
		if (!GetNumber(value.value("min", Json()), minimum) ||
		    !GetNumber(value.value("max", Json()), maximum) || minimum > maximum) {
			return {{}, Schema("transforms")};
		}
		result.numbers = {minimum, maximum};
	} else if (operation == "deadzone") {
		double width = 0.0;
		if (!GetNumber(value.value("width", Json()), width) || width < 0.0 || width >= 1.0) {
			return {{}, Schema("transforms")};
		}
		result.numbers = {width};
	} else if (operation == "quantize") {
		std::int64_t steps = 0;
		if (!GetInt(value.value("steps", Json()), steps) || steps < 2) {
			return {{}, Schema("transforms")};
		}
		result.integer = steps;
	} else if (operation == "threshold") {
		double threshold = 0.0;
		if (!GetNumber(value.value("at", Json()), threshold)) {
			return {{}, Schema("transforms")};
		}
		result.numbers = {threshold};
	} else if (operation == "relative") {
		if (!GetString(value, "encoding", result.text) ||
		    (result.text != "twos_complement" && result.text != "sign_magnitude" &&
		     result.text != "binary_offset")) {
			return {{}, Schema("transforms")};
		}
		double step = 0.0;
		if (!GetNumber(value.value("step", Json()), step) || step <= 0.0) {
			return {{}, Schema("transforms")};
		}
		result.numbers = {step};
	} else {
		return {{}, MakeError(ErrorCode::Schema, "op")};
	}
	return {std::move(result), {}};
}

bool ParseEnum(const Json &object, const char *key, const std::vector<std::string> &allowed, std::string &result)
{
	if (!GetString(object, key, result)) {
		return false;
	}
	return std::find(allowed.begin(), allowed.end(), result) != allowed.end();
}

Result<Target> ParseTarget(const Json &value, bool allow_empty_address)
{
	if (!value.is_object()) {
		return {{}, Schema("target")};
	}
	std::string kind;
	if (!GetString(value, "kind", kind)) {
		return {{}, Schema("target")};
	}
	Target result;
	if (kind == "source-property" || kind == "filter-property") {
		result.kind = kind == "source-property" ? TargetKind::SourceProperty : TargetKind::FilterProperty;
		if (!GetString(value, "id", result.id) || !GetString(value, "field", result.field) ||
		    result.id.empty() || result.field.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "scene") {
		result.kind = TargetKind::Scene;
		if (!GetString(value, "id", result.id) || result.id.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "scene-item") {
		result.kind = TargetKind::SceneItem;
		std::uint64_t item_id = 0U;
		if (!GetString(value, "scene_id", result.scene_id) ||
		    !GetUInt(value.value("item_id", Json()), item_id) || item_id > (std::uint64_t(1U) << 53U) ||
		    result.scene_id.empty()) {
			return {{}, Schema("target")};
		}
		result.item_id = item_id;
	} else if (kind == "mixer") {
		result.kind = TargetKind::Mixer;
		const std::vector<std::string> fields = {"volume",     "mute",   "balance",
							 "monitoring", "mixers", "sync_offset"};
		if (!GetString(value, "id", result.id) || !ParseEnum(value, "field", fields, result.field) ||
		    result.id.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "transition") {
		result.kind = TargetKind::Transition;
		if (!GetString(value, "name", result.name) || result.name.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "hotkey") {
		result.kind = TargetKind::Hotkey;
		std::string effect;
		const std::vector<std::string> effects = {"none", "scene", "mixer", "output", "unknown"};
		if (!GetString(value, "registerer_kind", result.registerer_kind) ||
		    !GetString(value, "registerer_id", result.registerer_id) ||
		    !GetString(value, "name", result.name) || !ParseEnum(value, "effect", effects, effect)) {
			return {{}, Schema("target")};
		}
		if (effect == "none") {
			result.effect = HotkeyEffect::None;
		} else if (effect == "scene") {
			result.effect = HotkeyEffect::Scene;
		} else if (effect == "mixer") {
			result.effect = HotkeyEffect::Mixer;
		} else if (effect == "output") {
			result.effect = HotkeyEffect::Output;
		} else {
			result.effect = HotkeyEffect::Unknown;
		}
	} else if (kind == "output") {
		result.kind = TargetKind::Output;
		std::string output;
		std::string action;
		const std::vector<std::string> outputs = {"stream", "record", "replay", "virtualcam"};
		const std::vector<std::string> actions = {"start", "stop", "toggle"};
		if (!ParseEnum(value, "output", outputs, output) || !ParseEnum(value, "action", actions, action)) {
			return {{}, Schema("target")};
		}
		result.output = output == "stream"   ? OutputKind::Stream
				: output == "record" ? OutputKind::Record
				: output == "replay" ? OutputKind::Replay
						     : OutputKind::VirtualCam;
		result.action = action == "start"  ? OutputAction::Start
				: action == "stop" ? OutputAction::Stop
						   : OutputAction::Toggle;
	} else if (kind == "profile") {
		result.kind = TargetKind::Profile;
		if (!GetString(value, "name", result.name) || result.name.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "plugin-param") {
		result.kind = TargetKind::PluginParam;
		if (!GetString(value, "instance", result.instance) || !GetString(value, "param", result.parameter) ||
		    result.instance.empty() || result.parameter.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "plugin-event") {
		result.kind = TargetKind::PluginEvent;
		if (!GetString(value, "instance", result.instance) || result.instance.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "graph") {
		result.kind = TargetKind::Graph;
		if (!GetString(value, "node", result.node) || !GetString(value, "port", result.port) ||
		    result.node.empty() || result.port.empty()) {
			return {{}, Schema("target")};
		}
	} else if (kind == "egress") {
		result.kind = TargetKind::Egress;
		std::string trust;
		const std::vector<std::string> trusts = {"hw", "virtual", "loopback-net", "remote-net"};
		if (!GetString(value, "endpoint", result.id) || !ParseEnum(value, "trust", trusts, trust) ||
		    result.id.empty()) {
			return {{}, Schema("target")};
		}
		result.trust = trust == "hw"             ? TrustClass::Hw
			       : trust == "virtual"      ? TrustClass::Virtual
			       : trust == "loopback-net" ? TrustClass::LoopbackNet
							 : TrustClass::RemoteNet;
	} else if (kind == "emit") {
		result.kind = TargetKind::Emit;
		auto address = ParseConcreteAddress(value.value("address", Json()), "target", allow_empty_address);
		if (!address.ok()) {
			return {{}, address.error};
		}
		result.emit_address = std::move(*address.value);
	} else {
		return {{}, Schema("target")};
	}
	return {std::move(result), {}};
}

Result<MergeSpec> ParseMerge(const Json &value)
{
	if (!value.is_object()) {
		return {{}, Schema("merge")};
	}
	std::string policy;
	const std::vector<std::string> policies = {"last-writer", "priority", "touch-lease", "sum",
						   "min",         "max",      "boolean-or"};
	if (!ParseEnum(value, "policy", policies, policy)) {
		return {{}, Schema("merge")};
	}
	std::int64_t priority = 0;
	if (IsField(value, "priority") && !GetInt(value.at("priority"), priority)) {
		return {{}, Schema("merge")};
	}
	MergeSpec result;
	result.policy = policy == "last-writer"   ? MergePolicy::LastWriter
			: policy == "priority"    ? MergePolicy::Priority
			: policy == "touch-lease" ? MergePolicy::TouchLease
			: policy == "sum"         ? MergePolicy::Sum
			: policy == "min"         ? MergePolicy::Min
			: policy == "max"         ? MergePolicy::Max
						  : MergePolicy::BooleanOr;
	result.priority = priority;
	return {std::move(result), {}};
}

Result<Slot> ParseSlot(const Json &value, bool require_r1_fields)
{
	if (!value.is_object()) {
		return {{}, Schema("slots")};
	}
	Slot result;
	if (!GetString(value, "id", result.id) || !IsId(result.id)) {
		return {{}, Schema("slots")};
	}
	const auto iterator = value.find("requires");
	if (iterator == value.end() && !require_r1_fields) {
		return {std::move(result), {}};
	}
	if (iterator == value.end() || !iterator->is_array() || iterator->size() > 64U) {
		return {{}, Schema("requires")};
	}
	result.requirements.reserve(iterator->size());
	for (const Json &requirement : *iterator) {
		if (!requirement.is_string() || !IsRequirement(requirement.get<std::string>())) {
			return {{}, Schema("requires")};
		}
		result.requirements.push_back(requirement.get<std::string>());
	}
	return {std::move(result), {}};
}

Result<Route> ParseRoute(const Json &value, bool require_r1_fields, bool allow_empty_address)
{
	if (!value.is_object()) {
		return {{}, Schema("routes")};
	}
	Route result;
	if (!GetString(value, "id", result.id) || !IsId(result.id)) {
		return {{}, Schema("routes")};
	}
	if (!IsField(value, "enabled") && !require_r1_fields) {
		result.enabled = true;
	} else if (!IsField(value, "enabled") || !value.at("enabled").is_boolean()) {
		return {{}, Schema("routes")};
	} else {
		result.enabled = value.at("enabled").get<bool>();
	}
	const auto match = value.find("match");
	if (match == value.end() || !match->is_object()) {
		return {{}, Schema("match")};
	}
	const auto slot = match->find("slot");
	if (slot == match->end()) {
		return {{}, Schema("match")};
	}
	if (!slot->is_null()) {
		if (!slot->is_string()) {
			return {{}, Schema("match")};
		}
		result.match.slot = slot->get<std::string>();
	}
	const auto address = match->find("address");
	if (address == match->end()) {
		return {{}, Schema("address")};
	}
	auto parsed_address = ParsePattern(*address, "address", allow_empty_address);
	if (!parsed_address.ok()) {
		return {{}, parsed_address.error};
	}
	result.match.address = std::move(*parsed_address.value);
	const auto guard = value.find("guard");
	if (guard != value.end()) {
		auto parsed_guard = ParseGuard(*guard);
		if (!parsed_guard.ok()) {
			return {{}, parsed_guard.error};
		}
		result.guard = std::move(*parsed_guard.value);
	}
	const auto transforms = value.find("transforms");
	if (transforms != value.end()) {
		if (!transforms->is_array()) {
			return {{}, Schema("transforms")};
		}
		if (transforms->size() > Limits::kTransforms) {
			return {{}, MakeError(ErrorCode::Limit, "transforms")};
		}
		result.transforms.reserve(transforms->size());
		for (const Json &entry : *transforms) {
			auto transform = ParseTransform(entry);
			if (!transform.ok()) {
				return {{}, transform.error};
			}
			result.transforms.push_back(std::move(*transform.value));
		}
	}
	const auto target = value.find("target");
	if (target == value.end()) {
		return {{}, Schema("target")};
	}
	auto parsed_target = ParseTarget(*target, allow_empty_address);
	if (!parsed_target.ok()) {
		return {{}, parsed_target.error};
	}
	result.target = std::move(*parsed_target.value);
	const auto merge = value.find("merge");
	if (merge != value.end()) {
		auto parsed_merge = ParseMerge(*merge);
		if (!parsed_merge.ok()) {
			return {{}, parsed_merge.error};
		}
		result.merge = std::move(*parsed_merge.value);
	}
	return {std::move(result), {}};
}

bool PatternHasWildcard(const ControlPattern &pattern) noexcept
{
	for (const PatternSegment &segment : pattern.segments) {
		if (segment.kind == PatternSegmentKind::Any || segment.kind == PatternSegmentKind::Range ||
		    segment.kind == PatternSegmentKind::Set) {
			return true;
		}
	}
	return false;
}

bool IsSecretField(std::string_view field)
{
	std::string folded;
	folded.reserve(field.size());
	for (const char character : field) {
		folded.push_back(character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a')
								      : character);
	}
	const char *const denied[] = {"key",   "stream_key",    "bearer_token", "username",   "password", "server",
				      "token", "refresh_token", "auth",         "passphrase", "secret",   "streamid"};
	for (const char *entry : denied) {
		if (folded == entry) {
			return true;
		}
	}
	return false;
}

std::string TargetKindName(TargetKind kind)
{
	switch (kind) {
	case TargetKind::SourceProperty:
		return "source-property";
	case TargetKind::FilterProperty:
		return "filter-property";
	case TargetKind::Scene:
		return "scene";
	case TargetKind::SceneItem:
		return "scene-item";
	case TargetKind::Mixer:
		return "mixer";
	case TargetKind::Transition:
		return "transition";
	case TargetKind::Hotkey:
		return "hotkey";
	case TargetKind::Output:
		return "output";
	case TargetKind::Profile:
		return "profile";
	case TargetKind::PluginParam:
		return "plugin-param";
	case TargetKind::PluginEvent:
		return "plugin-event";
	case TargetKind::Graph:
		return "graph";
	case TargetKind::Egress:
		return "egress";
	case TargetKind::Emit:
		return "emit";
	}
	return {};
}

std::string OutputName(OutputKind output)
{
	switch (output) {
	case OutputKind::Stream:
		return "stream";
	case OutputKind::Record:
		return "record";
	case OutputKind::Replay:
		return "replay";
	case OutputKind::VirtualCam:
		return "virtualcam";
	}
	return {};
}

std::string SegmentKey(const AddressSegment &segment)
{
	switch (segment.kind) {
	case AddressSegmentKind::Symbol:
		return "s:" + segment.text;
	case AddressSegmentKind::Int:
		return "i:" + std::to_string(segment.integer);
	case AddressSegmentKind::Text:
		return "t:" + segment.text;
	}
	return {};
}

std::string AddressKey(const ControlAddress &address)
{
	std::string result;
	for (const AddressSegment &segment : address.segments) {
		if (!result.empty()) {
			result.push_back('/');
		}
		result += SegmentKey(segment);
	}
	return result;
}

bool ValidateModel(const Mapping &mapping, Error &error)
{
	if (mapping.major != 1 || mapping.minor > (std::uint64_t(1U) << 53U)) {
		error = mapping.major != 1 ? MakeError(ErrorCode::Major) : Schema("minor");
		return false;
	}
	if (mapping.slots.size() > Limits::kMappingSlots) {
		error = MakeError(ErrorCode::Limit, "slots");
		return false;
	}
	if (mapping.routes.size() > Limits::kMappingRoutes) {
		error = MakeError(ErrorCode::Limit, "routes");
		return false;
	}
	std::set<std::string> slot_ids;
	for (const Slot &slot : mapping.slots) {
		if (!IsId(slot.id)) {
			error = Schema("slots");
			return false;
		}
		if (!slot_ids.insert(slot.id).second) {
			error = MakeError(ErrorCode::Duplicate, slot.id);
			return false;
		}
	}
	std::set<std::string> route_ids;
	for (const Route &route : mapping.routes) {
		if (!IsId(route.id)) {
			error = Schema("routes");
			return false;
		}
		if (!route_ids.insert(route.id).second) {
			error = MakeError(ErrorCode::Duplicate, route.id);
			return false;
		}
		if (route.match.address.segments.size() > Limits::kAddressSegments) {
			error = MakeError(ErrorCode::Limit, "address");
			return false;
		}
		if (route.transforms.size() > Limits::kTransforms) {
			error = MakeError(ErrorCode::Limit, "transforms");
			return false;
		}
		if (route.match.slot.has_value() && slot_ids.find(*route.match.slot) == slot_ids.end()) {
			error = MakeError(ErrorCode::Reference, *route.match.slot);
			return false;
		}
	}
	return true;
}

Error FindCycleError(const Mapping &mapping)
{
	const std::size_t route_count = mapping.routes.size();
	std::vector<std::vector<std::size_t>> edges(route_count);
	for (std::size_t source = 0U; source < route_count; ++source) {
		const Route &source_route = mapping.routes[source];
		if (!source_route.enabled || source_route.target.kind != TargetKind::Emit) {
			continue;
		}
		for (std::size_t target = 0U; target < route_count; ++target) {
			const Route &target_route = mapping.routes[target];
			if (!target_route.enabled ||
			    (target_route.match.slot.has_value() &&
			     target_route.match.slot != source_route.match.slot) ||
			    !Matches(target_route.match.address, source_route.target.emit_address)) {
				continue;
			}
			edges[source].push_back(target);
		}
	}

	std::vector<int> index(route_count, -1);
	std::vector<int> lowlink(route_count, -1);
	std::vector<bool> on_stack(route_count, false);
	std::vector<std::size_t> stack;
	int next_index = 0;
	std::size_t first_cycle = route_count;
	// Iterative Tarjan: a chain of 10,000 routes would overflow the stack with recursion (a 1 MiB
	// Windows stack, or any stack under AddressSanitizer).
	struct Frame {
		std::size_t node;
		std::size_t next_edge;
	};
	std::vector<Frame> call_stack;
	const auto finish = [&](std::size_t node) {
		if (lowlink[node] != index[node]) {
			return;
		}
		std::size_t component_size = 0U;
		std::size_t component_first = route_count;
		while (!stack.empty()) {
			const std::size_t member = stack.back();
			stack.pop_back();
			on_stack[member] = false;
			++component_size;
			component_first = std::min(component_first, member);
			if (member == node) {
				break;
			}
		}
		const bool self_cycle = component_size == 1U &&
					std::find(edges[node].begin(), edges[node].end(), node) != edges[node].end();
		if (component_size > 1U || self_cycle) {
			first_cycle = std::min(first_cycle, component_first);
		}
	};
	const auto enter = [&](std::size_t node) {
		index[node] = next_index;
		lowlink[node] = next_index;
		++next_index;
		stack.push_back(node);
		on_stack[node] = true;
		call_stack.push_back({node, 0U});
	};
	for (std::size_t route = 0U; route < route_count; ++route) {
		if (index[route] != -1) {
			continue;
		}
		enter(route);
		while (!call_stack.empty()) {
			Frame &frame = call_stack.back();
			const std::size_t node = frame.node;
			if (frame.next_edge < edges[node].size()) {
				const std::size_t adjacent = edges[node][frame.next_edge++];
				if (index[adjacent] == -1) {
					enter(adjacent); // invalidates `frame`
				} else if (on_stack[adjacent]) {
					lowlink[node] = std::min(lowlink[node], index[adjacent]);
				}
				continue;
			}
			call_stack.pop_back();
			finish(node);
			if (!call_stack.empty()) {
				const std::size_t parent = call_stack.back().node;
				lowlink[parent] = std::min(lowlink[parent], lowlink[node]);
			}
		}
	}
	if (first_cycle == route_count) {
		return {};
	}
	return MakeError(ErrorCode::Cycle, mapping.routes[first_cycle].id);
}

} // namespace

const char *CapabilityClassName(CapabilityClass capability) noexcept
{
	switch (capability) {
	case CapabilityClass::Mix:
		return "mix";
	case CapabilityClass::Scene:
		return "scene";
	case CapabilityClass::SourceProperty:
		return "source-property";
	case CapabilityClass::Filter:
		return "filter";
	case CapabilityClass::PluginParam:
		return "plugin-param";
	case CapabilityClass::Hotkey:
		return "hotkey";
	case CapabilityClass::OutputControl:
		return "output-control";
	case CapabilityClass::Profile:
		return "profile";
	case CapabilityClass::Meta:
		return "meta";
	case CapabilityClass::EgressByTrust:
		return "egress-by-trust";
	}
	return "meta";
}

CapabilityClass ClassOfTarget(const Target &target) noexcept
{
	switch (target.kind) {
	case TargetKind::SourceProperty:
		return CapabilityClass::SourceProperty;
	case TargetKind::FilterProperty:
		return CapabilityClass::Filter;
	case TargetKind::Scene:
	case TargetKind::SceneItem:
	case TargetKind::Transition:
		return CapabilityClass::Scene;
	case TargetKind::Mixer:
	case TargetKind::Graph:
		return CapabilityClass::Mix;
	case TargetKind::Hotkey:
		if (target.effect == HotkeyEffect::Output || target.effect == HotkeyEffect::Unknown) {
			return CapabilityClass::OutputControl;
		}
		if (target.effect == HotkeyEffect::Scene) {
			return CapabilityClass::Scene;
		}
		if (target.effect == HotkeyEffect::Mixer) {
			return CapabilityClass::Mix;
		}
		return CapabilityClass::Hotkey;
	case TargetKind::Output:
		return CapabilityClass::OutputControl;
	case TargetKind::Profile:
		return CapabilityClass::Profile;
	case TargetKind::PluginParam:
	case TargetKind::PluginEvent:
		return CapabilityClass::PluginParam;
	case TargetKind::Egress:
		return CapabilityClass::EgressByTrust;
	case TargetKind::Emit:
		return CapabilityClass::Meta;
	}
	return CapabilityClass::Meta;
}

const char *MergePolicyName(MergePolicy policy) noexcept
{
	switch (policy) {
	case MergePolicy::LastWriter:
		return "last-writer";
	case MergePolicy::Priority:
		return "priority";
	case MergePolicy::TouchLease:
		return "touch-lease";
	case MergePolicy::Sum:
		return "sum";
	case MergePolicy::Min:
		return "min";
	case MergePolicy::Max:
		return "max";
	case MergePolicy::BooleanOr:
		return "boolean-or";
	}
	return "last-writer";
}

std::size_t PatternFanout(const ControlPattern &pattern) noexcept
{
	std::size_t product = 1U;
	for (const PatternSegment &segment : pattern.segments) {
		std::size_t factor = 1U;
		if (segment.kind == PatternSegmentKind::Range) {
			const std::uint64_t distance = segment.high - segment.low;
			factor = distance >= Limits::kWildcardFanout ? Limits::kWildcardFanout + 1U
								     : static_cast<std::size_t>(distance + 1U);
		} else if (segment.kind == PatternSegmentKind::Set) {
			factor = segment.set.size();
		}
		if (factor != 0U && product > Limits::kWildcardFanout / factor) {
			return Limits::kWildcardFanout + 1U;
		}
		product *= factor;
	}
	return product;
}

std::string TargetKey(const Target &target)
{
	auto escape_value = [](std::string_view value) {
		std::string escaped;
		escaped.reserve(value.size());
		for (const char character : value) {
			if (character == '|' || character == '=') {
				escaped.push_back('\\');
			}
			escaped.push_back(character);
		}
		return escaped;
	};

	try {
		std::vector<std::pair<std::string, std::string>> fields;
		switch (target.kind) {
		case TargetKind::SourceProperty:
		case TargetKind::FilterProperty:
			fields = {{"field", target.field}, {"id", target.id}};
			break;
		case TargetKind::Scene:
			fields = {{"id", target.id}};
			break;
		case TargetKind::SceneItem:
			fields = {{"item_id", std::to_string(target.item_id)}, {"scene_id", target.scene_id}};
			break;
		case TargetKind::Mixer:
			fields = {{"field", target.field}, {"id", target.id}};
			break;
		case TargetKind::Transition:
			fields = {{"name", target.name}};
			break;
		case TargetKind::Hotkey:
			fields = {{"name", target.name},
				  {"registerer_id", target.registerer_id},
				  {"registerer_kind", target.registerer_kind}};
			break;
		case TargetKind::Output:
			fields = {{"output", OutputName(target.output)}};
			break;
		case TargetKind::Profile:
			fields = {{"name", target.name}};
			break;
		case TargetKind::PluginParam:
			fields = {{"instance", target.instance}, {"param", target.parameter}};
			break;
		case TargetKind::PluginEvent:
			fields = {{"instance", target.instance}};
			break;
		case TargetKind::Graph:
			fields = {{"node", target.node}, {"port", target.port}};
			break;
		case TargetKind::Egress:
			fields = {{"endpoint", target.id}};
			break;
		case TargetKind::Emit:
			fields = {{"address", AddressKey(target.emit_address)}};
			break;
		}
		std::sort(fields.begin(), fields.end());
		std::string result = TargetKindName(target.kind);
		for (const auto &field : fields) {
			result += '|';
			result += field.first;
			result += '=';
			result += escape_value(field.second);
		}
		return result;
	} catch (...) {
		return {};
	}
}

Result<Mapping> ParseMappingInternal(std::string_view bytes, bool require_r1_fields)
{
	auto document = ParseJson(bytes);
	if (!document.ok()) {
		return {{}, document.error};
	}
	if (!document.value->is_object()) {
		return {{}, Schema("root")};
	}
	const Json &root = *document.value;
	const auto format = root.find("format");
	if (format == root.end() || !format->is_string() || format->get<std::string>() != "jeiobs-control") {
		return {{}, Schema("format")};
	}
	const auto major = root.find("major");
	std::int64_t major_number = 0;
	if (major == root.end() || !GetInt(*major, major_number)) {
		return {{}, Schema("major")};
	}
	if (major_number != 1) {
		return {{}, MakeError(ErrorCode::Major)};
	}
	const auto minor = root.find("minor");
	std::uint64_t minor_number = 0U;
	if (minor == root.end() || !GetUInt(*minor, minor_number) || minor_number > (std::uint64_t(1U) << 53U)) {
		return {{}, Schema("minor")};
	}
	const auto slots = root.find("slots");
	if (slots == root.end() || !slots->is_array()) {
		return {{}, Schema("slots")};
	}
	if (slots->size() > Limits::kMappingSlots) {
		return {{}, MakeError(ErrorCode::Limit, "slots")};
	}
	Mapping mapping;
	mapping.major = static_cast<int>(major_number);
	mapping.minor = minor_number;
	mapping.slots.reserve(slots->size());
	std::set<std::string> slot_ids;
	for (const Json &entry : *slots) {
		auto slot = ParseSlot(entry, require_r1_fields);
		if (!slot.ok()) {
			return {{}, slot.error};
		}
		if (!slot_ids.insert(slot.value->id).second) {
			return {{}, MakeError(ErrorCode::Duplicate, slot.value->id)};
		}
		mapping.slots.push_back(std::move(*slot.value));
	}
	const auto routes = root.find("routes");
	if (routes == root.end() || !routes->is_array()) {
		return {{}, Schema("routes")};
	}
	if (routes->size() > Limits::kMappingRoutes) {
		return {{}, MakeError(ErrorCode::Limit, "routes")};
	}
	mapping.routes.reserve(routes->size());
	std::set<std::string> route_ids;
	for (const Json &entry : *routes) {
		auto route = ParseRoute(entry, require_r1_fields, !require_r1_fields);
		if (!route.ok()) {
			return {{}, route.error};
		}
		if (!route_ids.insert(route.value->id).second) {
			return {{}, MakeError(ErrorCode::Duplicate, route.value->id)};
		}
		if (route.value->match.slot.has_value() && slot_ids.find(*route.value->match.slot) == slot_ids.end()) {
			return {{}, MakeError(ErrorCode::Reference, *route.value->match.slot)};
		}
		mapping.routes.push_back(std::move(*route.value));
	}
	return {std::move(mapping), {}};
}

Result<Mapping> ParseMapping(std::string_view bytes)
{
	try {
		return ParseMappingInternal(bytes, false);
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Mapping> LoadMapping(const std::string &path)
{
	auto bytes = ReadUtf8File(path);
	if (!bytes.ok()) {
		return {{}, bytes.error};
	}
	try {
		return ParseMappingInternal(*bytes.value, true);
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<Mapping> ValidateMapping(const Mapping &mapping)
{
	try {
		Error error;
		if (!ValidateModel(mapping, error)) {
			return {{}, error};
		}
		return {mapping, {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<CompiledMapping> CompileMapping(const Mapping &mapping)
{
	try {
		auto validated = ValidateMapping(mapping);
		if (!validated.ok()) {
			return {{}, validated.error};
		}
		const Mapping &source = *validated.value;
		for (const Route &route : source.routes) {
			if ((route.target.kind == TargetKind::SourceProperty ||
			     route.target.kind == TargetKind::FilterProperty) &&
			    IsSecretField(route.target.field)) {
				return {{}, MakeError(ErrorCode::Denied, route.id)};
			}
		}
		for (const Route &route : source.routes) {
			if (PatternHasWildcard(route.match.address)) {
				const CapabilityClass capability = ClassOfTarget(route.target);
				if (capability == CapabilityClass::OutputControl ||
				    capability == CapabilityClass::Profile || capability == CapabilityClass::Meta) {
					return {{}, MakeError(ErrorCode::WildcardPrivileged, route.id)};
				}
			}
		}
		for (const Route &route : source.routes) {
			if (PatternFanout(route.match.address) > Limits::kWildcardFanout) {
				return {{}, MakeError(ErrorCode::Fanout, route.id)};
			}
		}

		std::map<std::string, std::vector<std::size_t>> groups;
		for (std::size_t index = 0U; index < source.routes.size(); ++index) {
			if (source.routes[index].enabled) {
				groups[TargetKey(source.routes[index].target)].push_back(index);
			}
		}
		for (const Route &route : source.routes) {
			const auto group = groups.find(TargetKey(route.target));
			if (group == groups.end() || group->second.size() <= 1U || !route.enabled) {
				continue;
			}
			const std::optional<MergeSpec> &first_merge = source.routes[group->second.front()].merge;
			if (!first_merge.has_value()) {
				return {{},
					MakeError(ErrorCode::AmbiguousMerge, source.routes[group->second.front()].id)};
			}
			std::set<std::int64_t> priorities;
			for (const std::size_t member : group->second) {
				const std::optional<MergeSpec> &merge = source.routes[member].merge;
				if (!merge.has_value() || merge->policy != first_merge->policy ||
				    (merge->policy == MergePolicy::Priority &&
				     !priorities.insert(merge->priority).second)) {
					return {{},
						MakeError(ErrorCode::AmbiguousMerge,
							  source.routes[group->second.front()].id)};
				}
			}
		}

		const Error cycle = FindCycleError(source);
		if (!cycle.ok()) {
			return {{}, cycle};
		}

		CompiledMapping result;
		result.slots.reserve(source.slots.size());
		for (const Slot &slot : source.slots) {
			result.slots.push_back(slot.id);
		}
		result.routes.reserve(source.routes.size());
		for (const Route &route : source.routes) {
			CompiledRoute compiled;
			compiled.id = route.id;
			compiled.enabled = route.enabled;
			compiled.capability = ClassOfTarget(route.target);
			compiled.target_key = TargetKey(route.target);
			compiled.merge = route.merge.value_or(MergeSpec{});
			compiled.fanout = PatternFanout(route.match.address);
			result.routes.push_back(std::move(compiled));
		}
		return {std::move(result), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<CompiledMapping> CompileMapping(std::string_view bytes)
{
	auto mapping = ParseMapping(bytes);
	if (!mapping.ok()) {
		return {{}, mapping.error};
	}
	return CompileMapping(*mapping.value);
}

Result<ControlAddress> ParseEventAddress(std::string_view bytes)
{
	auto event = ParseEvent(bytes);
	if (!event.ok()) {
		return {{}, event.error};
	}
	return {std::move(event.value->address), {}};
}

Result<ParsedEvent> ParseEvent(std::string_view bytes)
{
	auto document = ParseJson(bytes);
	if (!document.ok()) {
		return {{}, document.error};
	}
	if (!document.value->is_object()) {
		return {{}, Schema("event")};
	}
	const Json &root = *document.value;
	const auto slot = root.find("slot");
	if (slot == root.end() || (!slot->is_null() && !slot->is_string())) {
		return {{}, Schema("event")};
	}
	const auto address = root.find("address");
	if (address == root.end()) {
		return {{}, Schema("event")};
	}
	auto parsed_address = ParseConcreteAddress(*address, "event", false);
	if (!parsed_address.ok()) {
		if (parsed_address.error.code == ErrorCode::Limit) {
			return {{}, Schema("event")};
		}
		return {{}, parsed_address.error};
	}
	ParsedEvent result;
	if (!slot->is_null()) {
		result.slot = slot->get<std::string>();
	}
	result.address = std::move(*parsed_address.value);
	return {std::move(result), {}};
}

} // namespace Jeiobs::Control
