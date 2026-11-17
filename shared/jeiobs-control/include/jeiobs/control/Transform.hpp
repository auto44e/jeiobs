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

#include "jeiobs/control/Mapping.hpp"

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace Jeiobs::Control {

struct TransformState {
	bool toggled = false;
	double position = 0.0;
};

class TransformPipeline final {
public:
	static Result<TransformPipeline> Compile(const std::vector<TransformSpec> &stages);

	enum class Outcome {
		Output,
		NoOutput,
	};

	struct Applied {
		Outcome outcome = Outcome::NoOutput;
		std::optional<Value> value;
	};

	Result<Applied> Apply(const Value &input, TransformState &state) const;

	[[nodiscard]] std::size_t size() const noexcept { return stages_.size(); }

private:
	explicit TransformPipeline(std::vector<TransformSpec> stages) noexcept : stages_(std::move(stages)) {}

	std::vector<TransformSpec> stages_;
};

} // namespace Jeiobs::Control
