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
#include "jc_test.hpp"

#include "jeiobs/control/Mapping.hpp"
#include "jeiobs/control/Value.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace {

const char *kMapping = R"json({
  "format": "jeiobs-control",
  "major": 1,
  "minor": 0,
  "slots": [{"id": "surface", "requires": ["mcu.fader"]}],
  "routes": [{
    "id": "volume",
    "enabled": true,
    "match": {"slot": "surface", "address": [{"s": "midi"}, {"r": [0, 1]}]},
    "target": {"kind": "mixer", "id": "source", "field": "volume"}
  }]
})json";

void TestValues()
{
	for (const unsigned int width : {7U, 14U, 16U, 32U}) {
		for (std::uint64_t code = 0U; code < (std::uint64_t(1U) << std::min(width, 10U)); ++code) {
			const auto value = Jeiobs::Control::Value::MakeUInt(width, {0U, code});
			JC_CHECK(value.ok());
			const auto roundtrip =
				Jeiobs::Control::Value::FromDoubleInCodeSpace(*value.value, value.value->AsDouble());
			JC_CHECK(roundtrip.ok());
			JC_CHECK(*roundtrip.value == *value.value);
		}
	}

	const auto text = Jeiobs::Control::Value::MakeText("hello");
	JC_CHECK(text.ok());
	const auto tuple = Jeiobs::Control::Value::MakeTuple({*text.value});
	JC_CHECK(tuple.ok());

	const auto uint7 = Jeiobs::Control::Value::MakeUInt(7U, {0U, 127U});
	JC_CHECK(uint7.ok());
	const auto uint7_fraction = Jeiobs::Control::Value::FromDoubleInCodeSpace(*uint7.value, 127.5);
	JC_CHECK(uint7_fraction.ok());
	JC_CHECK_EQ(uint7_fraction.value->uintRaw().low, 127U);

	const auto int8 = Jeiobs::Control::Value::MakeInt(8U, {0U, 0U});
	JC_CHECK(int8.ok());
	const auto int8_fraction = Jeiobs::Control::Value::FromDoubleInCodeSpace(*int8.value, 127.5);
	JC_CHECK(int8_fraction.ok());
	JC_CHECK_EQ(int8_fraction.value->intRaw().low, 127U);
	const auto int8_negative_fraction = Jeiobs::Control::Value::FromDoubleInCodeSpace(*int8.value, -128.5);
	JC_CHECK(int8_negative_fraction.ok());
	JC_CHECK_EQ(int8_negative_fraction.value->intRaw().low, 0x80U);

	const auto uint7_relative =
		uint7.value->WithRelative({Jeiobs::Control::RelativeEncoding::TwosComplement, "0.5"});
	JC_CHECK(uint7_relative.ok());
	const auto relative_result = Jeiobs::Control::Value::FromDoubleInCodeSpace(*uint7_relative.value, 127.5);
	JC_CHECK(relative_result.ok());
	JC_CHECK(relative_result.value->relative() == uint7_relative.value->relative());

	const auto uint_min =
		Jeiobs::Control::Value::FromDoubleInCodeSpace(*uint7.value, -std::numeric_limits<double>::infinity());
	JC_CHECK(uint_min.ok());
	JC_CHECK_EQ(uint_min.value->uintRaw().low, 0U);
	const auto int_min =
		Jeiobs::Control::Value::FromDoubleInCodeSpace(*int8.value, -std::numeric_limits<double>::infinity());
	JC_CHECK(int_min.ok());
	JC_CHECK_EQ(int_min.value->intRaw().low, 0x80U);
	const auto nan_result =
		Jeiobs::Control::Value::FromDoubleInCodeSpace(*uint7.value, std::numeric_limits<double>::quiet_NaN());
	JC_CHECK(nan_result.error.code == Jeiobs::Control::ErrorCode::Value);

	const auto invalid_uint = Jeiobs::Control::Value::MakeUInt(7U, {1U, 0U});
	JC_CHECK(invalid_uint.error.code == Jeiobs::Control::ErrorCode::Limit);
	const auto invalid_int = Jeiobs::Control::Value::MakeInt(8U, {0U, 0x80U});
	JC_CHECK(invalid_int.error.code == Jeiobs::Control::ErrorCode::Limit);
	const auto invalid_float =
		Jeiobs::Control::Value::MakeFloatBits(Jeiobs::Control::FloatFormat::Ieee32, 0x1'0000'0000ULL);
	JC_CHECK(invalid_float.error.code == Jeiobs::Control::ErrorCode::Limit);
}

void TestAddresses()
{
	const auto symbol = Jeiobs::Control::AddressSegment::Symbol("midi");
	const auto integer = Jeiobs::Control::AddressSegment::Int(7U);
	const auto pattern = Jeiobs::Control::PatternSegment::Range(0U, 8U);
	JC_CHECK(symbol.ok());
	JC_CHECK(integer.ok());
	JC_CHECK(pattern.ok());
	auto address = Jeiobs::Control::ControlAddress::Create({*symbol.value, *integer.value});
	auto address_pattern = Jeiobs::Control::ControlPattern::Create(
		{Jeiobs::Control::PatternSegment::Symbol("midi").value.value(), *pattern.value});
	JC_CHECK(address.ok());
	JC_CHECK(address_pattern.ok());
	JC_CHECK(Jeiobs::Control::Matches(*address_pattern.value, *address.value));
}

void TestMapping()
{
	const auto mapping = Jeiobs::Control::ParseMapping(kMapping);
	JC_CHECK(mapping.ok());
	const auto compiled = Jeiobs::Control::CompileMapping(*mapping.value);
	JC_CHECK(compiled.ok());
	JC_CHECK_EQ(compiled.value->routes.size(), 1U);
	JC_CHECK_EQ(compiled.value->routes[0].target_key, "mixer|field=volume|id=source");
	JC_CHECK_EQ(compiled.value->routes[0].fanout, 2U);

	const auto duplicate = Jeiobs::Control::ParseMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[],"routes":[]})json");
	JC_CHECK(duplicate.error.code == Jeiobs::Control::ErrorCode::DuplicateKey);

	const auto denied = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"secret","enabled":true,"match":{"slot":null,"address":[{"s":"x"}]},"target":{"kind":"source-property","id":"x","field":"Token"}}
	]})json");
	JC_CHECK(denied.error.code == Jeiobs::Control::ErrorCode::Denied);
	JC_CHECK_EQ(denied.error.detail, "secret");

	const auto missing_enabled = Jeiobs::Control::ParseMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"r1","match":{"slot":null,"address":[{"s":"x"}]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(missing_enabled.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto compile_missing_enabled = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"r1","match":{"slot":null,"address":[{"s":"x"}]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(compile_missing_enabled.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto missing_requires = Jeiobs::Control::ParseMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[{"id":"surface"}],"routes":[]})json");
	JC_CHECK(missing_requires.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto empty_address = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"r1","enabled":true,"match":{"slot":null,"address":[]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(empty_address.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto non_ascii_requirement = Jeiobs::Control::ParseMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[{"id":"surface","requires":["caf\u00e9"]}],"routes":[]})json");
	JC_CHECK(non_ascii_requirement.error.code == Jeiobs::Control::ErrorCode::Schema);

	for (const char *field : {"URL", "input", "ffmpeg_options", "playlist"}) {
		const std::string document =
			std::string(R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"secret","enabled":true,"match":{"slot":null,"address":[{"s":"x"}]},"target":{"kind":"source-property","id":"x","field":")json") +
			field + R"json("}}]})json";
		const auto result = Jeiobs::Control::CompileMapping(document);
		JC_CHECK(result.error.code == Jeiobs::Control::ErrorCode::Denied);
	}
}

void TestCompileChecks()
{
	const auto fanout = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"wide","enabled":true,"match":{"slot":null,"address":[{"r":[0,64]}]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(fanout.error.code == Jeiobs::Control::ErrorCode::Fanout);

	const auto privileged = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"out","enabled":true,"match":{"slot":null,"address":[{"any":true}]},"target":{"kind":"output","output":"stream","action":"start"}}
	]})json");
	JC_CHECK(privileged.error.code == Jeiobs::Control::ErrorCode::WildcardPrivileged);

	const auto ambiguous = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"first","enabled":true,"match":{"slot":null,"address":[{"s":"a"}]},"target":{"kind":"mixer","id":"x","field":"volume"}},
		{"id":"second","enabled":true,"match":{"slot":null,"address":[{"s":"b"}]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(ambiguous.error.code == Jeiobs::Control::ErrorCode::AmbiguousMerge);
	JC_CHECK_EQ(ambiguous.error.detail, "first");

	const auto cycle = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"a","enabled":true,"match":{"slot":null,"address":[{"s":"in"}]},"target":{"kind":"emit","address":[{"s":"out"}]}},
		{"id":"b","enabled":true,"match":{"slot":null,"address":[{"s":"out"}]},"target":{"kind":"emit","address":[{"s":"in"}]}}
	]})json");
	JC_CHECK(cycle.error.code == Jeiobs::Control::ErrorCode::Cycle);
	JC_CHECK_EQ(cycle.error.detail, "a");

	const auto hotkey_wildcard = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"hotkey","enabled":true,"match":{"slot":null,"address":[{"any":true}]},"target":{"kind":"hotkey","registerer_kind":"frontend","registerer_id":"obs","name":"x.y","effect":"scene"}}
	]})json");
	JC_CHECK(hotkey_wildcard.error.code == Jeiobs::Control::ErrorCode::WildcardPrivileged);

	const auto event_duplicate =
		Jeiobs::Control::ParseEvent(R"json({"slot":null,"address":[{"s":"x"}],"address":[{"s":"y"}]})json");
	JC_CHECK(event_duplicate.error.code == Jeiobs::Control::ErrorCode::DuplicateKey);
	const auto event_empty = Jeiobs::Control::ParseEvent(R"json({"slot":null,"address":[]})json");
	JC_CHECK(event_empty.error.code == Jeiobs::Control::ErrorCode::Schema);

	auto first_address =
		Jeiobs::Control::ControlAddress::Create({*Jeiobs::Control::AddressSegment::Text("a/i:1").value});
	auto second_address = Jeiobs::Control::ControlAddress::Create(
		{*Jeiobs::Control::AddressSegment::Text("a").value, *Jeiobs::Control::AddressSegment::Int(1U).value});
	JC_CHECK(first_address.ok());
	JC_CHECK(second_address.ok());
	Jeiobs::Control::Target first_target;
	first_target.kind = Jeiobs::Control::TargetKind::Emit;
	first_target.emit_address = *first_address.value;
	Jeiobs::Control::Target second_target;
	second_target.kind = Jeiobs::Control::TargetKind::Emit;
	second_target.emit_address = *second_address.value;
	JC_CHECK(Jeiobs::Control::TargetKey(first_target) != Jeiobs::Control::TargetKey(second_target));
}

} // namespace

int main()
{
	try {
		TestValues();
		TestAddresses();
		TestMapping();
		TestCompileChecks();
	} catch (...) {
		++JcTest::failures;
	}
	return JcTest::failures == 0 ? 0 : 1;
}
