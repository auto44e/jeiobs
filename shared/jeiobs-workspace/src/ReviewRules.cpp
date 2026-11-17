/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 or later.
 */
#include "jeiobs/workspace/ReviewRules.hpp"

#include "jeiobs/workspace/JsonScan.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace Jeiobs::Workspace {
namespace {

constexpr std::array<std::string_view, 2U> kFirstPartySourceTypes = {"scene", "group"};

const JsonMemberOccurrence *FindMember(const JsonScanResult &scan, std::size_t parent, std::string_view key)
{
	for (const JsonMemberOccurrence &member : scan.members) {
		if (member.parent_index == parent && member.key == key) {
			return &member;
		}
	}
	return nullptr;
}

std::string_view SourceType(const JsonScanResult &scan, std::size_t source_index)
{
	const JsonMemberOccurrence *versioned = FindMember(scan, source_index, "versioned_id");
	if (versioned != nullptr && versioned->value_index < scan.values.size()) {
		const JsonValueOccurrence &value = scan.values[versioned->value_index];
		if (value.kind == JsonValueKind::String && !value.string_value.empty()) {
			return value.string_value;
		}
	}
	const JsonMemberOccurrence *id = FindMember(scan, source_index, "id");
	if (id != nullptr && id->value_index < scan.values.size()) {
		const JsonValueOccurrence &value = scan.values[id->value_index];
		if (value.kind == JsonValueKind::String) {
			return value.string_value;
		}
	}
	return {};
}

std::size_t CountArray(const JsonScanResult &scan, const JsonMemberOccurrence *member)
{
	if (member == nullptr) {
		return 0U;
	}
	if (member->value_index >= scan.values.size() ||
	    scan.values[member->value_index].kind != JsonValueKind::Array) {
		return 1U;
	}

	std::size_t unknown = 0U;
	const std::size_t array_index = member->value_index;
	for (const JsonValueOccurrence &value : scan.values) {
		if (value.parent_index != array_index || value.array_index == kJsonNoIndex) {
			continue;
		}
		if (value.kind != JsonValueKind::Object ||
		    !IsFirstPartySourceType(SourceType(scan, &value - scan.values.data()))) {
			++unknown;
		}
	}
	return unknown;
}

std::size_t CountAudioDevice(const JsonScanResult &scan, std::string_view key)
{
	const JsonMemberOccurrence *member = FindMember(scan, 0U, key);
	if (member == nullptr || member->value_index >= scan.values.size()) {
		return 0U;
	}
	const JsonValueOccurrence &value = scan.values[member->value_index];
	if (value.kind != JsonValueKind::Object || SourceType(scan, member->value_index).empty()) {
		return 1U;
	}
	return IsFirstPartySourceType(SourceType(scan, member->value_index)) ? 0U : 1U;
}

} // namespace

bool ReviewRules::IsFirstPartySource(std::string_view source_type) noexcept
{
	return std::find(kFirstPartySourceTypes.begin(), kFirstPartySourceTypes.end(), source_type) !=
	       kFirstPartySourceTypes.end();
}

bool IsFirstPartySourceType(std::string_view source_type) noexcept
{
	return ReviewRules::IsFirstPartySource(source_type);
}

ReviewResult ReviewRules::AnalyzeCollection(std::string_view bytes)
{
	const JsonScanResult scan = JsonScan::Scan(bytes);
	if (!scan.ok()) {
		return {0U, scan.error};
	}
	if (scan.values.empty() || scan.values.front().kind != JsonValueKind::Object) {
		return {1U, {}};
	}

	std::size_t unknown = CountArray(scan, FindMember(scan, 0U, "sources"));
	unknown += CountArray(scan, FindMember(scan, 0U, "groups"));
	for (const std::string_view key : {"DesktopAudioDevice1", "DesktopAudioDevice2", "AuxAudioDevice1",
					   "AuxAudioDevice2", "AuxAudioDevice3", "AuxAudioDevice4"}) {
		unknown += CountAudioDevice(scan, key);
	}
	return {std::max<std::size_t>(1U, unknown), {}};
}

} // namespace Jeiobs::Workspace
