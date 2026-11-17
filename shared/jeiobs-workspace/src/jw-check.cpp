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
#include "Utf8Path.hpp"
#include "jeiobs/workspace/CapturePlanner.hpp"
#include "jeiobs/workspace/Hash.hpp"
#include "jeiobs/workspace/Manifest.hpp"
#include "jeiobs/workspace/PathRules.hpp"
#include "jeiobs/workspace/Redactor.hpp"
#include "jeiobs/workspace/Unicode.hpp"

#include "jeiobs/workspace/Limits.hpp"

#include <filesystem>
#include <fstream>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace {

using Jeiobs::Workspace::Error;
using Jeiobs::Workspace::ErrorCode;
using Jeiobs::Workspace::BytesResult;
using Jeiobs::Workspace::CapturePlanner;
using Jeiobs::Workspace::FsListResult;
using Jeiobs::Workspace::Json;
using Jeiobs::Workspace::Manifest;
using Jeiobs::Workspace::ManifestMode;
using Jeiobs::Workspace::ManifestResult;
using Jeiobs::Workspace::PlanToJson;
using Jeiobs::Workspace::PlanResult;
using Jeiobs::Workspace::RedactOptions;
using Jeiobs::Workspace::RedactOutcome;
using Jeiobs::Workspace::RedactResult;
using Jeiobs::Workspace::RealFs;
using Jeiobs::Workspace::Roots;
using Jeiobs::Workspace::SeededUidSource;
namespace Limits = Jeiobs::Workspace::Limits;

void PrintUsage();

void PrintError(const Error &error)
{
	std::cout << "ERR " << Jeiobs::Workspace::ErrorCodeName(error.code);
	if (!error.detail.empty()) {
		std::cout << ' ' << error.detail;
	}
	std::cout << '\n';
}

void PrintManifestOk(const Manifest &manifest)
{
	std::cout << "OK";
	if (manifest.readonly) {
		std::cout << " readonly";
	}
	if (manifest.roCompat) {
		std::cout << " ro_compat";
	}
	std::cout << '\n';
}

bool ParseRevisionName(const std::string &path, std::uint64_t &sequence, std::string &revision_id)
{
	std::size_t separator = path.find_last_of("/\\");
	const std::string name = separator == std::string::npos ? path : path.substr(separator + 1U);
	if (name.size() != 80U || name[10] != '-' || name.substr(75U) != ".json") {
		return false;
	}
	for (std::size_t index = 0U; index < 10U; ++index) {
		if (name[index] < '0' || name[index] > '9') {
			return false;
		}
	}
	for (std::size_t index = 11U; index < 75U; ++index) {
		const char character = name[index];
		if (!((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'))) {
			return false;
		}
	}
	try {
		sequence = std::stoull(name.substr(0U, 10U));
	} catch (...) {
		return false;
	}
	revision_id = name.substr(11U, 64U);
	return true;
}

int CheckHash(const std::string &path)
{
	const auto result = Jeiobs::Workspace::HashFile(path);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	std::cout << result.value << '\n';
	return 0;
}

int CheckPath(const std::string &path)
{
	const auto result = Jeiobs::Workspace::ValidatePath(path);
	if (!result.ok()) {
		PrintError({Jeiobs::Workspace::IsPathLimitRule(result.rule) ? ErrorCode::Limit : ErrorCode::Path,
			    result.rule});
		return 1;
	}
	std::cout << "OK";
	std::cout << '\n';
	return 0;
}

int CheckUnicode(const std::string &function, const std::string &text)
{
	std::string out;
	bool ok = false;
	if (function == "nfd") {
		ok = Jeiobs::Workspace::ToNfd(text, out);
	} else if (function == "fold") {
		ok = Jeiobs::Workspace::ToCaseFold(text, out);
	} else {
		ok = Jeiobs::Workspace::CollisionKey(text, out);
	}
	if (!ok) {
		PrintError({ErrorCode::Path, "not-utf8"});
		return 1;
	}
	static const char digits[] = "0123456789abcdef";
	std::string hex;
	for (const char byte : out) {
		const unsigned char value = static_cast<unsigned char>(byte);
		hex.push_back(digits[value >> 4U]);
		hex.push_back(digits[value & 0x0FU]);
	}
	std::cout << "OK " << hex << '\n';
	return 0;
}

int CheckManifest(const std::string &path, ManifestMode mode)
{
	const auto result = Jeiobs::Workspace::LoadManifest(path, mode);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	PrintManifestOk(*result.value);
	return 0;
}

int CheckRevision(const std::string &path)
{
	std::uint64_t sequence = 0U;
	std::string revision_id;
	if (!ParseRevisionName(path, sequence, revision_id)) {
		PrintError({ErrorCode::Name, {}});
		return 1;
	}
	const auto bytes = Jeiobs::Workspace::ReadFile(path);
	if (!bytes.ok()) {
		PrintError(bytes.error);
		return 1;
	}
	const auto hash = Jeiobs::Workspace::HashBytes(bytes.bytes);
	if (!hash.ok()) {
		PrintError(hash.error);
		return 1;
	}
	if (hash.value != revision_id) {
		PrintError({ErrorCode::Revid, {}});
		return 1;
	}
	const auto result = Jeiobs::Workspace::ParseManifest(bytes.bytes, ManifestMode::Local);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const Json &manifest_sequence = result.value->document.at("seq");
	if (!manifest_sequence.is_number_integer() && !manifest_sequence.is_number_unsigned()) {
		PrintError({ErrorCode::Schema, "seq"});
		return 1;
	}
	std::uint64_t stored_sequence = 0U;
	try {
		stored_sequence = manifest_sequence.get<std::uint64_t>();
	} catch (...) {
		PrintError({ErrorCode::Schema, "seq"});
		return 1;
	}
	if (stored_sequence != sequence) {
		PrintError({ErrorCode::Seq, {}});
		return 1;
	}
	PrintManifestOk(*result.value);
	return 0;
}

int CheckChild(const std::string &parent_path, const std::string &draft_path)
{
	const auto parent = Jeiobs::Workspace::LoadManifest(parent_path, ManifestMode::Local);
	if (!parent.ok()) {
		PrintError(parent.error);
		return 1;
	}
	const auto draft = Jeiobs::Workspace::LoadManifest(draft_path, ManifestMode::Local);
	if (!draft.ok()) {
		PrintError(draft.error);
		return 1;
	}
	const auto result = Jeiobs::Workspace::CarryOver(*parent.value, *draft.value);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const std::string child_bytes = result.value->dump();
	const auto validated_child = Jeiobs::Workspace::ParseManifest(child_bytes, ManifestMode::Local);
	if (!validated_child.ok()) {
		PrintError(validated_child.error);
		return 1;
	}
	std::cout << child_bytes << '\n';
	return 0;
}

int CheckExport(const std::string &manifest_path, const std::string &decisions_path, const std::string &now)
{
	const auto manifest_bytes = Jeiobs::Workspace::ReadFile(manifest_path);
	if (!manifest_bytes.ok()) {
		PrintError(manifest_bytes.error);
		return 1;
	}
	const auto manifest_hash = Jeiobs::Workspace::HashBytes(manifest_bytes.bytes);
	if (!manifest_hash.ok()) {
		PrintError(manifest_hash.error);
		return 1;
	}
	const auto manifest = Jeiobs::Workspace::ParseManifest(manifest_bytes.bytes, ManifestMode::Local);
	if (!manifest.ok()) {
		PrintError(manifest.error);
		return 1;
	}
	const auto decisions_bytes = Jeiobs::Workspace::ReadFile(decisions_path);
	if (!decisions_bytes.ok()) {
		PrintError(decisions_bytes.error);
		return 1;
	}
	const auto decisions = Jeiobs::Workspace::ParseJson(decisions_bytes.bytes);
	if (!decisions.ok()) {
		PrintError(decisions.error);
		return 1;
	}
	const auto result =
		Jeiobs::Workspace::ProjectForExport(*manifest.value, manifest_hash.value, *decisions.value, now);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const std::string exported_bytes = result.value->dump();
	std::cout << exported_bytes << '\n';
	return 0;
}

Jeiobs::Workspace::BytesResult ReadBlob(const std::string &path)
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(path);
		std::error_code status_error;
		const std::filesystem::file_status status = std::filesystem::status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return {{}, {ErrorCode::Io, {}}};
		}
		std::error_code size_error;
		const std::uintmax_t file_size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error) {
			return {{}, {ErrorCode::Io, {}}};
		}
		if (file_size > Limits::kDocumentBytes ||
		    file_size > static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max())) {
			return {{}, {ErrorCode::Limit, "blob"}};
		}
		std::ifstream input;
		Jeiobs::Workspace::Detail::OpenUtf8(input, path);
		if (!input.is_open()) {
			return {{}, {ErrorCode::Io, {}}};
		}
		std::string bytes(static_cast<std::size_t>(file_size), '\0');
		if (!bytes.empty()) {
			input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
				return {{}, {ErrorCode::Io, {}}};
			}
		}
		return {std::move(bytes), {}};
	} catch (...) {
		return {{}, {ErrorCode::Io, {}}};
	}
}

bool WriteUtf8(const std::string &path, std::string_view bytes)
{
	try {
		std::ofstream output;
		Jeiobs::Workspace::Detail::OpenUtf8(output, path);
		if (!output.is_open()) {
			return false;
		}
		output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		return output.good();
	} catch (...) {
		return false;
	}
}

Json RedactReportJson(const Jeiobs::Workspace::RedactReport &report)
{
	Json value = Json::object();
	value["outcome"] = report.outcome == RedactOutcome::Stored ? "stored" : "omitted";
	if (report.outcome == RedactOutcome::Omitted) {
		value["reason"] = report.reason;
		return value;
	}
	value["fidelity"] = report.fidelity;
	if (report.parse_invalid) {
		value["parse"] = "invalid";
	}
	Json redactions = Json::array();
	for (const auto &redaction : report.redactions) {
		Json item = {
			{"pointer", redaction.pointer},
			{"kind", redaction.kind},
			{"start", redaction.start},
			{"end", redaction.end},
		};
		if (redaction.has_was) {
			item["was"] = redaction.was;
		}
		redactions.push_back(std::move(item));
	}
	value["redactions"] = std::move(redactions);
	if (report.unreviewed_count != 0U) {
		value["unreviewed"] = {{"rules", "u1"}, {"count", report.unreviewed_count}};
	}
	return value;
}

int CheckRedact(const std::string &role, const std::string &input_path, const std::string &output_path,
		const std::string &report_path, const RedactOptions &options)
{
	if (!Jeiobs::Workspace::IsRedactRole(role)) {
		PrintError({ErrorCode::Schema, "role"});
		return 1;
	}
	const auto input = ReadBlob(input_path);
	if (!input.ok()) {
		PrintError(input.error);
		return 1;
	}
	const RedactResult result = Jeiobs::Workspace::Redact(role, input.bytes, options);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const std::string report_bytes = RedactReportJson(result.report).dump();
	if (!WriteUtf8(report_path, report_bytes)) {
		PrintError({ErrorCode::Io, {}});
		return 1;
	}
	if (result.report.outcome == RedactOutcome::Stored && !WriteUtf8(output_path, result.bytes)) {
		PrintError({ErrorCode::Io, {}});
		return 1;
	}
	std::cout << "OK\n";
	return 0;
}

std::string JoinCliPath(const std::string &first, const std::string &second)
{
	try {
		return (std::filesystem::u8path(first) / std::filesystem::u8path(second)).u8string();
	} catch (...) {
		return {};
	}
}

bool ParseUint64(std::string_view text, std::uint64_t &value)
{
	if (text.empty()) {
		return false;
	}
	const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
	return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
}

bool IsRootRelative(std::string_view path)
{
	if (path.empty() || path.front() == '/' || path.front() == '\\' || path.find('\\') != std::string_view::npos) {
		return false;
	}
	const Jeiobs::Workspace::PathResult result = Jeiobs::Workspace::ValidatePath(path);
	return result.ok();
}

Error ReadRootsFile(const std::string &path, const std::string &tree_dir, Roots &roots)
{
	const BytesResult bytes = Jeiobs::Workspace::ReadFile(path);
	if (!bytes.ok()) {
		return bytes.error;
	}
	const auto parsed = Jeiobs::Workspace::ParseJson(bytes.bytes);
	if (!parsed.ok()) {
		return parsed.error;
	}
	if (!parsed.value->is_object()) {
		return {ErrorCode::Schema, "roots"};
	}

	std::map<std::string, std::string *> root_paths = {
		{"config", &roots.config.path},
		{"user", &roots.user.path},
		{"scenes", &roots.scenes.path},
		{"profiles", &roots.profiles.path},
		{"plugin_manager", &roots.plugin_manager.path},
	};
	for (const auto &[key, value] : parsed.value->items()) {
		const auto root = root_paths.find(key);
		if (root == root_paths.end() || !value.is_string() || !IsRootRelative(value.get<std::string>())) {
			return {ErrorCode::Schema, "roots"};
		}
		*root->second = JoinCliPath(tree_dir, value.get<std::string>());
		if (root->second->empty()) {
			return {ErrorCode::Io, {}};
		}
		if (key == "config") {
			roots.config.custom = true;
		} else if (key == "user") {
			roots.user.custom = true;
		} else if (key == "scenes") {
			roots.scenes.custom = true;
		} else if (key == "profiles") {
			roots.profiles.custom = true;
		} else if (key == "plugin_manager") {
			roots.plugin_manager.custom = true;
		}
	}
	return {};
}

int CheckPlan(int argc, char **argv)
{
	if (argc < 3) {
		PrintUsage();
		return 2;
	}
	const std::string tree_dir = argv[2];
	Roots roots;
	roots.config.path = JoinCliPath(tree_dir, "config");
	roots.user.path = JoinCliPath(tree_dir, "user");
	roots.scenes.path = JoinCliPath(tree_dir, "scenes");
	roots.profiles.path = JoinCliPath(tree_dir, "profiles");
	roots.plugin_manager.path = JoinCliPath(tree_dir, "plugin_manager");
	if (roots.config.path.empty() || roots.user.path.empty() || roots.scenes.path.empty() ||
	    roots.profiles.path.empty() || roots.plugin_manager.path.empty()) {
		PrintError({ErrorCode::Io, {}});
		return 1;
	}

	std::string roots_path;
	std::string parent_path;
	std::string app_dir = "obs-studio";
	std::uint64_t uid_seed = 0U;
	bool has_uid_seed = false;
	for (int index = 3; index < argc; ++index) {
		const std::string option = argv[index];
		if (option == "--roots" || option == "--parent" || option == "--uid-seed" || option == "--app-dir") {
			if (index + 1 >= argc) {
				PrintUsage();
				return 2;
			}
			const std::string value = argv[++index];
			if (option == "--roots") {
				roots_path = value;
			} else if (option == "--parent") {
				parent_path = value;
			} else if (option == "--app-dir") {
				app_dir = value;
			} else {
				if (!ParseUint64(value, uid_seed)) {
					PrintError({ErrorCode::Schema, "uid-seed"});
					return 1;
				}
				has_uid_seed = true;
			}
			continue;
		}
		PrintUsage();
		return 2;
	}
	roots.app_dir = app_dir;

	if (!roots_path.empty()) {
		const Error roots_error = ReadRootsFile(roots_path, tree_dir, roots);
		if (!roots_error.ok()) {
			PrintError(roots_error);
			return 1;
		}
	}

	RealFs fs;
	const FsListResult tree_status = fs.List(tree_dir);
	if (!tree_status.ok()) {
		PrintError(tree_status.error);
		return 1;
	}

	std::optional<Manifest> parent;
	if (!parent_path.empty()) {
		const ManifestResult loaded = Jeiobs::Workspace::LoadManifest(parent_path, ManifestMode::Local);
		if (!loaded.ok()) {
			PrintError(loaded.error);
			return 1;
		}
		parent = *loaded.value;
	}

	PlanResult result;
	if (has_uid_seed) {
		SeededUidSource source(uid_seed);
		result = CapturePlanner::Plan(fs, roots, parent.has_value() ? &*parent : nullptr, source);
	} else {
		result = CapturePlanner::Plan(fs, roots, parent.has_value() ? &*parent : nullptr);
	}
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	std::cout << PlanToJson(*result.value).dump() << '\n';
	return 0;
}

void PrintUsage()
{
	std::cerr << "usage: jw-check hash FILE | path PATH | unicode nfd|fold|key TEXT | manifest FILE [--export] | "
		     "revision FILE | child PARENT DRAFT | export MANIFEST DECISIONS --now RFC3339 | "
		     "redact ROLE INFILE OUTFILE REPORT [--ws-placeholder STR] | "
		     "plan TREE_DIR [--roots ROOTS.json] [--parent MANIFEST] [--uid-seed N] [--app-dir NAME]\n";
}

} // namespace

int Run(int argc, char **argv)
{
	try {
		if (argc < 3) {
			PrintUsage();
			return 2;
		}
		const std::string command = argv[1];
		if (command == "hash" && argc == 3) {
			return CheckHash(argv[2]);
		}
		if (command == "path" && argc == 3) {
			return CheckPath(argv[2]);
		}
		if (command == "unicode" && argc == 4) {
			const std::string function = argv[2];
			if (function == "nfd" || function == "fold" || function == "key") {
				return CheckUnicode(function, argv[3]);
			}
		}
		if (command == "manifest" && (argc == 3 || (argc == 4 && std::string(argv[3]) == "--export"))) {
			return CheckManifest(argv[2], argc == 4 ? ManifestMode::Export : ManifestMode::Local);
		}
		if (command == "revision" && argc == 3) {
			return CheckRevision(argv[2]);
		}
		if (command == "child" && argc == 4) {
			return CheckChild(argv[2], argv[3]);
		}
		if (command == "export" && argc == 6 && std::string(argv[4]) == "--now") {
			return CheckExport(argv[2], argv[3], argv[5]);
		}
		if (command == "redact" && (argc == 6 || argc == 8)) {
			RedactOptions options;
			if (argc == 8) {
				if (std::string(argv[6]) != "--ws-placeholder") {
					PrintUsage();
					return 2;
				}
				options.ws_placeholder = argv[7];
				options.has_ws_placeholder = true;
			}
			return CheckRedact(argv[2], argv[3], argv[4], argv[5], options);
		}
		if (command == "plan") {
			return CheckPlan(argc, argv);
		}
		PrintUsage();
		return 2;
	} catch (...) {
		PrintError({ErrorCode::Json, {}});
		return 1;
	}
}

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

// Windows passes the command line in UTF-16; convert it so that every path and argument reaches the
// library as UTF-8, as on the other platforms.
int wmain(int argc, wchar_t **wargv)
{
	try {
		std::vector<std::string> args;
		for (int i = 0; i < argc; ++i) {
			const int bytes = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
			std::string arg(bytes > 0 ? static_cast<std::size_t>(bytes) : 1U, '\0');
			if (bytes > 0) {
				WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, arg.data(), bytes, nullptr, nullptr);
			}
			arg.resize(std::char_traits<char>::length(arg.c_str()));
			args.push_back(std::move(arg));
		}
		std::vector<char *> argv;
		for (std::string &arg : args) {
			argv.push_back(arg.data());
		}
		argv.push_back(nullptr);
		return Run(argc, argv.data());
	} catch (...) {
		PrintError({ErrorCode::Json, {}});
		return 1;
	}
}
#else
int main(int argc, char **argv)
{
	return Run(argc, argv);
}
#endif
