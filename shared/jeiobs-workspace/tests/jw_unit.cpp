/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
#include "jw_test.hpp"

#include "jeiobs/workspace/Hash.hpp"
#include "jeiobs/workspace/Manifest.hpp"
#include "jeiobs/workspace/PathRules.hpp"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <string>

namespace {

using Jeiobs::Workspace::Json;
using Jeiobs::Workspace::ManifestMode;

const char *kLocalManifest = R"json({
  "format": "jeiobs-workspace",
  "major": 1,
  "minor": 0,
  "compat": {"incompat": [], "ro_compat": []},
  "workspace_id": "workspace",
  "seq": 1,
  "parent": null,
  "created_utc": "2026-11-16T00:00:00Z",
  "creator": {},
  "active": {},
  "target": {"os": "linux"},
  "app_keys": [],
  "entries": [{
    "id": "entry",
    "status": "stored",
    "root": "config",
    "path": "global.ini",
    "role": "config",
    "size": 3,
    "hash": "0000000000000000000000000000000000000000000000000000000000000000",
    "fidelity": "exact"
  }],
  "secret_refs": [],
  "dropped_ext": []
})json";

void TestHashes()
{
	const auto empty = Jeiobs::Workspace::HashBytes("");
	JW_CHECK(empty.ok());
	JW_CHECK_EQ(empty.value, "0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8");

	const auto abc = Jeiobs::Workspace::HashBytes("abc");
	JW_CHECK(abc.ok());
	JW_CHECK_EQ(abc.value, "bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319");
}

void TestPaths()
{
	const auto valid = Jeiobs::Workspace::ValidatePath("basic/scenes/Main.json");
	JW_CHECK(valid.status == Jeiobs::Workspace::PathStatus::Ok);

	const auto unchecked = Jeiobs::Workspace::ValidatePath(u8"basic/scenes/Mün.json");
	JW_CHECK(unchecked.status == Jeiobs::Workspace::PathStatus::Unchecked);

	const auto dot = Jeiobs::Workspace::ValidatePath("basic//Main.json");
	JW_CHECK_EQ(dot.rule, "dot");

	const auto device = Jeiobs::Workspace::ValidatePath("CON.txt");
	JW_CHECK_EQ(device.rule, "device-name");
}

void TestManifest()
{
	const auto parsed = Jeiobs::Workspace::ParseManifest(kLocalManifest, ManifestMode::Local);
	JW_CHECK(parsed.ok());
	JW_CHECK(!parsed.value->readonly);
	JW_CHECK(!parsed.value->uncheckedPaths);

	const auto duplicate = Jeiobs::Workspace::ParseManifest(
		R"json({"format":"jeiobs-workspace","format":"jeiobs-workspace"})json", ManifestMode::Local);
	JW_CHECK(duplicate.error.code == Jeiobs::Workspace::ErrorCode::DuplicateKey);

	const auto nul =
		Jeiobs::Workspace::ParseManifest(R"json({"format":"jeiobs-workspace\u0000"})json", ManifestMode::Local);
	JW_CHECK(nul.error.code == Jeiobs::Workspace::ErrorCode::Nul);
}

void TestCarryOverAndExport()
{
	Json parent_document = Json::parse(kLocalManifest);
	parent_document["future_top"] = {{"enabled", true}};
	parent_document["entries"][0]["future_entry"] = "preserve";
	const auto parent = Jeiobs::Workspace::ParseManifest(parent_document.dump(), ManifestMode::Local);
	JW_CHECK(parent.ok());

	Json draft_document = Json::parse(kLocalManifest);
	const auto draft = Jeiobs::Workspace::ParseManifest(draft_document.dump(), ManifestMode::Local);
	JW_CHECK(draft.ok());

	const auto child = Jeiobs::Workspace::CarryOver(*parent.value, *draft.value);
	JW_CHECK(child.ok());
	JW_CHECK(child.value->at("future_top").at("enabled").get<bool>());
	JW_CHECK_EQ(child.value->at("entries")[0].at("future_entry").get<std::string>(), "preserve");

	const Json no_decisions = Json::object();
	const auto exported =
		Jeiobs::Workspace::ProjectForExport(*parent.value, std::string(64, 'a'), no_decisions, "now");
	JW_CHECK(exported.ok());
	JW_CHECK(exported.value->contains("origin"));
	JW_CHECK(!exported.value->contains("workspace_id"));
	JW_CHECK(exported.value->at("dropped_ext").size() == 2U);
	JW_CHECK(exported.value->at("dropped_ext")[0].at("entry").is_null());

	Json opaque_document = Json::parse(kLocalManifest);
	opaque_document["entries"][0]["fidelity"] = "opaque-unreviewed";
	opaque_document["entries"][0]["unreviewed"] = {{"rules", "u1"}, {"count", 1}};
	const auto opaque = Jeiobs::Workspace::ParseManifest(opaque_document.dump(), ManifestMode::Local);
	JW_CHECK(opaque.ok());
	Json decisions = {{"entry", {{"decision", "include"}, {"fidelity", "redacted"}}}};
	const auto projected =
		Jeiobs::Workspace::ProjectForExport(*opaque.value, std::string(64, 'b'), decisions, "now");
	JW_CHECK(projected.ok());
	JW_CHECK_EQ(projected.value->at("entries")[0].at("fidelity").get<std::string>(), "redacted");
	JW_CHECK_EQ(projected.value->at("entries")[0].at("review").at("rules").get<std::string>(), "u1");
}

void TestFileHash()
{
	const std::string path = "jw-unit-hash-input";
	{
		std::ofstream output(path, std::ios::binary);
		output << "abc";
	}
	const auto result = Jeiobs::Workspace::HashFile(path);
	JW_CHECK(result.ok());
	JW_CHECK_EQ(result.value, "bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319");
	std::remove(path.c_str());
}

} // namespace

int main()
{
	TestHashes();
	TestPaths();
	TestManifest();
	TestCarryOverAndExport();
	TestFileHash();
	return JwTest::failures == 0 ? 0 : 1;
}
