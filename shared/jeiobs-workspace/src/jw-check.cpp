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
#include "jeiobs/workspace/Hash.hpp"
#include "jeiobs/workspace/Manifest.hpp"
#include "jeiobs/workspace/PathRules.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

namespace {

using Jeiobs::Workspace::Error;
using Jeiobs::Workspace::ErrorCode;
using Jeiobs::Workspace::Json;
using Jeiobs::Workspace::Manifest;
using Jeiobs::Workspace::ManifestMode;

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
	if (manifest.uncheckedPaths) {
		std::cout << " unchecked-paths";
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
	if (result.status == Jeiobs::Workspace::PathStatus::Unchecked) {
		std::cout << " unchecked";
	}
	std::cout << '\n';
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
	const auto manifest = Jeiobs::Workspace::LoadManifest(manifest_path, ManifestMode::Local);
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
	const auto hash = Jeiobs::Workspace::HashFile(manifest_path);
	if (!hash.ok()) {
		PrintError(hash.error);
		return 1;
	}
	const auto result = Jeiobs::Workspace::ProjectForExport(*manifest.value, hash.value, *decisions.value, now);
	if (!result.ok()) {
		PrintError(result.error);
		return 1;
	}
	const std::string exported_bytes = result.value->dump();
	std::cout << exported_bytes << '\n';
	return 0;
}

void PrintUsage()
{
	std::cerr << "usage: jw-check hash FILE | path PATH | manifest FILE [--export] | "
		     "revision FILE | child PARENT DRAFT | export MANIFEST DECISIONS --now RFC3339\n";
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
