/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 or later.
 */
#pragma once

#include "jeiobs/workspace/Fs.hpp"
#include "jeiobs/workspace/Manifest.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Jeiobs::Workspace {

struct RootSpec {
	std::string path;
	bool custom = false;

	RootSpec() = default;
	RootSpec(std::string value) : path(std::move(value)) {}

	RootSpec &operator=(std::string value)
	{
		path = std::move(value);
		custom = false;
		return *this;
	}
};

struct Roots {
	RootSpec config;
	RootSpec user;
	RootSpec scenes;
	RootSpec profiles;
	RootSpec plugin_manager;
	std::string app_dir = "obs-studio";
};

class UidSource {
public:
	virtual ~UidSource() = default;
	[[nodiscard]] virtual std::string NextUid() = 0;
};

class RandomUidSource final : public UidSource {
public:
	[[nodiscard]] std::string NextUid() override;
};

class SeededUidSource final : public UidSource {
public:
	explicit SeededUidSource(std::string seed);
	explicit SeededUidSource(std::uint64_t seed);

	[[nodiscard]] std::string NextUid() override;

private:
	std::string seed_;
	std::uint64_t sequence_ = 0U;
};

struct PlannedEntry {
	std::string id;
	std::string status;
	std::string root;
	std::string path;
	std::string role;
	std::uint64_t size = 0U;
	std::optional<std::string> fidelity;
	std::optional<std::string> collection;
	std::optional<std::string> profile;
	std::optional<std::string> bak;
	std::optional<std::string> load_rule;
	std::string reason;
	std::string kind;
	std::uint64_t count = 0U;
	std::vector<std::string> scene_uuids;
	bool has_scene_uuids = false;
	bool collision = false;
};

struct PlannedCollection {
	std::string uid;
	std::string path;
	std::vector<std::string> scene_uuids;
};

struct PlannedProfile {
	std::string uid;
	std::string dir;
};

struct PlanWarning {
	std::string code;
	std::string path;
};

struct Plan {
	Roots roots;
	std::vector<PlannedEntry> entries;
	std::vector<PlannedCollection> collections;
	std::vector<PlannedProfile> profiles;
	std::vector<PlanWarning> warnings;
};

struct PlanResult {
	std::optional<Plan> value;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

class CapturePlanner {
public:
	[[nodiscard]] static PlanResult Plan(const Fs &fs, const Roots &roots, const Manifest *parent = nullptr);
	[[nodiscard]] static PlanResult Plan(const Fs &fs, const Roots &roots, const Manifest &parent)
	{
		return Plan(fs, roots, &parent);
	}
	[[nodiscard]] static PlanResult Plan(const Fs &fs, const Roots &roots, const Manifest *parent,
					     UidSource &uid_source);
	[[nodiscard]] static PlanResult Plan(const Fs &fs, const Roots &roots, const Manifest &parent,
					     UidSource &uid_source)
	{
		return Plan(fs, roots, &parent, uid_source);
	}
};

[[nodiscard]] Json PlanToJson(const Plan &plan);

} // namespace Jeiobs::Workspace
