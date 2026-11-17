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
#include "jeiobs/control/Transform.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <string_view>
#include <utility>

namespace Jeiobs::Control {
namespace {

enum class SignalKind {
	Number,
	Bool,
	UInt,
	Tuple,
	NoOutput,
};

struct Signal {
	SignalKind kind = SignalKind::Number;
	double number = 0.0;
	bool boolean = false;
	const Value *value = nullptr;
};

Error Schema(std::string_view detail)
{
	return MakeError(ErrorCode::Schema, std::string(detail));
}

Error ValueError(std::string_view detail)
{
	return MakeError(ErrorCode::Value, std::string(detail));
}

bool IsKnownOperation(std::string_view operation) noexcept
{
	return operation == "scale" || operation == "curve" || operation == "invert" || operation == "clamp" ||
	       operation == "deadzone" || operation == "quantize" || operation == "threshold" ||
	       operation == "relative" || operation == "toggle" || operation == "momentary" || operation == "join14" ||
	       operation == "split14";
}

bool IsFiniteNumbers(const TransformSpec &stage) noexcept
{
	for (const double number : stage.numbers) {
		if (!std::isfinite(number)) {
			return false;
		}
	}
	return true;
}

bool IsValidStage(const TransformSpec &stage) noexcept
{
	if (!IsKnownOperation(stage.op) || !IsFiniteNumbers(stage)) {
		return false;
	}
	if (stage.op == "scale") {
		return stage.numbers.size() == 4U && stage.text.empty() && stage.integer == 0 &&
		       stage.numbers[0] != stage.numbers[1];
	}
	if (stage.op == "curve") {
		return stage.numbers.empty() && stage.integer == 0 &&
		       (stage.text == "linear" || stage.text == "log" || stage.text == "exp");
	}
	if (stage.op == "clamp") {
		return stage.numbers.size() == 2U && stage.text.empty() && stage.integer == 0 &&
		       stage.numbers[0] <= stage.numbers[1];
	}
	if (stage.op == "deadzone") {
		return stage.numbers.size() == 1U && stage.text.empty() && stage.integer == 0 &&
		       stage.numbers[0] >= 0.0 && stage.numbers[0] < 1.0;
	}
	if (stage.op == "quantize") {
		return stage.numbers.empty() && stage.text.empty() && stage.integer >= 2;
	}
	if (stage.op == "threshold") {
		return stage.numbers.size() == 1U && stage.text.empty() && stage.integer == 0;
	}
	if (stage.op == "relative") {
		return stage.numbers.size() == 1U && stage.integer == 0 && stage.numbers[0] > 0.0 &&
		       (stage.text == "twos_complement" || stage.text == "sign_magnitude" ||
			stage.text == "binary_offset");
	}
	return stage.numbers.empty() && stage.text.empty() && stage.integer == 0;
}

bool HasInvalidPosition(const std::vector<TransformSpec> &stages) noexcept
{
	for (std::size_t index = 0U; index < stages.size(); ++index) {
		const std::string &operation = stages[index].op;
		if ((operation == "join14" || operation == "relative") && index != 0U) {
			return true;
		}
		if (operation == "split14" && index + 1U != stages.size()) {
			return true;
		}
	}
	return false;
}

double NormalizeZero(double value) noexcept
{
	return value == 0.0 ? 0.0 : value;
}

bool SetNumber(Signal &signal, double value) noexcept
{
	if (!std::isfinite(value)) {
		return false;
	}
	signal.kind = SignalKind::Number;
	signal.number = NormalizeZero(value);
	signal.boolean = false;
	signal.value = nullptr;
	return true;
}

Result<double> AsNumber(const Signal &signal)
{
	switch (signal.kind) {
	case SignalKind::Number:
		if (!std::isfinite(signal.number)) {
			return {0.0, ValueError("nonfinite")};
		}
		return {NormalizeZero(signal.number), {}};
	case SignalKind::Bool:
		return {signal.boolean ? 1.0 : 0.0, {}};
	case SignalKind::UInt:
		if (signal.value == nullptr) {
			return {0.0, ValueError("type")};
		}
		if (!std::isfinite(signal.value->AsDouble())) {
			return {0.0, ValueError("nonfinite")};
		}
		return {NormalizeZero(signal.value->AsDouble()), {}};
	case SignalKind::Tuple:
		return {0.0, ValueError("type")};
	case SignalKind::NoOutput:
		return {0.0, ValueError("type")};
	}
	return {0.0, ValueError("type")};
}

Result<Signal> InitialSignal(const Value &input)
{
	switch (input.kind()) {
	case ValueKind::UInt:
		return {Signal{SignalKind::UInt, 0.0, false, &input}, {}};
	case ValueKind::Tuple:
		return {Signal{SignalKind::Tuple, 0.0, false, &input}, {}};
	case ValueKind::Bool:
		return {Signal{SignalKind::Bool, 0.0, input.boolValue(), nullptr}, {}};
	case ValueKind::Trigger:
		return {Signal{SignalKind::Number, 1.0, false, nullptr}, {}};
	case ValueKind::Int:
	case ValueKind::FloatBits: {
		const double number = input.AsDouble();
		if (!std::isfinite(number)) {
			return {{}, ValueError("nonfinite")};
		}
		return {Signal{SignalKind::Number, NormalizeZero(number), false, nullptr}, {}};
	}
	case ValueKind::Text:
	case ValueKind::Blob:
	case ValueKind::Invalid:
		return {{}, ValueError("type")};
	}
	return {{}, ValueError("type")};
}

double DecodeRelative(const Value &value, const TransformSpec &stage) noexcept
{
	const unsigned int width = value.widthBits();
	const std::uint64_t raw = value.uintRaw().low;
	const std::uint64_t half_code = std::uint64_t(1U) << (width - 1U);
	if (stage.text == "twos_complement") {
		if (raw < half_code) {
			return static_cast<double>(raw);
		}
		const std::uint64_t magnitude = width == 64U ? (~raw) + 1U : (std::uint64_t(1U) << width) - raw;
		return -static_cast<double>(magnitude);
	}
	if (stage.text == "sign_magnitude") {
		const std::uint64_t magnitude = raw & (half_code - 1U);
		const double signed_magnitude = static_cast<double>(magnitude);
		return (raw & half_code) == 0U ? signed_magnitude : -signed_magnitude;
	}
	if (raw < half_code) {
		return -static_cast<double>(half_code - raw);
	}
	return static_cast<double>(raw - half_code);
}

Result<Value> NumberValue(double number)
{
	if (!std::isfinite(number)) {
		return {{}, ValueError("nonfinite")};
	}
	number = NormalizeZero(number);
	std::uint64_t bits = 0U;
	static_assert(sizeof(bits) == sizeof(number), "double must be 64-bit");
	std::memcpy(&bits, &number, sizeof(bits));
	auto value = Value::MakeFloatBits(FloatFormat::Ieee64, bits);
	if (!value.ok()) {
		return {{}, ValueError(value.error.detail.empty() ? "value" : value.error.detail)};
	}
	return value;
}

Result<Signal> MakeUIntSignal(Value value, std::optional<Value> &owned)
{
	owned = std::move(value);
	return {Signal{SignalKind::UInt, 0.0, false, &*owned}, {}};
}

Result<Signal> MakeTupleSignal(Value value, std::optional<Value> &owned)
{
	owned = std::move(value);
	return {Signal{SignalKind::Tuple, 0.0, false, &*owned}, {}};
}

Result<Signal> ApplyStage(const TransformSpec &stage, std::size_t index, std::size_t stage_count, Signal signal,
			  TransformState &state, std::optional<Value> &owned)
{
	if (stage.op == "join14") {
		if (index != 0U || signal.kind != SignalKind::Tuple || signal.value == nullptr ||
		    signal.value->tuple().size() != 2U) {
			return {{}, ValueError("type")};
		}
		const Value &msb = signal.value->tuple()[0];
		const Value &lsb = signal.value->tuple()[1];
		if (msb.kind() != ValueKind::UInt || lsb.kind() != ValueKind::UInt || msb.widthBits() < 7U ||
		    lsb.widthBits() < 7U) {
			return {{}, ValueError("type")};
		}
		if (msb.uintRaw().high != 0U || lsb.uintRaw().high != 0U || msb.uintRaw().low > 127U ||
		    lsb.uintRaw().low > 127U) {
			return {{}, ValueError("range")};
		}
		auto result = Value::MakeUInt(14U, {0U, msb.uintRaw().low * 128U + lsb.uintRaw().low});
		if (!result.ok()) {
			return {{}, ValueError(result.error.detail.empty() ? "type" : result.error.detail)};
		}
		return MakeUIntSignal(std::move(*result.value), owned);
	}

	if (stage.op == "split14") {
		if (index + 1U != stage_count || signal.kind != SignalKind::UInt || signal.value == nullptr ||
		    signal.value->widthBits() != 14U) {
			return {{}, ValueError("type")};
		}
		const std::uint64_t raw = signal.value->uintRaw().low;
		auto msb = Value::MakeUInt(7U, {0U, raw >> 7U});
		auto lsb = Value::MakeUInt(7U, {0U, raw & 127U});
		if (!msb.ok() || !lsb.ok()) {
			return {{}, ValueError("type")};
		}
		auto result = Value::MakeTuple({std::move(*msb.value), std::move(*lsb.value)});
		if (!result.ok()) {
			return {{}, ValueError(result.error.detail.empty() ? "type" : result.error.detail)};
		}
		return MakeTupleSignal(std::move(*result.value), owned);
	}

	if (stage.op == "relative") {
		if (index != 0U || signal.kind != SignalKind::UInt || signal.value == nullptr ||
		    signal.value->widthBits() == 0U || signal.value->widthBits() > 64U ||
		    signal.value->uintRaw().high != 0U || !std::isfinite(state.position)) {
			return {{},
				ValueError(signal.value != nullptr && !std::isfinite(state.position) ? "nonfinite"
												     : "type")};
		}
		const double delta = DecodeRelative(*signal.value, stage);
		const double change = delta * stage.numbers[0];
		double position = state.position;
		if (std::isinf(change)) {
			position = std::signbit(change) ? 0.0 : 1.0;
		} else {
			position += change;
			if (!std::isfinite(position)) {
				position = std::signbit(position) ? 0.0 : 1.0;
			}
		}
		state.position = std::clamp(position, 0.0, 1.0);
		if (!SetNumber(signal, state.position)) {
			return {{}, ValueError("nonfinite")};
		}
		return {std::move(signal), {}};
	}

	if (stage.op == "toggle") {
		auto number = AsNumber(signal);
		if (!number.ok()) {
			return {{}, number.error};
		}
		if (*number.value == 0.0) {
			return {Signal{SignalKind::NoOutput, 0.0, false, nullptr}, {}};
		}
		state.toggled = !state.toggled;
		signal.kind = SignalKind::Bool;
		signal.boolean = state.toggled;
		signal.value = nullptr;
		return {std::move(signal), {}};
	}

	if (stage.op == "momentary" || stage.op == "threshold" || stage.op == "scale" || stage.op == "curve" ||
	    stage.op == "invert" || stage.op == "clamp" || stage.op == "deadzone" || stage.op == "quantize") {
		auto number = AsNumber(signal);
		if (!number.ok()) {
			return {{}, number.error};
		}
		const double input = *number.value;
		if (stage.op == "threshold") {
			signal.kind = SignalKind::Bool;
			signal.boolean = input >= stage.numbers[0];
			signal.value = nullptr;
			return {std::move(signal), {}};
		}
		if (stage.op == "momentary") {
			signal.kind = SignalKind::Bool;
			signal.boolean = input != 0.0;
			signal.value = nullptr;
			return {std::move(signal), {}};
		}

		double output = input;
		if (stage.op == "scale") {
			if (input == stage.numbers[0]) {
				output = stage.numbers[2];
			} else if (input == stage.numbers[1]) {
				output = stage.numbers[3];
			} else {
				output -= stage.numbers[0];
				output *= stage.numbers[3] - stage.numbers[2];
				output /= stage.numbers[1] - stage.numbers[0];
				output += stage.numbers[2];
			}
		} else if (stage.op == "curve") {
			const double normalized = std::clamp(input, 0.0, 1.0);
			if (normalized == 0.0 || normalized == 1.0 || stage.text == "linear") {
				output = normalized;
			} else if (stage.text == "log") {
				output = std::log10(1.0 + 9.0 * normalized);
			} else {
				output = (std::pow(10.0, normalized) - 1.0) / 9.0;
			}
		} else if (stage.op == "invert") {
			output = 1.0 - input;
		} else if (stage.op == "clamp") {
			output = std::clamp(input, stage.numbers[0], stage.numbers[1]);
		} else if (stage.op == "deadzone") {
			const double width = stage.numbers[0];
			const double magnitude = std::abs(input);
			if (magnitude <= width) {
				output = 0.0;
			} else {
				output = std::copysign((magnitude - width) / (1.0 - width), input);
			}
		} else if (stage.op == "quantize") {
			const double levels = static_cast<double>(stage.integer - 1);
			output = std::nearbyint(std::clamp(input, 0.0, 1.0) * levels) / levels;
		}
		if (!SetNumber(signal, output)) {
			return {{}, ValueError("nonfinite")};
		}
		return {std::move(signal), {}};
	}

	return {{}, Schema("op")};
}

Result<Value> OutputValue(const Signal &signal)
{
	switch (signal.kind) {
	case SignalKind::Number:
		return NumberValue(signal.number);
	case SignalKind::Bool:
		return Value::MakeBool(signal.boolean);
	case SignalKind::UInt:
	case SignalKind::Tuple:
		if (signal.value == nullptr) {
			return {{}, ValueError("type")};
		}
		return {*signal.value, {}};
	case SignalKind::NoOutput:
		return {{}, ValueError("type")};
	}
	return {{}, ValueError("type")};
}

} // namespace

Result<TransformPipeline> TransformPipeline::Compile(const std::vector<TransformSpec> &stages)
{
	try {
		if (stages.size() > Limits::kTransforms) {
			return {{}, MakeError(ErrorCode::Limit, "transforms")};
		}
		if (HasInvalidPosition(stages)) {
			return {{}, Schema("transforms")};
		}
		for (const TransformSpec &stage : stages) {
			if (!IsKnownOperation(stage.op)) {
				return {{}, Schema("op")};
			}
			if (!IsValidStage(stage)) {
				return {{}, Schema("transforms")};
			}
		}
		return {TransformPipeline(std::vector<TransformSpec>(stages)), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

Result<TransformPipeline::Applied> TransformPipeline::Apply(const Value &input, TransformState &state) const
{
	try {
		if (stages_.empty()) {
			Applied applied;
			applied.outcome = Outcome::Output;
			applied.value = input;
			return {std::move(applied), {}};
		}

		auto signal = InitialSignal(input);
		if (!signal.ok()) {
			return {{}, signal.error};
		}
		TransformState working_state = state;
		std::optional<Value> owned;
		Signal working_signal = std::move(*signal.value);
		for (std::size_t index = 0U; index < stages_.size(); ++index) {
			auto applied = ApplyStage(stages_[index], index, stages_.size(), std::move(working_signal),
						  working_state, owned);
			if (!applied.ok()) {
				return {{}, applied.error};
			}
			working_signal = std::move(*applied.value);
			if (working_signal.kind == SignalKind::NoOutput) {
				state = working_state;
				Applied no_output;
				no_output.outcome = Outcome::NoOutput;
				return {std::move(no_output), {}};
			}
		}

		auto output = OutputValue(working_signal);
		if (!output.ok()) {
			return {{}, output.error};
		}
		state = working_state;
		Applied applied;
		applied.outcome = Outcome::Output;
		applied.value = std::move(*output.value);
		return {std::move(applied), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Value)};
	}
}

} // namespace Jeiobs::Control
