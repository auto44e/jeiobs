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
#include "jeiobs/workspace/CapturePlanner.hpp"
#include "jeiobs/workspace/Fs.hpp"
#include "jeiobs/workspace/IniScan.hpp"
#include "jeiobs/workspace/JsonScan.hpp"
#include "jeiobs/workspace/Limits.hpp"
#include "jeiobs/workspace/Manifest.hpp"
#include "jeiobs/workspace/PathRules.hpp"
#include "jeiobs/workspace/Redactor.hpp"
#include "jeiobs/workspace/ReviewRules.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <clocale>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

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

	const auto non_ascii = Jeiobs::Workspace::ValidatePath(u8"basic/scenes/Mün.json");
	JW_CHECK(non_ascii.status == Jeiobs::Workspace::PathStatus::Ok);

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
	document["entries"][0].erase("hash");
	document["entries"][0].erase("fidelity");
	document["entries"][0]["kind"] = "dir";
	document["entries"][0]["count"] = 1;
	JW_CHECK(Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local).ok());

	document["entries"][0]["count"] = 0;
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "count");
	document["entries"][0]["count"] = 1;
	document["entries"][0]["kind"] = "other";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "count");
	document["entries"][0]["kind"] = "dir";
	document["entries"][0]["count"] = "1";
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "count");

	document = Json::parse(kLocalManifest);
	document["entries"][0]["count"] = 1;
	CheckLocalSchema(document, Jeiobs::Workspace::ErrorCode::Schema, "count");

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
	JW_CHECK(json.strings[0].pointer.empty());
	JW_CHECK(json.strings[1].pointer.empty());
	JW_CHECK(json.strings[2].pointer.empty());
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
	JW_CHECK(ini.keys[0].pointer.empty());
	JW_CHECK(ini.keys[0].section.empty());
	JW_CHECK(ini.keys[0].key.empty());
	JW_CHECK(ini.keys[0].value_start < ini.keys[0].value_end);
	JW_CHECK(ini.keys[0].value_end - ini.keys[0].value_start == 7U);

	const auto ini_pointer = Jeiobs::Workspace::IniScan::Scan("[a/b~c]\nkey=value\n");
	JW_CHECK(ini_pointer.ok());
	JW_CHECK(ini_pointer.keys[0].pointer.empty());
	JW_CHECK(ini_pointer.keys[0].section.empty());
	JW_CHECK(ini_pointer.keys[0].key.empty());
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

void TestReviewR3()
{
	const auto bom_json = Jeiobs::Workspace::JsonScan::Scan("\xef\xbb\xbf{\"number\":1.5}");
	JW_CHECK(bom_json.ok());
	JW_CHECK(bom_json.values.front().start == 3U);

	const auto modules = Jeiobs::Workspace::Redact("app.modules", "[{\"name\":\"module\"}]");
	JW_CHECK(modules.ok());
	JW_CHECK(modules.report.fidelity == "exact");
	JW_CHECK(modules.bytes == "[{\"name\":\"module\"}]");

	const auto srt = Jeiobs::Workspace::Redact(
		"profile.service",
		R"json({"type":"rtmp_common","settings":{"server":"srt://h:9?streamid=#!::r=x,s=SECRET1&passphrase=SECRET2"}})json");
	JW_CHECK(srt.ok());
	JW_CHECK(srt.bytes.find("SECRET1") == std::string::npos);
	JW_CHECK(srt.bytes.find("SECRET2") == std::string::npos);

	const auto nested_url = Jeiobs::Workspace::Redact(
		"profile.service",
		R"json({"type":"rtmp_common","settings":{"server":"https://h/?token=https://u:p@x/"}})json");
	JW_CHECK(nested_url.ok());
	JW_CHECK(nested_url.bytes.find("u:p") == std::string::npos);
	JW_CHECK(nested_url.bytes.find("https://u:p@x/") == std::string::npos);

	const auto ffmpeg_options = Jeiobs::Workspace::Redact(
		"collection",
		R"json({"sources":[{"id":"ffmpeg_source","settings":{"ffmpeg_options":"http_proxy='http://u:pa ss@proxy.example:3128'"}}]})json");
	JW_CHECK(ffmpeg_options.ok());
	JW_CHECK(ffmpeg_options.bytes.find("u:pa ss") == std::string::npos);

	const auto local_input = Jeiobs::Workspace::Redact(
		"collection",
		R"json({"sources":[{"id":"ffmpeg_source","settings":{"is_local_file":true,"input":"rtmp://u:p@host/live?key=SECRET"}}]})json");
	JW_CHECK(local_input.ok());
	JW_CHECK(local_input.bytes.find("SECRET") == std::string::npos);

	const auto versioned_source = Jeiobs::Workspace::Redact(
		"collection",
		R"json({"sources":[{"id":"scene","versioned_id":"browser_source","settings":{"url":"https://u:p@host/?token=SECRET"}}]})json");
	JW_CHECK(versioned_source.ok());
	JW_CHECK(versioned_source.bytes.find("u:p") == std::string::npos);
	JW_CHECK(versioned_source.bytes.find("SECRET") == std::string::npos);

	const auto additional_sources = Jeiobs::Workspace::Redact(
		"collection",
		R"json({"groups":[{"id":"browser_source","settings":{"url":"https://u:p@group/?token=GROUP"}}],"DesktopAudioDevice1":{"id":"ffmpeg_source","settings":{"input":"rtmp://u:p@audio/live?key=AUDIO"}},"AuxAudioDevice4":{"id":"vlc_source","settings":{"playlist":[{"value":"srt://u:p@vlc:9?streamid=VLC"}]}}})json");
	JW_CHECK(additional_sources.ok());
	JW_CHECK(additional_sources.bytes.find("u:p@") == std::string::npos);
	JW_CHECK(additional_sources.bytes.find("GROUP") == std::string::npos);
	JW_CHECK(additional_sources.bytes.find("AUDIO") == std::string::npos);
	JW_CHECK(additional_sources.bytes.find("VLC") == std::string::npos);

	const std::vector<std::string> ini_inputs = {
		"[Twitch] ;x\nToken=S\n", "[Twitch\nToken=S\n",      "[Twitch]\rToken=S\n",    "[Twitch]\n\rToken=S\n",
		"[Twitch]]\nToken=S\n",   "[Twitch]\n[]\nToken=S\n", "Token=S\n[Twitch] ;x\n", "[\nToken=S\n",
		"[]\nToken=S\n",          "[ ]\nToken=S\n",
	};
	for (const std::string &input : ini_inputs) {
		const auto result = Jeiobs::Workspace::Redact("profile.basic", input);
		JW_CHECK(result.ok());
		JW_CHECK(result.bytes.find("Token=S") == std::string::npos);
		JW_CHECK(!result.report.redactions.empty());
	}

	const std::string long_section_input = "[Twitch" + std::string(1019U, 'x') + "]\nToken=S\n";
	const auto long_section = Jeiobs::Workspace::Redact("profile.basic", long_section_input);
	JW_CHECK(long_section.ok());
	JW_CHECK(long_section.report.fidelity == "opaque-unreviewed");
	JW_CHECK(long_section.bytes.find("Token=S") == std::string::npos);
	JW_CHECK(long_section.report.redactions.size() == 1U);
	JW_CHECK_EQ(long_section.report.redactions[0].pointer, "/~section-0/Token");

	std::string oversized_report_url = "https://host/?";
	for (std::size_t index = 0U; index < 100000U; ++index) {
		oversized_report_url += "token=x&";
	}
	const std::string oversized_report_document =
		R"json({"type":"rtmp_common","settings":{"server":")json" + oversized_report_url + R"json("}})json";
	const auto oversized_report = Jeiobs::Workspace::Redact("profile.service", oversized_report_document);
	JW_CHECK(oversized_report.ok());
	JW_CHECK(oversized_report.report.outcome == Jeiobs::Workspace::RedactOutcome::Omitted);
	JW_CHECK(oversized_report.report.reason == "unparseable");
	JW_CHECK(oversized_report.bytes.empty());

	const char *previous_locale = std::setlocale(LC_NUMERIC, nullptr);
	const std::string saved_locale = previous_locale == nullptr ? "C" : previous_locale;
	const char *comma_locale = std::setlocale(LC_NUMERIC, "de_DE.UTF-8");
	if (comma_locale == nullptr) {
		comma_locale = std::setlocale(LC_NUMERIC, "de_DE");
	}
	const auto decimal = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1.5e300}");
	JW_CHECK(decimal.ok());
	const auto locale_underflow = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1e-9999999}");
	JW_CHECK(locale_underflow.ok());
	if (comma_locale != nullptr) {
		JW_CHECK(std::setlocale(LC_NUMERIC, saved_locale.c_str()) != nullptr);
	}

	Json export_document = Json::parse(kLocalManifest);
	export_document["target"]["audio"] = {{"sample_rate", 48000}, {"channels", "Stereo"}, {"explicit", "yes"}};
	const auto audio_error = Jeiobs::Workspace::ParseManifest(export_document.dump(), ManifestMode::Export);
	JW_CHECK(audio_error.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK(audio_error.error.detail == "target.audio");
}

void TestPartBNumbers()
{
	const auto near_double = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1.5e300}");
	JW_CHECK(near_double.ok());
	const auto underflow = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1e-9999999}");
	JW_CHECK(underflow.ok());
	const auto oversized = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1" + std::string(4096U, '0') + "}");
	JW_CHECK(oversized.error.code == Jeiobs::Workspace::ErrorCode::Json);
	const auto long_exponent = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1e9999999}");
	JW_CHECK(long_exponent.error.code == Jeiobs::Workspace::ErrorCode::Nonfinite);
	const auto negative_long_exponent = Jeiobs::Workspace::JsonScan::Scan("{\"number\":1e-9999999}");
	JW_CHECK(negative_long_exponent.ok());
	const auto manifest_underflow = Jeiobs::Workspace::ParseJson("{\"number\":1e-9999999}");
	JW_CHECK(manifest_underflow.ok());
	const auto manifest_oversized = Jeiobs::Workspace::ParseJson("{\"number\":1" + std::string(4096U, '0') + "}");
	JW_CHECK(manifest_oversized.error.code == Jeiobs::Workspace::ErrorCode::Json);
}

void TestSceneUuidValidationAndExport()
{
	Json document = Json::parse(kLocalManifest);
	document["entries"][0]["root"] = "scenes";
	document["entries"][0]["path"] = "basic/scenes/Main.json";
	document["entries"][0]["role"] = "collection";
	document["entries"][0]["scene_uuids"] = {"scene-a", "scene-b"};
	const auto parsed = Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local);
	JW_CHECK(parsed.ok());
	const auto exported = Jeiobs::Workspace::ProjectForExport(*parsed.value, std::string(64, 'a'), Json::object(),
								  "2026-11-17T00:00:00Z");
	JW_CHECK(exported.ok());
	JW_CHECK(!exported.value->at("entries")[0].contains("scene_uuids"));

	document["entries"][0]["scene_uuids"] = {"scene-b", "scene-a"};
	const auto unsorted = Jeiobs::Workspace::ParseManifest(document.dump(), ManifestMode::Local);
	JW_CHECK(unsorted.error.code == Jeiobs::Workspace::ErrorCode::Schema);
	JW_CHECK_EQ(unsorted.error.detail, "scene_uuids");
}

void TestReviewRules()
{
	const auto scene_and_group = Jeiobs::Workspace::ReviewRules::AnalyzeCollection(
		R"json({"sources":[{"id":"scene"},{"id":"group"}]})json");
	JW_CHECK(scene_and_group.ok());
	JW_CHECK_EQ(scene_and_group.unreviewed_count, 1U);
	const auto opaque = Jeiobs::Workspace::ReviewRules::AnalyzeCollection(
		R"json({"sources":[{"id":"scene"},{"id":"third-party"},{"id":"another-third-party"}]})json");
	JW_CHECK(opaque.ok());
	JW_CHECK_EQ(opaque.unreviewed_count, 2U);
}

const Jeiobs::Workspace::PlannedEntry *FindPlanEntry(const Jeiobs::Workspace::Plan &plan, std::string_view root,
						     std::string_view path)
{
	for (const auto &entry : plan.entries) {
		if (entry.root == root && entry.path == path) {
			return &entry;
		}
	}
	return nullptr;
}

Jeiobs::Workspace::Roots TestRoots(std::string_view prefix)
{
	Jeiobs::Workspace::Roots roots;
	const std::string root = prefix.empty() ? std::string{} : std::string(prefix) + "/";
	roots.config.path = root + "config";
	roots.user.path = root + "user";
	roots.scenes.path = root + "scenes";
	roots.profiles.path = root + "profiles";
	roots.plugin_manager.path = root + "plugin_manager";
	return roots;
}

class PartialListingFs final : public Jeiobs::Workspace::Fs {
public:
	Jeiobs::Workspace::MemFs storage;
	std::string failing_directory;

	[[nodiscard]] Jeiobs::Workspace::FsListResult List(std::string_view directory) const override
	{
		if (directory == failing_directory) {
			Jeiobs::Workspace::FsListResult result = storage.List(directory);
			result.error = {Jeiobs::Workspace::ErrorCode::Io, "listing"};
			return result;
		}
		return storage.List(directory);
	}

	[[nodiscard]] Jeiobs::Workspace::BytesResult ReadBounded(std::string_view path,
								 std::size_t max_bytes) const override
	{
		return storage.ReadBounded(path, max_bytes);
	}

	[[nodiscard]] Jeiobs::Workspace::Error
	ListEach(std::string_view directory, const Jeiobs::Workspace::FsListCallback &callback) const override
	{
		if (directory != failing_directory) {
			return storage.ListEach(directory, callback);
		}
		const auto listed = storage.List(directory);
		if (!listed.ok()) {
			return listed.error;
		}
		for (const auto &entry : listed.entries) {
			if (!callback(entry)) {
				return {};
			}
		}
		return {Jeiobs::Workspace::ErrorCode::Io, "listing"};
	}
};

class SizedFs final : public Jeiobs::Workspace::Fs {
	struct Node {
		Jeiobs::Workspace::FsEntryKind kind = Jeiobs::Workspace::FsEntryKind::Directory;
		std::uint64_t size = 0U;
		std::string bytes;
	};

	std::map<std::string, Node> nodes_;

	void AddParents(std::string_view path)
	{
		std::size_t slash = path.find('/');
		while (slash != std::string_view::npos) {
			nodes_.try_emplace(std::string(path.substr(0U, slash)), Node{});
			slash = path.find('/', slash + 1U);
		}
		nodes_.try_emplace("", Node{});
	}

public:
	SizedFs() { nodes_.emplace("", Node{}); }

	void AddDirectory(std::string_view path)
	{
		AddParents(path);
		nodes_[std::string(path)] = Node{};
	}

	void AddFile(std::string_view path, std::uint64_t size)
	{
		AddParents(path);
		nodes_[std::string(path)] = {Jeiobs::Workspace::FsEntryKind::File, size, {}};
	}

	[[nodiscard]] Jeiobs::Workspace::FsListResult List(std::string_view directory) const override
	{
		const auto directory_iterator = nodes_.find(std::string(directory));
		if (directory_iterator == nodes_.end()) {
			return {{}, {Jeiobs::Workspace::ErrorCode::Io, "missing"}};
		}
		if (directory_iterator->second.kind != Jeiobs::Workspace::FsEntryKind::Directory) {
			return {{}, {Jeiobs::Workspace::ErrorCode::Io, "not-directory"}};
		}

		const std::string prefix = directory.empty() ? std::string{} : std::string(directory) + '/';
		std::map<std::string, Jeiobs::Workspace::FsEntry> children;
		for (const auto &[path, node] : nodes_) {
			if (path.size() <= prefix.size() || path.compare(0U, prefix.size(), prefix) != 0) {
				continue;
			}
			const std::string_view remainder(path.data() + prefix.size(), path.size() - prefix.size());
			const std::size_t slash = remainder.find('/');
			if (slash != std::string_view::npos) {
				continue;
			}
			children.emplace(std::string(remainder),
					 Jeiobs::Workspace::FsEntry{std::string(remainder), node.kind, node.size,
								    true});
		}

		Jeiobs::Workspace::FsListResult result;
		for (const auto &[name, entry] : children) {
			(void)name;
			result.entries.push_back(entry);
		}
		return result;
	}

	[[nodiscard]] Jeiobs::Workspace::BytesResult ReadBounded(std::string_view path,
								 std::size_t max_bytes) const override
	{
		const auto iterator = nodes_.find(std::string(path));
		if (iterator == nodes_.end() || iterator->second.kind != Jeiobs::Workspace::FsEntryKind::File) {
			return {{}, {Jeiobs::Workspace::ErrorCode::Io, {}}};
		}
		if (iterator->second.size > max_bytes) {
			return {{}, {Jeiobs::Workspace::ErrorCode::Limit, "blob"}};
		}
		return {iterator->second.bytes, {}};
	}

	[[nodiscard]] Jeiobs::Workspace::Error
	ListEach(std::string_view directory, const Jeiobs::Workspace::FsListCallback &callback) const override
	{
		const auto listed = List(directory);
		if (!listed.ok()) {
			return listed.error;
		}
		for (const auto &entry : listed.entries) {
			if (!callback(entry)) {
				break;
			}
		}
		return {};
	}
};

void TestPlannerR4()
{
	using Jeiobs::Workspace::CapturePlanner;
	using Jeiobs::Workspace::SeededUidSource;

	const auto roots = TestRoots("");

	PartialListingFs partial;
	partial.failing_directory = "scenes/obs-studio/basic/scenes/bad?";
	const std::string partial_relative = "basic/scenes/bad?";
	JW_CHECK(partial.storage.AddFile(partial.failing_directory + "/partial.txt", "partial").ok());
	SeededUidSource partial_uid("partial");
	const auto partial_result = CapturePlanner::Plan(partial, roots, nullptr, partial_uid);
	JW_CHECK(partial_result.ok());
	if (partial_result.ok()) {
		const auto names = FindPlanEntry(*partial_result.value, "scenes", "basic/scenes");
		JW_CHECK(names != nullptr);
		if (names != nullptr) {
			JW_CHECK_EQ(names->reason, "unrepresentable");
			JW_CHECK_EQ(names->count, 1U);
			JW_CHECK_EQ(names->size, 0U);
		}
		// The folder's own name is unrepresentable: it is never recorded as an entry or in a warning,
		// the warning names the containing folder.
		JW_CHECK(FindPlanEntry(*partial_result.value, "scenes", partial_relative) == nullptr);
		JW_CHECK(std::any_of(partial_result.value->warnings.begin(), partial_result.value->warnings.end(),
				     [](const auto &warning) {
					     return warning.code == "listing-error" && warning.path == "basic/scenes";
				     }));
		JW_CHECK(std::none_of(partial_result.value->warnings.begin(), partial_result.value->warnings.end(),
				      [](const auto &warning) { return warning.path == "basic/scenes/bad?"; }));
	}

	Jeiobs::Workspace::MemFs owned;
	JW_CHECK(owned.AddFile("user/obs-studio/notes?.txt", "ignored").ok());
	JW_CHECK(owned.AddFile("scenes/obs-studio/basic/odd?/ignored.txt", "ignored").ok());
	SeededUidSource owned_uid("owned");
	const auto owned_result = CapturePlanner::Plan(owned, roots, nullptr, owned_uid);
	JW_CHECK(owned_result.ok());
	if (owned_result.ok()) {
		JW_CHECK(FindPlanEntry(*owned_result.value, "user", "obs-studio") == nullptr);
		JW_CHECK(FindPlanEntry(*owned_result.value, "scenes", "obs-studio") == nullptr);
		JW_CHECK(FindPlanEntry(*owned_result.value, "scenes", "basic") == nullptr);
	}

	Jeiobs::Workspace::MemFs empty_directories;
	for (std::size_t index = 0U; index < 150U; ++index) {
		JW_CHECK(empty_directories.AddDirectory("scenes/obs-studio/basic/scenes/empty" + std::to_string(index))
				 .ok());
	}
	SeededUidSource empty_uid("empty");
	const auto empty_result = CapturePlanner::Plan(empty_directories, roots, nullptr, empty_uid);
	JW_CHECK(empty_result.ok());
	if (empty_result.ok()) {
		std::size_t unknown_directories = 0U;
		for (const auto &entry : empty_result.value->entries) {
			if (entry.root == "scenes" && entry.reason == "unknown" && entry.kind == "dir") {
				++unknown_directories;
			}
		}
		JW_CHECK_EQ(unknown_directories, 150U);
	}

	SizedFs sized;
	const std::uint64_t sixty_four_mib = Jeiobs::Workspace::Limits::kBlobBytes;
	for (const char name : {'A', 'B', 'C', 'D', 'E'}) {
		sized.AddFile(std::string("scenes/obs-studio/basic/scenes/") + name + ".json", sixty_four_mib);
	}
	sized.AddFile("config/obs-studio/plugin_config/obs-websocket/config.json", 1U);
	SeededUidSource sized_uid("sized");
	const auto sized_result = CapturePlanner::Plan(sized, roots, nullptr, sized_uid);
	JW_CHECK(sized_result.ok());
	if (sized_result.ok()) {
		for (const char name : {'A', 'B', 'C', 'D'}) {
			const auto entry = FindPlanEntry(*sized_result.value, "scenes",
							 std::string("basic/scenes/") + name + ".json");
			JW_CHECK(entry != nullptr);
			if (entry != nullptr) {
				JW_CHECK_EQ(entry->status, "stored");
			}
		}
		const auto e_entry = FindPlanEntry(*sized_result.value, "scenes", "basic/scenes/E.json");
		const auto module_entry =
			FindPlanEntry(*sized_result.value, "config", "plugin_config/obs-websocket/config.json");
		JW_CHECK(e_entry != nullptr);
		JW_CHECK(module_entry != nullptr);
		if (e_entry != nullptr) {
			JW_CHECK_EQ(e_entry->reason, "too-large");
		}
		if (module_entry != nullptr) {
			JW_CHECK_EQ(module_entry->reason, "too-large");
		}
		JW_CHECK(sized_result.value->entries.size() >= 4U);
		for (std::size_t index = 0U; index < 4U && index < sized_result.value->entries.size(); ++index) {
			JW_CHECK_EQ(sized_result.value->entries[index].path,
				    std::string("basic/scenes/") + static_cast<char>('A' + index) + ".json");
		}
	}
}

void TestPlannerR2()
{
	using Jeiobs::Workspace::CapturePlanner;
	using Jeiobs::Workspace::MemFs;
	using Jeiobs::Workspace::SeededUidSource;

	MemFs fs;
	JW_CHECK(fs.AddFile("scenes/obs-studio/basic/scenes/Changed.json",
			    R"json({"sources":[{"id":"scene","uuid":"new"}]})json")
			 .ok());
	JW_CHECK(fs.AddFile("scenes/obs-studio/basic/scenes/Archive.json",
			    R"json({"sources":[{"id":"scene","uuid":"old"}]})json")
			 .ok());
	JW_CHECK(fs.AddFile("scenes/obs-studio/basic/scenes/themes.json", "{}").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/themes/basic.ini", "x=y\n").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/NoBasic/service.json", "{}").ok());
	JW_CHECK(fs.AddSymlink("config/obs-studio").ok());

	Json parent_document = Json::parse(kLocalManifest);
	parent_document["entries"] = {{{"id", "old-collection"},
				       {"status", "stored"},
				       {"root", "scenes"},
				       {"path", "basic/scenes/Changed.json"},
				       {"role", "collection"},
				       {"collection", "collection-old"},
				       {"size", 1},
				       {"hash", std::string(64, 'a')},
				       {"fidelity", "exact"},
				       {"scene_uuids", {"old"}}}};
	const auto parent = Jeiobs::Workspace::ParseManifest(parent_document.dump(), ManifestMode::Local);
	JW_CHECK(parent.ok());
	if (!parent.ok()) {
		return;
	}

	const auto roots = TestRoots("");
	SeededUidSource uid_source("r2");
	const auto result = CapturePlanner::Plan(fs, roots, &*parent.value, uid_source);
	JW_CHECK(result.ok());
	if (!result.ok()) {
		return;
	}
	JW_CHECK_EQ(result.value->collections.size(), 3U);
	const auto changed = FindPlanEntry(*result.value, "scenes", "basic/scenes/Changed.json");
	const auto archive = FindPlanEntry(*result.value, "scenes", "basic/scenes/Archive.json");
	JW_CHECK(changed != nullptr);
	JW_CHECK(archive != nullptr);
	if (changed != nullptr && archive != nullptr) {
		JW_CHECK_EQ(changed->collection.value(), "collection-old");
		JW_CHECK_EQ(archive->collection.value(), "uid-r2-0001");
	}
	JW_CHECK(FindPlanEntry(*result.value, "scenes", "basic/scenes/themes.json") != nullptr);
	JW_CHECK(FindPlanEntry(*result.value, "profiles", "basic/profiles/themes/basic.ini") != nullptr);
	JW_CHECK(result.value->profiles.size() == 1U);
	const auto no_basic = FindPlanEntry(*result.value, "profiles", "basic/profiles/NoBasic");
	JW_CHECK(no_basic != nullptr);
	if (no_basic != nullptr) {
		JW_CHECK_EQ(no_basic->reason, "unknown");
		JW_CHECK_EQ(no_basic->kind, "dir");
	}
	const auto app_symlink = FindPlanEntry(*result.value, "config", "obs-studio");
	JW_CHECK(app_symlink != nullptr);
	if (app_symlink != nullptr) {
		JW_CHECK_EQ(app_symlink->reason, "symlink");
	}
	JW_CHECK(std::any_of(result.value->warnings.begin(), result.value->warnings.end(), [](const auto &warning) {
		return warning.code == "root-symlink" && warning.path == "obs-studio";
	}));

	MemFs scene_limit;
	std::string scene_document = R"json({"sources":[)json";
	for (std::size_t index = 0U; index < 100001U; ++index) {
		if (index != 0U) {
			scene_document.push_back(',');
		}
		scene_document += "{\"id\":\"scene\",\"uuid\":\"u" + std::to_string(index) + "\"}";
	}
	scene_document += "]}";
	JW_CHECK(scene_limit.AddFile("scenes/obs-studio/basic/scenes/Limited.json", scene_document).ok());
	SeededUidSource scene_uid("scene");
	const auto scene_result = CapturePlanner::Plan(scene_limit, roots, nullptr, scene_uid);
	JW_CHECK(scene_result.ok());
	if (scene_result.ok()) {
		JW_CHECK(scene_result.value->collections.size() == 1U);
		if (scene_result.value->collections.size() == 1U) {
			JW_CHECK(scene_result.value->collections[0].scene_uuids.empty());
		}
		JW_CHECK(std::any_of(scene_result.value->warnings.begin(), scene_result.value->warnings.end(),
				     [](const auto &warning) { return warning.code == "scene-uuids-limit"; }));
	}

	MemFs too_large;
	const std::string large(Jeiobs::Workspace::Limits::kBlobBytes + 1U, 'x');
	JW_CHECK(too_large.AddFile("scenes/obs-studio/basic/scenes/Large.json", large).ok());
	SeededUidSource large_uid("large");
	const auto large_result = CapturePlanner::Plan(too_large, roots, nullptr, large_uid);
	JW_CHECK(large_result.ok());
	if (large_result.ok()) {
		const auto omitted = FindPlanEntry(*large_result.value, "scenes", "basic/scenes/Large.json");
		JW_CHECK(omitted != nullptr);
		if (omitted != nullptr) {
			JW_CHECK_EQ(omitted->reason, "too-large");
		}
		JW_CHECK(large_result.value->collections.empty());
	}
}

#if !defined(_WIN32)
void TestRealFsR2Failures()
{
	char directory_template[] = "/tmp/jeiobs-workspace-r2-XXXXXX";
	char *directory_name = mkdtemp(directory_template);
	JW_CHECK(directory_name != nullptr);
	if (directory_name == nullptr) {
		return;
	}
	const std::filesystem::path root(directory_name);
	std::error_code filesystem_error;
	std::filesystem::create_directories(root / "config" / "obs-studio" / "plugin_config", filesystem_error);
	std::filesystem::create_directories(root / "user", filesystem_error);
	std::filesystem::create_directories(root / "outside", filesystem_error);
	std::filesystem::create_symlink("../outside", root / "user" / "obs-studio", filesystem_error);
	JW_CHECK(!filesystem_error);

	Jeiobs::Workspace::RealFs fs;
	const auto roots = TestRoots(root.string());
	Jeiobs::Workspace::SeededUidSource source("real");
	const auto symlink_result = Jeiobs::Workspace::CapturePlanner::Plan(fs, roots, nullptr, source);
	JW_CHECK(symlink_result.ok());
	if (symlink_result.ok()) {
		const auto entry = FindPlanEntry(*symlink_result.value, "user", "obs-studio");
		JW_CHECK(entry != nullptr);
		if (entry != nullptr) {
			JW_CHECK_EQ(entry->reason, "symlink");
		}
	}

	if (geteuid() != 0) {
		const std::filesystem::path protected_directory = root / "config" / "obs-studio" / "plugin_config";
		JW_CHECK(chmod(protected_directory.c_str(), 0) == 0);
		Jeiobs::Workspace::SeededUidSource protected_source("protected");
		const auto protected_result =
			Jeiobs::Workspace::CapturePlanner::Plan(fs, roots, nullptr, protected_source);
		JW_CHECK(protected_result.ok());
		if (protected_result.ok()) {
			const auto entry = FindPlanEntry(*protected_result.value, "config", "plugin_config");
			JW_CHECK(entry != nullptr);
			if (entry != nullptr) {
				JW_CHECK_EQ(entry->reason, "unreadable");
			}
			JW_CHECK(std::any_of(protected_result.value->warnings.begin(),
					     protected_result.value->warnings.end(), [](const auto &warning) {
						     return warning.code == "listing-error" &&
							    warning.path == "plugin_config";
					     }));
		}
		JW_CHECK(chmod(protected_directory.c_str(), 0700) == 0);
	}
	std::filesystem::remove_all(root, filesystem_error);
}
#endif

void TestMemFsAndPlanner()
{
	Jeiobs::Workspace::MemFs fs;
	JW_CHECK(fs.AddFile("config/obs-studio/plugin_config/obs-websocket/config.json", "{}").ok());
	JW_CHECK(fs.AddFile("user/obs-studio/user.ini", "x=y\n").ok());
	JW_CHECK(fs.AddFile("plugin_manager/obs-studio/plugin_manager/modules.json", "[]").ok());
	JW_CHECK(fs.AddFile("scenes/obs-studio/basic/scenes/Main.json",
			    R"json({"sources":[{"id":"scene","uuid":"scene-b"},{"id":"scene","uuid":"scene-a"}]})json")
			 .ok());
	JW_CHECK(fs.AddFile("scenes/obs-studio/basic/scenes/Main.json.bak", "{}").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/Main/basic.ini", "x=y\n").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/Main/service.json", "{}").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/Main/service.json.bak", "{}").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/main/basic.ini", "x=y\n").ok());
	JW_CHECK(fs.AddFile("profiles/obs-studio/basic/profiles/Main/unknown.bin", "unknown").ok());
	JW_CHECK(fs.AddSymlink("config/obs-studio/plugin_config/obs-websocket/link.json").ok());

	Jeiobs::Workspace::Roots roots;
	roots.config.path = "config";
	roots.user.path = "user";
	roots.scenes.path = "scenes";
	roots.profiles.path = "profiles";
	roots.plugin_manager.path = "plugin_manager";
	Jeiobs::Workspace::SeededUidSource source("42");
	const auto planned = Jeiobs::Workspace::CapturePlanner::Plan(fs, roots, nullptr, source);
	JW_CHECK(planned.ok());
	if (planned.ok()) {
		JW_CHECK_EQ(planned.value->entries.size(), 11U);
		if (planned.value->entries.size() >= 2U) {
			JW_CHECK_EQ(planned.value->entries[0].path, "user.ini");
			JW_CHECK_EQ(planned.value->entries[1].path, "plugin_manager/modules.json");
		}
		JW_CHECK_EQ(planned.value->collections.size(), 1U);
		if (planned.value->collections.size() == 1U) {
			JW_CHECK_EQ(planned.value->collections[0].scene_uuids[0], "scene-a");
			JW_CHECK_EQ(planned.value->collections[0].scene_uuids[1], "scene-b");
		}
		JW_CHECK_EQ(planned.value->profiles.size(), 2U);
		if (planned.value->profiles.size() >= 1U) {
			JW_CHECK_EQ(planned.value->profiles[0].uid, "uid-42-0002");
		}
		JW_CHECK(!planned.value->warnings.empty());
		if (!planned.value->warnings.empty()) {
			JW_CHECK_EQ(planned.value->warnings.back().code, "case-collision");
		}

		const auto plan_json = Jeiobs::Workspace::PlanToJson(*planned.value);
		if (plan_json.at("entries").size() >= 3U) {
			JW_CHECK(plan_json.at("entries")[2].at("fidelity").is_null());
			JW_CHECK(plan_json.at("collections")[0].at("scene_uuids").is_array());
		}

		Json parent_document = Json::parse(kLocalManifest);
		parent_document["entries"] = {{{"id", "old-collection"},
					       {"status", "stored"},
					       {"root", "scenes"},
					       {"path", "basic/scenes/Main.json"},
					       {"role", "collection"},
					       {"collection", "collection-old"},
					       {"size", 1},
					       {"hash", std::string(64, 'a')},
					       {"fidelity", "exact"},
					       {"scene_uuids", {"scene-a", "scene-b"}}},
					      {{"id", "old-profile"},
					       {"status", "stored"},
					       {"root", "profiles"},
					       {"path", "basic/profiles/Main/basic.ini"},
					       {"role", "profile.basic"},
					       {"profile", "profile-old"},
					       {"size", 1},
					       {"hash", std::string(64, 'b')},
					       {"fidelity", "exact"}}};
		const auto parent = Jeiobs::Workspace::ParseManifest(parent_document.dump(), ManifestMode::Local);
		JW_CHECK(parent.ok());
		if (parent.ok()) {
			Jeiobs::Workspace::SeededUidSource parent_source("90");
			const auto matched =
				Jeiobs::Workspace::CapturePlanner::Plan(fs, roots, &*parent.value, parent_source);
			JW_CHECK(matched.ok());
			if (matched.ok()) {
				JW_CHECK_EQ(matched.value->collections[0].uid, "collection-old");
				JW_CHECK_EQ(matched.value->profiles[0].uid, "profile-old");
				JW_CHECK_EQ(matched.value->profiles[1].uid, "uid-90-0001");
			}
		}
	}
}

void TestBoundedWorstCases()
{
	const auto start = std::chrono::steady_clock::now();

	const std::string long_key(4096U, 'k');
	std::string array_document = "{\"" + long_key + "\":[";
	for (std::size_t index = 0U; index < 100000U; ++index) {
		if (index != 0U) {
			array_document.push_back(',');
		}
		array_document.push_back('0');
	}
	array_document += "]}";
	const auto array_scan = Jeiobs::Workspace::JsonScan::Scan(array_document);
	JW_CHECK(array_scan.ok());

	std::string ini_document = "[" + std::string(Jeiobs::Workspace::Limits::kIniLineBytes - 2U, 's') + "]\n";
	for (std::size_t index = 0U; index < 100000U; ++index) {
		ini_document += "key" + std::to_string(index) + "=value\n";
	}
	const auto ini_scan = Jeiobs::Workspace::IniScan::Scan(ini_document);
	JW_CHECK(ini_scan.ok());
	JW_CHECK_EQ(ini_scan.keys.size(), 100000U);
	JW_CHECK(ini_scan.keys.front().section.empty());
	JW_CHECK(ini_scan.keys.front().pointer.empty());

	std::string collection = "{\"sources\":[";
	for (std::size_t index = 0U; index < 10000U; ++index) {
		if (index != 0U) {
			collection.push_back(',');
		}
		collection +=
			"{\"id\":\"browser_source\",\"settings\":{\"url\":\"https://u:p@example.test/live?token=SECRET";
		collection += std::to_string(index);
		collection += "\"}}";
	}
	collection += "]}";
	const auto collection_result = Jeiobs::Workspace::Redact("collection", collection);
	JW_CHECK(collection_result.ok());
	JW_CHECK(collection_result.bytes.find("SECRET") == std::string::npos);

	const auto elapsed = std::chrono::steady_clock::now() - start;
	JW_CHECK(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() < 5);
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
	TestReviewR3();
	TestPartBNumbers();
	TestSceneUuidValidationAndExport();
	TestReviewRules();
	TestPlannerR4();
	TestMemFsAndPlanner();
	TestPlannerR2();
#if !defined(_WIN32)
	TestRealFsR2Failures();
#endif
	TestBoundedWorstCases();
	return JwTest::failures == 0 ? 0 : 1;
}
