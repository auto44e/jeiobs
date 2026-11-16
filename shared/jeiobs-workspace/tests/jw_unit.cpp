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
#include "jeiobs/workspace/IniScan.hpp"
#include "jeiobs/workspace/JsonScan.hpp"
#include "jeiobs/workspace/Manifest.hpp"
#include "jeiobs/workspace/PathRules.hpp"
#include "jeiobs/workspace/Redactor.hpp"

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
	    "root": "user",
	    "path": "user.ini",
	    "role": "app.user",
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
	JW_CHECK_EQ(Jeiobs::Workspace::ValidatePath("CON .txt").rule, "device-name");
	JW_CHECK_EQ(Jeiobs::Workspace::ValidatePath("nul  .json").rule, "device-name");
	JW_CHECK_EQ(Jeiobs::Workspace::ValidatePath("aux .").rule, "device-name");
	JW_CHECK_EQ(Jeiobs::Workspace::ValidatePath("CON ").rule, "trailing-dot-space");
}

void CheckLocalSchema(const Json &document, Jeiobs::Workspace::ErrorCode code, const char *detail)
{
	const auto result = Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local);
	JW_CHECK(result.error.code == code);
	JW_CHECK_EQ(result.error.detail, detail);
}

void TestEntryRules()
{
	Json document = Json::parse(kLocalManifest);
	document["entries"][0]["root"] = "config";
	document["entries"][0]["path"] = "plugin_config/obs-websocket/config.json";
	document["entries"][0]["role"] = "module.obs-websocket";
	JW_CHECK(Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local).ok());

	document["entries"][0]["role"] = "wrong";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "role");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["root"] = "profiles";
	document["entries"][0]["path"] = "basic/profiles/Main/basic.ini";
	document["entries"][0]["role"] = "profile.basic";
	JW_CHECK(Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local).ok());

	document["entries"][0]["path"] = "basic/profiles/Main/other.ini";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "role");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["fidelity"] = "opaque-unreviewed";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "unreviewed");

	document["entries"][0]["unreviewed"] = {{"rules", "rules"}};
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "unreviewed");

	document["entries"][0]["unreviewed"]["count"] = 0;
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "unreviewed");

	document["entries"][0]["unreviewed"]["count"] = 1;
	document["entries"][0]["unreviewed"]["rules"] = "";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "unreviewed");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["unreviewed"] = {{"rules", "rules"}, {"count", 1}};
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "unreviewed");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["parse"] = "invalid";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "parse");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["status"] = "omitted";
	document["entries"][0].erase("role");
	document["entries"][0]["reason"] = "unknown";
	document["entries"][0]["fidelity"] = "exact";
	document["entries"][0]["hash"] = document["entries"][0]["hash"];
	document["entries"][0]["redactions"] = Json::array();
	document["entries"][0]["unreviewed"] = Json::object();
	document["entries"][0]["parse"] = "invalid";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "fidelity");
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

void TestExportRules()
{
	const auto parsed = Jeiobs::Workspace::ParseManifest(kLocalManifest, ManifestMode::Local);
	JW_CHECK(parsed.ok());

	const Json no_decisions = Json::object();
	const auto invalid_now = Jeiobs::Workspace::ProjectForExport(*parsed.value, std::string(64, 'a'), no_decisions,
								     "2026-02-29T00:00:00Z");
	JW_CHECK(invalid_now.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK_EQ(invalid_now.error.detail, "now");

	const auto valid_leap = Jeiobs::Workspace::ProjectForExport(*parsed.value, std::string(64, 'a'), no_decisions,
								    "2024-02-29T23:59:59Z");
	JW_CHECK(valid_leap.ok());

	Json opaque_document = Json::parse(kLocalManifest);
	opaque_document["entries"][0]["fidelity"] = "opaque-unreviewed";
	opaque_document["entries"][0]["unreviewed"] = {{"rules", "u1"}, {"count", 1}};
	const auto opaque = Jeiobs::Workspace::ParseManifest(opaque_document.dump(), ManifestMode::Local);
	JW_CHECK(opaque.ok());

	Json decisions = {{"entry", {{"decision", "include"}}}};
	const auto exact = Jeiobs::Workspace::ProjectForExport(*opaque.value, std::string(64, 'b'), decisions,
							       "2026-11-16T00:00:00Z");
	JW_CHECK(exact.ok());
	JW_CHECK_EQ(exact.value->at("entries")[0].at("fidelity").get<std::string>(), "exact");

	opaque_document["entries"][0]["redactions"] = {{{"pointer", "/secret"}, {"kind", "secret"}}};
	const auto redacted_manifest = Jeiobs::Workspace::ParseManifest(opaque_document.dump(), ManifestMode::Local);
	JW_CHECK(redacted_manifest.ok());
	decisions["entry"]["fidelity"] = "redacted";
	const auto redacted = Jeiobs::Workspace::ProjectForExport(*redacted_manifest.value, std::string(64, 'b'),
								  decisions, "2026-11-16T00:00:00Z");
	JW_CHECK(redacted.ok());
	JW_CHECK_EQ(redacted.value->at("entries")[0].at("fidelity").get<std::string>(), "redacted");

	decisions["entry"]["fidelity"] = "exact";
	const auto mismatched = Jeiobs::Workspace::ProjectForExport(*redacted_manifest.value, std::string(64, 'b'),
								    decisions, "2026-11-16T00:00:00Z");
	JW_CHECK(mismatched.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK_EQ(mismatched.error.detail, "decisions");
}

void TestExportObjectRebuild()
{
	Json document = Json::parse(kLocalManifest);
	document["creator"] = {{"app", "jeiobs"}, {"x", 1}};
	document["active"] = {{"profile", "profile"}, {"x", 2}};
	document["compat"]["x"] = 3;
	document["target"] = {
		{"os", "linux"},
		{"audio", {{"sample_rate", 48000}, {"channels", "Stereo"}, {"x", 4}}},
		{"x", 5},
	};
	document["app_keys"] = {
		{{"root", "config"},
		 {"file", "global.ini"},
		 {"section", "Video"},
		 {"key", "Renderer"},
		 {"value", "OpenGL"},
		 {"restore", "same-os"},
		 {"x", 6}},
		{{"root", "config"},
		 {"file", "global.ini"},
		 {"section", "General"},
		 {"key", "LastVersion"},
		 {"value", "1"},
		 {"restore", "never"}},
	};
	document["future"] = true;
	document["dropped_ext"] = {{{"entry", nullptr}, {"fields", {"old"}}}};
	const auto parsed = Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local);
	JW_CHECK(parsed.ok());

	const auto exported = Jeiobs::Workspace::ProjectForExport(*parsed.value, std::string(64, 'c'), Json::object(),
								  "2026-11-16T00:00:00Z");
	JW_CHECK(exported.ok());
	JW_CHECK(!exported.value->at("creator").contains("x"));
	JW_CHECK(!exported.value->at("active").contains("x"));
	JW_CHECK(!exported.value->at("compat").contains("x"));
	JW_CHECK(!exported.value->at("target").contains("x"));
	JW_CHECK(!exported.value->at("target").at("audio").contains("x"));
	JW_CHECK(!exported.value->at("app_keys")[0].contains("x"));
	JW_CHECK_EQ(exported.value->at("app_keys").size(), 1U);
	JW_CHECK(exported.value->at("dropped_ext")[0].at("entry").is_null());
	JW_CHECK_EQ(exported.value->at("dropped_ext")[0].at("fields")[0].get<std::string>(), "old");
	JW_CHECK_EQ(exported.value->at("dropped_ext")[1].at("fields").size(), 7U);
	JW_CHECK(Jeiobs::Workspace::ParseManifest(exported.value->dump(), ManifestMode::Export).ok());

	Json export_document = *exported.value;
	export_document["target"]["arch"] = "x86_64";
	const auto invalid_target = Jeiobs::Workspace::ParseManifest(export_document.dump(), ManifestMode::Export);
	JW_CHECK(invalid_target.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK_EQ(invalid_target.error.detail, "target");

	export_document = *exported.value;
	export_document["app_keys"].push_back({{"restore", "never"}});
	const auto invalid_app_key = Jeiobs::Workspace::ParseManifest(export_document.dump(), ManifestMode::Export);
	JW_CHECK(invalid_app_key.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK_EQ(invalid_app_key.error.detail, "app_keys");
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
	const auto exported = Jeiobs::Workspace::ProjectForExport(*parent.value, std::string(64, 'a'), no_decisions,
								  "2026-11-16T00:00:00Z");
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
	Json decisions = {{"entry", {{"decision", "include"}}}};
	const auto projected = Jeiobs::Workspace::ProjectForExport(*opaque.value, std::string(64, 'b'), decisions,
								   "2026-11-16T00:00:00Z");
	JW_CHECK(projected.ok());
	JW_CHECK_EQ(projected.value->at("entries")[0].at("fidelity").get<std::string>(), "exact");
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

	const auto directory_hash = Jeiobs::Workspace::HashFile(".");
	JW_CHECK(directory_hash.error.code == Jeiobs::Workspace::ErrorCode::Io);
	const auto directory_read = Jeiobs::Workspace::ReadFile(".");
	JW_CHECK(directory_read.error.code == Jeiobs::Workspace::ErrorCode::Io);
}

void TestScanners()
{
	const auto json = Jeiobs::Workspace::JsonScan::Scan(
		R"json({"a/b~c":"value","settings":{"key":"secret"},"items":[{"name":"one"}],"dup":1,"dup":2})json");
	JW_CHECK(!json.ok());
	JW_CHECK(json.duplicate_key);
	JW_CHECK(json.strings.size() == 3U);
	JW_CHECK(json.strings[0].pointer == "/a~1b~0c");
	JW_CHECK(json.strings[1].pointer == "/settings/key");
	JW_CHECK(json.strings[2].pointer == "/items/0/name");
	JW_CHECK(json.members[0].key == "a/b~c");
	JW_CHECK(json.members[0].key_start < json.members[0].key_end);

	const auto deep = Jeiobs::Workspace::JsonScan::Scan(
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[0]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]");
	JW_CHECK(deep.ok());

	const auto ini = Jeiobs::Workspace::IniScan::Scan(
		"\xef\xbb\xbf[Twitch]\r\nToken=one=two\r\nToken=three\n# comment\n[Other]\nkey=value\n");
	JW_CHECK(ini.ok());
	JW_CHECK(ini.has_bom);
	JW_CHECK(ini.sections.size() == 2U);
	JW_CHECK(ini.keys.size() == 3U);
	JW_CHECK(ini.keys[0].pointer == "/Twitch/Token");
	JW_CHECK(ini.keys[0].value_start < ini.keys[0].value_end);
	JW_CHECK(ini.keys[0].value_end - ini.keys[0].value_start == 7U);

	const auto ini_pointer = Jeiobs::Workspace::IniScan::Scan("[a/b~c]\nkey=value\n");
	JW_CHECK(ini_pointer.ok());
	JW_CHECK(ini_pointer.keys[0].pointer == "/a~1b~0c/key");
}

void TestRedactor()
{
	const std::string service =
		R"json({"type":"rtmp_common","settings":{"key":"STREAM","bearer_token":"BEARER","username":"USER","password":"PASS","server":"rtmp://u:p@example/live?token=QUERY&x=y"}})json";
	const auto service_result = Jeiobs::Workspace::Redact("profile.service", service);
	JW_CHECK(service_result.ok());
	JW_CHECK(service_result.report.outcome == Jeiobs::Workspace::RedactOutcome::Stored);
	JW_CHECK(service_result.report.fidelity == "redacted");
	JW_CHECK(service_result.report.redactions.size() == 6U);
	JW_CHECK(service_result.bytes.find("STREAM") == std::string::npos);
	JW_CHECK(service_result.bytes.find("u:p@") == std::string::npos);
	JW_CHECK(service_result.bytes.find("QUERY") == std::string::npos);

	Jeiobs::Workspace::RedactOptions websocket_options;
	websocket_options.ws_placeholder = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
	websocket_options.has_ws_placeholder = true;
	const auto websocket = Jeiobs::Workspace::Redact("module.obs-websocket",
							 R"json({"server_password":"old","auth_required":false})json",
							 websocket_options);
	JW_CHECK(websocket.ok());
	JW_CHECK(websocket.report.redactions.size() == 2U);
	JW_CHECK(websocket.bytes.find("\"auth_required\":true") != std::string::npos);
	JW_CHECK(websocket.bytes.find(websocket_options.ws_placeholder) != std::string::npos);
	JW_CHECK(websocket.report.redactions[1].has_was);
	JW_CHECK(!websocket.report.redactions[1].was);

	const auto basic = Jeiobs::Workspace::Redact(
		"profile.basic",
		"[Twitch]\nToken=oauth\n[AdvOut]\nFFURL=rtmp://user:pass@host/live?key=secret\n[Third]\nX=y\n");
	JW_CHECK(basic.ok());
	JW_CHECK(basic.report.fidelity == "opaque-unreviewed");
	JW_CHECK(basic.report.unreviewed_count == 2U);
	JW_CHECK(basic.bytes.find("oauth") == std::string::npos);
	JW_CHECK(basic.bytes.find("user:pass@") == std::string::npos);

	const auto collection = Jeiobs::Workspace::Redact(
		"collection",
		R"json({"sources":[{"id":"browser_source","settings":{"url":"https://u:p@host/?token=q"}},{"id":"scene"}]})json");
	JW_CHECK(collection.ok());
	JW_CHECK(collection.report.fidelity == "opaque-unreviewed");
	JW_CHECK(collection.report.unreviewed_count == 1U);
	JW_CHECK(collection.bytes.find("u:p@") == std::string::npos);
	JW_CHECK(collection.bytes.find("token=") != std::string::npos);

	const auto invalid_service =
		Jeiobs::Workspace::Redact("profile.service", R"json({"type":"rtmp_common","type":"rtmp_common"})json");
	JW_CHECK(invalid_service.ok());
	JW_CHECK(invalid_service.report.outcome == Jeiobs::Workspace::RedactOutcome::Omitted);
	JW_CHECK(invalid_service.report.reason == "unparseable");
	JW_CHECK(invalid_service.bytes.empty());

	const auto invalid_encoder = Jeiobs::Workspace::Redact("profile.encoder.stream", "{");
	JW_CHECK(invalid_encoder.ok());
	JW_CHECK(invalid_encoder.report.outcome == Jeiobs::Workspace::RedactOutcome::Stored);
	JW_CHECK(invalid_encoder.report.parse_invalid);
	JW_CHECK(invalid_encoder.bytes == "{");

	Jeiobs::Workspace::RedactOptions invalid_options;
	invalid_options.ws_placeholder = "short";
	invalid_options.has_ws_placeholder = true;
	const auto invalid_placeholder = Jeiobs::Workspace::Redact(
		"module.obs-websocket", R"json({"server_password":"old"})json", invalid_options);
	JW_CHECK(invalid_placeholder.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK(invalid_placeholder.error.detail == "ws-placeholder");
}

} // namespace

int main()
{
	TestHashes();
	TestPaths();
	TestManifest();
	TestEntryRules();
	TestExportRules();
	TestExportObjectRebuild();
	TestCarryOverAndExport();
	TestFileHash();
	TestScanners();
	TestRedactor();
	return JwTest::failures == 0 ? 0 : 1;
}
