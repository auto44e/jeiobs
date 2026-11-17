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
#include "jeiobs/control/Transform.hpp"
#include "jeiobs/control/Value.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

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
	JC_CHECK_EQ(int8_negative_fraction.value->intRaw().high, std::numeric_limits<std::uint64_t>::max());
	JC_CHECK_EQ(int8_negative_fraction.value->intRaw().low,
		    std::numeric_limits<std::uint64_t>::max() - std::uint64_t(127U));
	const auto int8_roundtrip = Jeiobs::Control::Value::MakeInt(int8_negative_fraction.value->widthBits(),
								    int8_negative_fraction.value->intRaw());
	JC_CHECK(int8_roundtrip.ok());
	JC_CHECK(*int8_roundtrip.value == *int8_negative_fraction.value);

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
	JC_CHECK_EQ(int_min.value->intRaw().high, std::numeric_limits<std::uint64_t>::max());
	JC_CHECK_EQ(int_min.value->intRaw().low, std::numeric_limits<std::uint64_t>::max() - std::uint64_t(127U));
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

Jeiobs::Control::TransformSpec MakeStage(std::string operation)
{
	Jeiobs::Control::TransformSpec stage;
	stage.op = std::move(operation);
	return stage;
}

Jeiobs::Control::Result<Jeiobs::Control::TransformPipeline>
CompileStages(std::vector<Jeiobs::Control::TransformSpec> stages)
{
	return Jeiobs::Control::TransformPipeline::Compile(stages);
}

Jeiobs::Control::Result<Jeiobs::Control::Value> FloatValue(double number)
{
	std::uint64_t bits = 0U;
	std::memcpy(&bits, &number, sizeof(bits));
	return Jeiobs::Control::Value::MakeFloatBits(Jeiobs::Control::FloatFormat::Ieee64, bits);
}

double AppliedNumber(const Jeiobs::Control::TransformPipeline &pipeline, const Jeiobs::Control::Value &input,
		     Jeiobs::Control::TransformState &state, bool &has_output)
{
	const auto applied = pipeline.Apply(input, state);
	JC_CHECK(applied.ok());
	if (!applied.ok() || applied.value->outcome != Jeiobs::Control::TransformPipeline::Outcome::Output ||
	    !applied.value->value.has_value()) {
		has_output = false;
		return 0.0;
	}
	has_output = true;
	JC_CHECK(std::isfinite(applied.value->value->AsDouble()));
	return applied.value->value->AsDouble();
}

struct Xorshift64Star {
	// Fixed property-test seed: 0x9e3779b97f4a7c15.
	std::uint64_t state = 0x9e3779b97f4a7c15ULL;

	std::uint64_t next() noexcept
	{
		state ^= state >> 12U;
		state ^= state << 25U;
		state ^= state >> 27U;
		return state * 0x2545f4914f6cdd1dULL;
	}

	double unit() noexcept { return static_cast<double>(next() >> 11U) * 0x1.0p-53; }
};

void TestTransformStages()
{
	const auto text = Jeiobs::Control::Value::MakeText("unchanged", "unit");
	const auto empty = CompileStages({});
	JC_CHECK(empty.ok());
	if (!empty.ok() || !text.ok()) {
		return;
	}
	Jeiobs::Control::TransformState empty_state;
	const auto empty_result = empty.value->Apply(*text.value, empty_state);
	JC_CHECK(empty_result.ok());
	JC_CHECK(*empty_result.value->value == *text.value);

	auto scale = MakeStage("scale");
	scale.numbers = {0.0, 1.0, -1.0, 1.0};
	auto scale_pipeline = CompileStages({scale});
	JC_CHECK(scale_pipeline.ok());
	auto half = FloatValue(0.5);
	JC_CHECK(half.ok());
	if (!scale_pipeline.ok() || !half.ok()) {
		return;
	}
	Jeiobs::Control::TransformState state;
	bool has_output = false;
	JC_CHECK_EQ(AppliedNumber(*scale_pipeline.value, *half.value, state, has_output), 0.0);
	JC_CHECK(has_output);

	auto curve_log = MakeStage("curve");
	curve_log.text = "log";
	auto curve_exp = MakeStage("curve");
	curve_exp.text = "exp";
	auto curves = CompileStages({curve_log, curve_exp});
	JC_CHECK(curves.ok());
	auto clamp = MakeStage("clamp");
	clamp.numbers = {-1.0, 1.0};
	auto deadzone = MakeStage("deadzone");
	deadzone.numbers = {0.2};
	auto quantize = MakeStage("quantize");
	quantize.integer = 5;
	auto threshold = MakeStage("threshold");
	threshold.numbers = {0.5};
	auto momentary = MakeStage("momentary");
	auto numeric_pipeline = CompileStages({clamp, deadzone, quantize});
	JC_CHECK(numeric_pipeline.ok());
	auto threshold_pipeline = CompileStages({threshold});
	auto momentary_pipeline = CompileStages({momentary});
	JC_CHECK(threshold_pipeline.ok());
	JC_CHECK(momentary_pipeline.ok());

	auto trigger = Jeiobs::Control::Value::MakeTrigger();
	JC_CHECK(trigger.ok());
	if (trigger.ok() && momentary_pipeline.ok()) {
		Jeiobs::Control::TransformState trigger_state;
		const auto result = momentary_pipeline.value->Apply(*trigger.value, trigger_state);
		JC_CHECK(result.ok());
		JC_CHECK(result.value->value->boolValue());
	}

	auto join = MakeStage("join14");
	auto split = MakeStage("split14");
	auto join_pipeline = CompileStages({join});
	auto split_pipeline = CompileStages({split});
	JC_CHECK(join_pipeline.ok());
	JC_CHECK(split_pipeline.ok());
	const auto msb = Jeiobs::Control::Value::MakeUInt(7U, {0U, 0x12U});
	const auto lsb = Jeiobs::Control::Value::MakeUInt(7U, {0U, 0x34U});
	JC_CHECK(msb.ok());
	JC_CHECK(lsb.ok());
	if (join_pipeline.ok() && split_pipeline.ok() && msb.ok() && lsb.ok()) {
		const auto tuple = Jeiobs::Control::Value::MakeTuple({*msb.value, *lsb.value});
		JC_CHECK(tuple.ok());
		Jeiobs::Control::TransformState join_state;
		const auto joined = join_pipeline.value->Apply(*tuple.value, join_state);
		JC_CHECK(joined.ok());
		JC_CHECK_EQ(joined.value->value->uintRaw().low, 0x934U);
		Jeiobs::Control::TransformState split_state;
		const auto split_result = split_pipeline.value->Apply(*joined.value->value, split_state);
		JC_CHECK(split_result.ok());
		JC_CHECK_EQ(split_result.value->value->tuple()[0].uintRaw().low, 0x12U);
		JC_CHECK_EQ(split_result.value->value->tuple()[1].uintRaw().low, 0x34U);
	}

	auto toggle = MakeStage("toggle");
	auto toggle_pipeline = CompileStages({toggle});
	JC_CHECK(toggle_pipeline.ok());
	const auto press = Jeiobs::Control::Value::MakeBool(true);
	const auto release = Jeiobs::Control::Value::MakeBool(false);
	if (toggle_pipeline.ok() && press.ok() && release.ok()) {
		Jeiobs::Control::TransformState toggle_state;
		const auto first_press = toggle_pipeline.value->Apply(*press.value, toggle_state);
		JC_CHECK(first_press.ok());
		JC_CHECK(first_press.value->value->boolValue());
		const auto no_output = toggle_pipeline.value->Apply(*release.value, toggle_state);
		JC_CHECK(no_output.ok());
		JC_CHECK(no_output.value->outcome == Jeiobs::Control::TransformPipeline::Outcome::NoOutput);
		JC_CHECK(toggle_state.toggled);
	}

	auto relative = MakeStage("relative");
	relative.text = "twos_complement";
	relative.numbers = {0.01};
	auto relative_pipeline = CompileStages({relative});
	JC_CHECK(relative_pipeline.ok());
	if (relative_pipeline.ok()) {
		const auto relative_input = Jeiobs::Control::Value::MakeUInt(7U, {0U, 5U});
		JC_CHECK(relative_input.ok());
		Jeiobs::Control::TransformState relative_state;
		relative_state.position = 0.5;
		const auto relative_result = relative_pipeline.value->Apply(*relative_input.value, relative_state);
		JC_CHECK(relative_result.ok());
		JC_CHECK(std::abs(relative_result.value->value->AsDouble() - 0.55) < 1e-12);
	}

	const auto valid_join_position = CompileStages({MakeStage("join14"), MakeStage("invert")});
	JC_CHECK(valid_join_position.ok());
	const auto invalid_join_position = CompileStages({MakeStage("invert"), MakeStage("join14")});
	JC_CHECK(!invalid_join_position.ok());
	JC_CHECK(invalid_join_position.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto invalid_relative_position = CompileStages({MakeStage("invert"), MakeStage("relative")});
	JC_CHECK(!invalid_relative_position.ok());
	JC_CHECK(invalid_relative_position.error.code == Jeiobs::Control::ErrorCode::Schema);
	const auto invalid_split = CompileStages({MakeStage("split14"), MakeStage("invert")});
	JC_CHECK(!invalid_split.ok());
	JC_CHECK(invalid_split.error.code == Jeiobs::Control::ErrorCode::Schema);
	// An unknown operation is reported before a position error (contract r1).
	const auto unknown_after_bad_position =
		CompileStages({MakeStage("invert"), MakeStage("join14"), MakeStage("nope")});
	JC_CHECK(!unknown_after_bad_position.ok());
	JC_CHECK_EQ(unknown_after_bad_position.error.detail, "op");
	auto invalid_parameters = MakeStage("invert");
	invalid_parameters.numbers = {1.0};
	const auto invalid_parameter_pipeline = CompileStages({invalid_parameters});
	JC_CHECK(!invalid_parameter_pipeline.ok());
	JC_CHECK(invalid_parameter_pipeline.error.code == Jeiobs::Control::ErrorCode::Schema);
}

void TestTransformExtraCoverage()
{
	using Jeiobs::Control::TransformState;
	using Jeiobs::Control::ValueKind;

	// deadzone: inside the zone is 0, outside rescales to keep the sign.
	auto deadzone = MakeStage("deadzone");
	deadzone.numbers = {0.25};
	const auto deadzone_pipeline = CompileStages({deadzone});
	JC_CHECK(deadzone_pipeline.ok());
	// threshold: x >= at.
	auto threshold = MakeStage("threshold");
	threshold.numbers = {0.5};
	const auto threshold_pipeline = CompileStages({threshold});
	JC_CHECK(threshold_pipeline.ok());
	if (!deadzone_pipeline.ok() || !threshold_pipeline.ok()) {
		return;
	}
	struct Case {
		double input;
		double expected;
	};
	for (const Case &item : {Case{0.0, 0.0}, Case{0.25, 0.0}, Case{-0.25, 0.0}, Case{0.5, 1.0 / 3.0},
				 Case{-0.5, -1.0 / 3.0}, Case{1.0, 1.0}, Case{-1.0, -1.0}}) {
		const auto input = FloatValue(item.input);
		JC_CHECK(input.ok());
		TransformState state;
		bool has_output = false;
		const double actual = AppliedNumber(*deadzone_pipeline.value, *input.value, state, has_output);
		JC_CHECK(has_output);
		JC_CHECK(std::abs(actual - item.expected) < 1e-15);
	}
	for (const Case &item : {Case{0.49, 0.0}, Case{0.5, 1.0}, Case{0.51, 1.0}, Case{-3.0, 0.0}}) {
		const auto input = FloatValue(item.input);
		JC_CHECK(input.ok());
		TransformState state;
		const auto applied = threshold_pipeline.value->Apply(*input.value, state);
		JC_CHECK(applied.ok());
		JC_CHECK(applied.value->value->kind() == ValueKind::Bool);
		JC_CHECK_EQ(applied.value->value->boolValue(), item.expected != 0.0);
	}

	// Negative zero never leaves a stage: clamp to [-1, -0.0] gives +0.0.
	auto clamp = MakeStage("clamp");
	clamp.numbers = {-1.0, -0.0};
	const auto clamp_pipeline = CompileStages({clamp});
	JC_CHECK(clamp_pipeline.ok());
	const auto five = FloatValue(5.0);
	const auto negative_zero = FloatValue(-0.0);
	JC_CHECK(five.ok());
	JC_CHECK(negative_zero.ok());
	if (clamp_pipeline.ok() && five.ok() && negative_zero.ok()) {
		for (const auto *input : {&five, &negative_zero}) {
			TransformState state;
			const auto applied = clamp_pipeline.value->Apply(*input->value, state);
			JC_CHECK(applied.ok());
			JC_CHECK(applied.ok() && !std::signbit(applied.value->value->AsDouble()));
			JC_CHECK(applied.ok() && applied.value->value->AsDouble() == 0.0);
		}
	}

	// Properties with the same fixed-seed generator family (different seed): quantize over many step
	// counts, relative position bounds, and error calls leaving the state alone.
	Xorshift64Star random;
	random.state = 0x0123456789abcdefULL;
	for (int round = 0; round < 2000; ++round) {
		const std::int64_t steps = 2 + static_cast<std::int64_t>(random.next() % 63U);
		auto quantize = MakeStage("quantize");
		quantize.integer = steps;
		const auto pipeline = CompileStages({quantize});
		JC_CHECK(pipeline.ok());
		if (!pipeline.ok()) {
			continue;
		}
		const double x = -0.5 + 2.0 * random.unit();
		const auto input = FloatValue(x);
		JC_CHECK(input.ok());
		TransformState state;
		bool has_output = false;
		const double y = AppliedNumber(*pipeline.value, *input.value, state, has_output);
		JC_CHECK(has_output);
		const double n = static_cast<double>(steps - 1);
		const double level = std::nearbyint(y * n);
		JC_CHECK(std::abs(y * n - level) < 1e-9); // an exact level k / n
		JC_CHECK(level >= 0.0 && level <= n);
		const auto again = FloatValue(y);
		JC_CHECK(again.ok());
		TransformState second_state;
		JC_CHECK_EQ(AppliedNumber(*pipeline.value, *again.value, second_state, has_output), y); // idempotent
	}

	const char *encodings[] = {"twos_complement", "sign_magnitude", "binary_offset"};
	for (const char *encoding : encodings) {
		auto relative = MakeStage("relative");
		relative.text = encoding;
		relative.numbers = {0.03};
		const auto pipeline = CompileStages({relative});
		JC_CHECK(pipeline.ok());
		if (!pipeline.ok()) {
			continue;
		}
		TransformState state;
		for (int round = 0; round < 2000; ++round) {
			const unsigned int width = round % 2 == 0 ? 7U : 14U;
			const std::uint64_t code = random.next() & ((std::uint64_t(1U) << width) - 1U);
			const auto input = Jeiobs::Control::Value::MakeUInt(width, {0U, code});
			JC_CHECK(input.ok());
			if (!input.ok()) {
				continue;
			}
			bool has_output = false;
			const double position = AppliedNumber(*pipeline.value, *input.value, state, has_output);
			JC_CHECK(has_output);
			JC_CHECK(position >= 0.0 && position <= 1.0);
			JC_CHECK_EQ(position, state.position);
			// A call that errors (wrong kind) leaves the state exactly as it was.
			const auto bad = Jeiobs::Control::Value::MakeText("x");
			JC_CHECK(bad.ok());
			const TransformState before = state;
			const auto failed = pipeline.value->Apply(*bad.value, state);
			JC_CHECK(!failed.ok());
			JC_CHECK_EQ(state.position, before.position);
			JC_CHECK_EQ(state.toggled, before.toggled);
		}
	}
}

void TestTransformRollback()
{
	auto toggle = MakeStage("toggle");
	auto failing_scale = MakeStage("scale");
	failing_scale.numbers = {0.0, 2.0, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
	auto pipeline = CompileStages({toggle, failing_scale});
	JC_CHECK(pipeline.ok());
	const auto input = FloatValue(0.5);
	JC_CHECK(input.ok());
	if (!pipeline.ok() || !input.ok()) {
		return;
	}
	Jeiobs::Control::TransformState state;
	state.toggled = false;
	const auto result = pipeline.value->Apply(*input.value, state);
	JC_CHECK(!result.ok());
	JC_CHECK(result.error.code == Jeiobs::Control::ErrorCode::Value);
	JC_CHECK(!state.toggled);

	auto relative = MakeStage("relative");
	relative.text = "twos_complement";
	relative.numbers = {0.01};
	auto relative_pipeline = CompileStages({relative});
	JC_CHECK(relative_pipeline.ok());
	if (relative_pipeline.ok()) {
		const auto too_wide = Jeiobs::Control::Value::MakeUInt(65U, {0U, 1U});
		JC_CHECK(too_wide.ok());
		Jeiobs::Control::TransformState relative_state;
		relative_state.position = 0.25;
		const auto failed = relative_pipeline.value->Apply(*too_wide.value, relative_state);
		JC_CHECK(!failed.ok());
		JC_CHECK_EQ(relative_state.position, 0.25);
	}
}

void TestTransformProperties()
{
	Xorshift64Star random;
	for (int index = 0; index < 2000; ++index) {
		const double input_min = -10.0 + 20.0 * random.unit();
		const double input_max = input_min + 0.01 + 20.0 * random.unit();
		const double output_min = -10.0 + 20.0 * random.unit();
		const double output_max = output_min + 0.01 + 20.0 * random.unit();
		auto scale = MakeStage("scale");
		scale.numbers = {input_min, input_max, output_min, output_max};
		const auto pipeline = CompileStages({scale});
		JC_CHECK(pipeline.ok());
		if (!pipeline.ok()) {
			continue;
		}
		const auto first = FloatValue(input_min);
		const auto second = FloatValue(input_max);
		JC_CHECK(first.ok());
		JC_CHECK(second.ok());
		Jeiobs::Control::TransformState first_state;
		Jeiobs::Control::TransformState second_state;
		bool first_output = false;
		bool second_output = false;
		const double first_value = AppliedNumber(*pipeline.value, *first.value, first_state, first_output);
		const double second_value = AppliedNumber(*pipeline.value, *second.value, second_state, second_output);
		JC_CHECK(first_output);
		JC_CHECK(second_output);
		JC_CHECK_EQ(first_value, output_min);
		// The literal formula maps b to d to within rounding of the span (contract r1).
		JC_CHECK(std::abs(second_value - output_max) <=
			 4.0 * std::numeric_limits<double>::epsilon() * (std::abs(output_min) + std::abs(output_max)));
		const double x1 = input_min + (input_max - input_min) * random.unit();
		const double x2 = input_min + (input_max - input_min) * random.unit();
		const double low = std::min(x1, x2);
		const double high = std::max(x1, x2);
		const auto low_value = FloatValue(low);
		const auto high_value = FloatValue(high);
		Jeiobs::Control::TransformState low_state;
		Jeiobs::Control::TransformState high_state;
		bool low_output = false;
		bool high_output = false;
		const double low_result = AppliedNumber(*pipeline.value, *low_value.value, low_state, low_output);
		const double high_result = AppliedNumber(*pipeline.value, *high_value.value, high_state, high_output);
		JC_CHECK(low_result <= high_result);
	}

	auto log_stage = MakeStage("curve");
	log_stage.text = "log";
	auto exp_stage = MakeStage("curve");
	exp_stage.text = "exp";
	auto log_pipeline = CompileStages({log_stage});
	auto exp_pipeline = CompileStages({exp_stage});
	auto roundtrip_pipeline = CompileStages({log_stage, exp_stage});
	JC_CHECK(log_pipeline.ok());
	JC_CHECK(exp_pipeline.ok());
	JC_CHECK(roundtrip_pipeline.ok());
	double previous_log = 0.0;
	double previous_exp = 0.0;
	for (int index = 0; index < 2000; ++index) {
		const double x = static_cast<double>(index) / 1999.0;
		const auto input = FloatValue(x);
		Jeiobs::Control::TransformState log_state;
		Jeiobs::Control::TransformState exp_state;
		Jeiobs::Control::TransformState roundtrip_state;
		bool log_output = false;
		bool exp_output = false;
		const double log_value = AppliedNumber(*log_pipeline.value, *input.value, log_state, log_output);
		const double exp_value = AppliedNumber(*exp_pipeline.value, *input.value, exp_state, exp_output);
		const double roundtrip =
			AppliedNumber(*roundtrip_pipeline.value, *input.value, roundtrip_state, log_output);
		JC_CHECK(log_value >= previous_log);
		JC_CHECK(exp_value >= previous_exp);
		JC_CHECK(std::abs(roundtrip - x) <= 1e-12);
		previous_log = log_value;
		previous_exp = exp_value;
	}
	{
		const auto zero = FloatValue(0.0);
		const auto one = FloatValue(1.0);
		Jeiobs::Control::TransformState zero_state;
		Jeiobs::Control::TransformState one_state;
		bool zero_output = false;
		bool one_output = false;
		JC_CHECK_EQ(AppliedNumber(*log_pipeline.value, *zero.value, zero_state, zero_output), 0.0);
		JC_CHECK_EQ(AppliedNumber(*log_pipeline.value, *one.value, one_state, one_output), 1.0);
	}

	auto clamp = MakeStage("clamp");
	clamp.numbers = {-0.25, 0.75};
	auto clamp_pipeline = CompileStages({clamp});
	JC_CHECK(clamp_pipeline.ok());
	for (int index = 0; index < 2000; ++index) {
		const double input = -2.0 + 4.0 * random.unit();
		const auto value = FloatValue(input);
		Jeiobs::Control::TransformState state;
		bool has_output = false;
		const double result = AppliedNumber(*clamp_pipeline.value, *value.value, state, has_output);
		const auto repeated = FloatValue(result);
		Jeiobs::Control::TransformState repeated_state;
		bool repeated_output = false;
		const double repeated_result =
			AppliedNumber(*clamp_pipeline.value, *repeated.value, repeated_state, repeated_output);
		JC_CHECK(result >= -0.25 && result <= 0.75);
		JC_CHECK_EQ(result, repeated_result);
	}

	auto invert = MakeStage("invert");
	auto invert_pipeline = CompileStages({invert, invert});
	JC_CHECK(invert_pipeline.ok());
	for (int index = 0; index < 2000; ++index) {
		const double input = static_cast<double>(index) / 1999.0;
		const auto value = FloatValue(input);
		Jeiobs::Control::TransformState state;
		bool has_output = false;
		const double result = AppliedNumber(*invert_pipeline.value, *value.value, state, has_output);
		JC_CHECK(std::abs(result - input) <= std::numeric_limits<double>::epsilon());
	}

	auto quantize = MakeStage("quantize");
	quantize.integer = 17;
	auto quantize_pipeline = CompileStages({quantize});
	JC_CHECK(quantize_pipeline.ok());
	double previous_quantized = 0.0;
	for (int index = 0; index < 2000; ++index) {
		const double input = static_cast<double>(index) / 1999.0;
		const auto value = FloatValue(input);
		Jeiobs::Control::TransformState state;
		bool has_output = false;
		const double result = AppliedNumber(*quantize_pipeline.value, *value.value, state, has_output);
		const double level = result * 16.0;
		JC_CHECK(std::abs(level - std::nearbyint(level)) <= 0.0);
		JC_CHECK(result >= previous_quantized);
		previous_quantized = result;
		const auto result_value = FloatValue(result);
		Jeiobs::Control::TransformState repeated_state;
		bool repeated_output = false;
		const double repeated =
			AppliedNumber(*quantize_pipeline.value, *result_value.value, repeated_state, repeated_output);
		JC_CHECK_EQ(result, repeated);
	}

	for (const unsigned int width : {7U, 14U}) {
		const std::uint64_t half = std::uint64_t(1U) << (width - 1U);
		const int limit = width == 7U ? 30 : 1000;
		auto twos = MakeStage("relative");
		twos.text = "twos_complement";
		twos.numbers = {0.0001};
		auto sign_magnitude = MakeStage("relative");
		sign_magnitude.text = "sign_magnitude";
		sign_magnitude.numbers = {0.0001};
		auto binary_offset = MakeStage("relative");
		binary_offset.text = "binary_offset";
		binary_offset.numbers = {0.0001};
		const auto twos_pipeline = CompileStages({twos});
		const auto sign_magnitude_pipeline = CompileStages({sign_magnitude});
		const auto binary_offset_pipeline = CompileStages({binary_offset});
		JC_CHECK(twos_pipeline.ok());
		JC_CHECK(sign_magnitude_pipeline.ok());
		JC_CHECK(binary_offset_pipeline.ok());
		for (int index = 0; index < 2000; ++index) {
			const int delta =
				static_cast<int>(random.next() % static_cast<std::uint64_t>(2 * limit + 1)) - limit;
			const std::uint64_t twos_code = delta >= 0 ? static_cast<std::uint64_t>(delta)
								   : half * 2U + delta;
			const std::uint64_t sign_magnitude_code = delta >= 0
									  ? static_cast<std::uint64_t>(delta)
									  : half | static_cast<std::uint64_t>(-delta);
			const std::uint64_t binary_offset_code = half + delta;
			const auto twos_input = Jeiobs::Control::Value::MakeUInt(width, {0U, twos_code});
			const auto sign_magnitude_input =
				Jeiobs::Control::Value::MakeUInt(width, {0U, sign_magnitude_code});
			const auto binary_offset_input =
				Jeiobs::Control::Value::MakeUInt(width, {0U, binary_offset_code});
			Jeiobs::Control::TransformState twos_state;
			Jeiobs::Control::TransformState sign_magnitude_state;
			Jeiobs::Control::TransformState binary_offset_state;
			twos_state.position = 0.5;
			sign_magnitude_state.position = 0.5;
			binary_offset_state.position = 0.5;
			const auto twos_result = twos_pipeline.value->Apply(*twos_input.value, twos_state);
			const auto sign_magnitude_result =
				sign_magnitude_pipeline.value->Apply(*sign_magnitude_input.value, sign_magnitude_state);
			const auto binary_offset_result =
				binary_offset_pipeline.value->Apply(*binary_offset_input.value, binary_offset_state);
			JC_CHECK(twos_result.ok());
			JC_CHECK(sign_magnitude_result.ok());
			JC_CHECK(binary_offset_result.ok());
			JC_CHECK(std::abs(twos_result.value->value->AsDouble() -
					  sign_magnitude_result.value->value->AsDouble()) <= 1e-15);
			JC_CHECK(std::abs(twos_result.value->value->AsDouble() -
					  binary_offset_result.value->value->AsDouble()) <= 1e-15);

			const int inverse_delta = -delta;
			const std::uint64_t inverse_twos = inverse_delta >= 0
								   ? static_cast<std::uint64_t>(inverse_delta)
								   : half * 2U + inverse_delta;
			const auto inverse_input = Jeiobs::Control::Value::MakeUInt(width, {0U, inverse_twos});
			Jeiobs::Control::TransformState pair_state;
			pair_state.position = 0.5;
			const auto pair_first = twos_pipeline.value->Apply(*twos_input.value, pair_state);
			const auto pair_second = twos_pipeline.value->Apply(*inverse_input.value, pair_state);
			JC_CHECK(pair_first.ok());
			JC_CHECK(pair_second.ok());
			JC_CHECK(std::abs(pair_second.value->value->AsDouble() - 0.5) <= 1e-15);
		}
		/*
		 * The three encodings have a zero code too; this also exercises the
		 * binary-offset midpoint and sign-magnitude negative zero.
		 */
		for (const char *encoding : {"twos_complement", "sign_magnitude", "binary_offset"}) {
			auto relative = MakeStage("relative");
			relative.text = encoding;
			relative.numbers = {0.0001};
			const auto pipeline = CompileStages({relative});
			JC_CHECK(pipeline.ok());
			const auto zero = Jeiobs::Control::Value::MakeUInt(
				width, {0U, std::string(encoding) == "binary_offset" ? half : 0U});
			Jeiobs::Control::TransformState state;
			state.position = 0.5;
			const auto result = pipeline.value->Apply(*zero.value, state);
			JC_CHECK(result.ok());
			JC_CHECK_EQ(result.value->value->AsDouble(), 0.5);
		}
	}

	auto split = MakeStage("split14");
	auto join = MakeStage("join14");
	auto split_pipeline = CompileStages({split});
	auto join_pipeline = CompileStages({join});
	JC_CHECK(split_pipeline.ok());
	JC_CHECK(join_pipeline.ok());
	for (std::uint64_t code = 0U; code < 16384U; ++code) {
		const auto input = Jeiobs::Control::Value::MakeUInt(14U, {0U, code});
		Jeiobs::Control::TransformState split_state;
		const auto split_result = split_pipeline.value->Apply(*input.value, split_state);
		Jeiobs::Control::TransformState join_state;
		const auto join_result = join_pipeline.value->Apply(*split_result.value->value, join_state);
		JC_CHECK(join_result.ok());
		JC_CHECK_EQ(join_result.value->value->uintRaw().low, code);
	}

	auto toggle = MakeStage("toggle");
	auto toggle_pipeline = CompileStages({toggle});
	JC_CHECK(toggle_pipeline.ok());
	if (toggle_pipeline.ok()) {
		Jeiobs::Control::TransformState state;
		for (int count = 0; count < 2000; ++count) {
			const auto input = Jeiobs::Control::Value::MakeBool(true);
			const auto result = toggle_pipeline.value->Apply(*input.value, state);
			JC_CHECK(result.ok());
			JC_CHECK_EQ(state.toggled, count % 2 == 0);
			const auto release = Jeiobs::Control::Value::MakeBool(false);
			const auto no_output = toggle_pipeline.value->Apply(*release.value, state);
			JC_CHECK(no_output.ok());
			JC_CHECK(no_output.value->outcome == Jeiobs::Control::TransformPipeline::Outcome::NoOutput);
			JC_CHECK_EQ(state.toggled, count % 2 == 0);
		}
	}

	auto safe_scale = MakeStage("scale");
	safe_scale.numbers = {0.0, 1.0, 0.0, 1.0};
	auto safe_pipeline = CompileStages({safe_scale});
	JC_CHECK(safe_pipeline.ok());
	for (int index = 0; index < 2000; ++index) {
		const std::uint64_t selector = random.next() % 4U;
		double number = 0.0;
		if (selector == 0U) {
			number = std::numeric_limits<double>::quiet_NaN();
		} else if (selector == 1U) {
			number = std::numeric_limits<double>::infinity();
		} else if (selector == 2U) {
			number = -std::numeric_limits<double>::infinity();
		} else {
			number = std::numeric_limits<double>::max();
		}
		const auto value = FloatValue(number);
		Jeiobs::Control::TransformState state;
		const auto result = safe_pipeline.value->Apply(*value.value, state);
		if (result.ok() && result.value->value.has_value()) {
			JC_CHECK(std::isfinite(result.value->value->AsDouble()));
		}
	}
}

} // namespace

int main()
{
	try {
		TestValues();
		TestAddresses();
		TestMapping();
		TestCompileChecks();
		TestTransformStages();
		TestTransformExtraCoverage();
		TestTransformRollback();
		TestTransformProperties();
	} catch (...) {
		++JcTest::failures;
	}
	return JcTest::failures == 0 ? 0 : 1;
}
