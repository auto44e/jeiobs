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
#include <cstdint>
#include <string>

namespace {

const char *kMapping = R"json({
  "format": "jeiobs-control",
  "major": 1,
  "minor": 0,
  "slots": [{"id": "surface", "requires": ["mcu.fader"]}],
  "routes": [{
    "id": "volume",
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
		{"id":"secret","match":{"slot":null,"address":[]},"target":{"kind":"source-property","id":"x","field":"Token"}}
	]})json");
	JC_CHECK(denied.error.code == Jeiobs::Control::ErrorCode::Denied);
	JC_CHECK_EQ(denied.error.detail, "secret");
}

void TestCompileChecks()
{
	const auto fanout = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"wide","match":{"slot":null,"address":[{"r":[0,64]}]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(fanout.error.code == Jeiobs::Control::ErrorCode::Fanout);

	const auto privileged = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"out","match":{"slot":null,"address":[{"any":true}]},"target":{"kind":"output","output":"stream","action":"start"}}
	]})json");
	JC_CHECK(privileged.error.code == Jeiobs::Control::ErrorCode::WildcardPrivileged);

	const auto ambiguous = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"first","match":{"slot":null,"address":[]},"target":{"kind":"mixer","id":"x","field":"volume"}},
		{"id":"second","match":{"slot":null,"address":[]},"target":{"kind":"mixer","id":"x","field":"volume"}}
	]})json");
	JC_CHECK(ambiguous.error.code == Jeiobs::Control::ErrorCode::AmbiguousMerge);
	JC_CHECK_EQ(ambiguous.error.detail, "first");

	const auto cycle = Jeiobs::Control::CompileMapping(
		R"json({"format":"jeiobs-control","major":1,"minor":0,"slots":[],"routes":[
		{"id":"a","match":{"slot":null,"address":[{"s":"in"}]},"target":{"kind":"emit","address":[{"s":"out"}]}},
		{"id":"b","match":{"slot":null,"address":[{"s":"out"}]},"target":{"kind":"emit","address":[{"s":"in"}]}}
	]})json");
	JC_CHECK(cycle.error.code == Jeiobs::Control::ErrorCode::Cycle);
	JC_CHECK_EQ(cycle.error.detail, "a");
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
