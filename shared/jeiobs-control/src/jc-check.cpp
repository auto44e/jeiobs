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

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
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

void PrintError(const Error &error)
{
	std::cout << "ERR " << ErrorCodeName(error.code);
	if (!error.detail.empty()) {
		std::cout << ' ' << error.detail;
	}
	std::cout << '\n';
}

bool ReadFile(const std::string &path, std::string &bytes)
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(path);
		std::error_code status_error;
		const std::filesystem::file_status status = std::filesystem::status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return false;
		}
		std::error_code size_error;
		const std::uintmax_t size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error || size > kMappingBytes || size > std::numeric_limits<std::size_t>::max()) {
			return false;
		}
		std::ifstream input;
		input.open(filesystem_path, std::ios::in | std::ios::binary);
		if (!input.is_open()) {
			return false;
		}
		bytes.assign(static_cast<std::size_t>(size), '\0');
		if (!bytes.empty()) {
			input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			return input.gcount() == static_cast<std::streamsize>(bytes.size());
		}
		return true;
	} catch (...) {
		return false;
	}
}

int CheckValidate(const std::string &path)
{
	const auto result = Jeiobs::Control::LoadMapping(path);
	if (!result.ok()) {
		PrintError(result.error);
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
	std::string bytes;
	if (!ReadFile(event_path, bytes)) {
		PrintError({ErrorCode::Schema, "event"});
		return 1;
	}
	const auto event = Jeiobs::Control::ParseEvent(bytes);
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

int CheckValue(const std::string &operation, const std::string &value_bytes)
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

void PrintUsage()
{
	std::cerr << "usage: jc-check validate FILE | compile FILE | match FILE EVENT.json | value OP VALUE_JSON\n";
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
		if ((argc == 4 || argc == 5) && std::string_view(argv[1]) == "value") {
			return CheckValue(argv[2], argv[3]);
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
