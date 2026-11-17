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

#include "jeiobs/control/Control.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Jeiobs::Control {

enum class CapabilityClass {
	Mix,
	Scene,
	SourceProperty,
	Filter,
	PluginParam,
	Hotkey,
	OutputControl,
	Profile,
	Meta,
	EgressByTrust,
};

[[nodiscard]] const char *CapabilityClassName(CapabilityClass capability) noexcept;

enum class TargetKind {
	SourceProperty,
	FilterProperty,
	Scene,
	SceneItem,
	Mixer,
	Transition,
	Hotkey,
	Output,
	Profile,
	PluginParam,
	PluginEvent,
	Graph,
	Egress,
	Emit,
};

enum class HotkeyEffect {
	None,
	Scene,
	Mixer,
	Output,
	Unknown,
};

enum class OutputKind {
	Stream,
	Record,
	Replay,
	VirtualCam,
};

enum class OutputAction {
	Start,
	Stop,
	Toggle,
};

struct Slot {
	std::string id;
	std::vector<std::string> requirements;
};

enum class PredicateKind {
	Value,
	Modifier,
	Layer,
	Bank,
};

struct Predicate {
	PredicateKind kind = PredicateKind::Value;
	double minimum = 0.0;
	double maximum = 0.0;
	std::uint64_t bank = 0U;
	std::string name;
};

struct Guard {
	std::vector<Predicate> all;
};

struct TransformSpec {
	std::string op;
	std::vector<double> numbers;
	std::string text;
	std::int64_t integer = 0;
};

struct Target {
	TargetKind kind = TargetKind::Scene;
	std::string id;
	std::string field;
	std::string scene_id;
	std::string item_id;
	std::string name;
	std::string registerer_kind;
	std::string registerer_id;
	HotkeyEffect effect = HotkeyEffect::None;
	OutputKind output = OutputKind::Stream;
	OutputAction action = OutputAction::Toggle;
	std::string instance;
	std::string parameter;
	std::string node;
	std::string port;
	TrustClass trust = TrustClass::Hw;
	ControlAddress emit_address;
};

[[nodiscard]] CapabilityClass ClassOfTarget(const Target &target) noexcept;

enum class MergePolicy {
	LastWriter,
	Priority,
	TouchLease,
	Sum,
	Min,
	Max,
	BooleanOr,
};

[[nodiscard]] const char *MergePolicyName(MergePolicy policy) noexcept;

struct MergeSpec {
	MergePolicy policy = MergePolicy::LastWriter;
	std::int64_t priority = 0;
};

struct MatchSpec {
	std::optional<std::string> slot;
	ControlPattern address;
};

struct Route {
	std::string id;
	bool enabled = true;
	MatchSpec match;
	std::optional<Guard> guard;
	std::vector<TransformSpec> transforms;
	Target target;
	std::optional<MergeSpec> merge;
};

struct Mapping {
	int major = 1;
	int minor = 0;
	std::vector<Slot> slots;
	std::vector<Route> routes;
};

struct CompiledRoute {
	std::string id;
	bool enabled = true;
	CapabilityClass capability = CapabilityClass::Meta;
	std::string target_key;
	MergeSpec merge;
	std::size_t fanout = 1U;
};

struct CompiledMapping {
	std::vector<std::string> slots;
	std::vector<CompiledRoute> routes;
};

[[nodiscard]] Result<Mapping> ParseMapping(std::string_view bytes);
[[nodiscard]] Result<Mapping> LoadMapping(const std::string &path);
[[nodiscard]] Result<Mapping> ValidateMapping(const Mapping &mapping);
[[nodiscard]] Result<CompiledMapping> CompileMapping(const Mapping &mapping);
[[nodiscard]] Result<CompiledMapping> CompileMapping(std::string_view bytes);
[[nodiscard]] Result<ControlAddress> ParseEventAddress(std::string_view bytes);

[[nodiscard]] std::string TargetKey(const Target &target);
[[nodiscard]] std::size_t PatternFanout(const ControlPattern &pattern) noexcept;

} // namespace Jeiobs::Control
