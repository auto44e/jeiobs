/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 or later.
 */
#include "jeiobs/workspace/CapturePlanner.hpp"

#include "jeiobs/workspace/JsonScan.hpp"
#include "jeiobs/workspace/Limits.hpp"
#include "jeiobs/workspace/PathRules.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

constexpr std::size_t kUnknownCollapseCount = 100U;
constexpr std::size_t kNoIndex = std::numeric_limits<std::size_t>::max();

Error MakeError(ErrorCode code, std::string detail = {})
{
	return {code, std::move(detail)};
}

std::string JoinPath(std::string_view first, std::string_view second)
{
	if (first.empty()) {
		return std::string(second);
	}
	if (second.empty()) {
		return std::string(first);
	}
	std::string result(first);
	if (result.back() != '/') {
		result.push_back('/');
	}
	result.append(second);
	return result;
}

std::string JoinRelative(std::string_view first, std::string_view second)
{
	return JoinPath(first, second);
}

bool EndsWith(std::string_view value, std::string_view suffix)
{
	return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
}

bool IsTransient(std::string_view name)
{
	return EndsWith(name, ".tmp");
}

bool IsRepresentablePath(std::string_view path)
{
	return ValidatePath(path).status != PathStatus::Invalid;
}

std::uint64_t SaturatingAdd(std::uint64_t left, std::uint64_t right)
{
	if (right > std::numeric_limits<std::uint64_t>::max() - left) {
		return std::numeric_limits<std::uint64_t>::max();
	}
	return left + right;
}

struct Summary {
	std::uint64_t size = 0U;
	std::uint64_t files = 0U;
	Error error;
};

struct Child {
	std::string name;
	std::string path;
	std::string relative_path;
	FsEntryKind kind = FsEntryKind::Other;
	std::uint64_t size = 0U;
	bool name_representable = true;
};

struct RawEntry {
	enum class Category {
		User,
		PluginManager,
		Collection,
		Profile,
		Module,
	};

	std::string root;
	std::string path;
	std::string role;
	std::uint64_t size = 0U;
	Category category = Category::Module;
	std::string identity_uid;
	std::string pair_key;
	bool pair_main = false;
};

struct CollectionInfo {
	std::string root;
	std::string main_path;
	std::string bak_path;
	std::string main_fs_path;
	std::string bak_fs_path;
	std::uint64_t main_size = 0U;
	std::uint64_t bak_size = 0U;
	bool has_main = false;
	bool has_bak = false;
	std::string uid;
	std::vector<std::string> scene_uuids;
};

struct ProfileInfo {
	std::string root;
	std::string dir;
	std::string identity_path;
	std::string uid;
};

struct UnknownAccumulator {
	std::vector<Child> sample;
	std::uint64_t file_count = 0U;
	std::uint64_t size = 0U;
	Error error;

	[[nodiscard]] bool Add(const Fs &fs, const Child &child);
};

class PlannerImpl {
	const Fs &fs_;
	const Roots &roots_;
	const Manifest *parent_;
	UidSource &uid_source_;
	std::vector<RawEntry> stored_;
	std::vector<PlannedEntry> omitted_;
	std::vector<CollectionInfo> collections_;
	std::vector<ProfileInfo> profiles_;
	std::vector<PlanWarning> warnings_;
	Error error_;
	std::uint64_t stored_bytes_ = 0U;
	std::map<std::string, std::size_t> collection_indices_;
	std::map<std::string, std::size_t> omitted_indices_;

	struct StateSizes {
		std::size_t stored = 0U;
		std::size_t omitted = 0U;
		std::size_t collections = 0U;
		std::size_t profiles = 0U;
		std::size_t warnings = 0U;
		std::uint64_t stored_bytes = 0U;
	};

	[[nodiscard]] StateSizes SaveState() const
	{
		return {stored_.size(),   omitted_.size(),  collections_.size(),
			profiles_.size(), warnings_.size(), stored_bytes_};
	}

	void RestoreState(const StateSizes &state)
	{
		stored_.resize(state.stored);
		omitted_.resize(state.omitted);
		collections_.resize(state.collections);
		profiles_.resize(state.profiles);
		warnings_.resize(state.warnings);
		stored_bytes_ = state.stored_bytes;
		collection_indices_.clear();
		for (std::size_t index = 0U; index < collections_.size(); ++index) {
			collection_indices_[collections_[index].root + '\0' + collections_[index].main_path] = index;
		}
		omitted_indices_.clear();
		for (std::size_t index = 0U; index < omitted_.size(); ++index) {
			omitted_indices_[omitted_[index].root + '\0' + omitted_[index].path] = index;
		}
	}

	void Fail(Error error)
	{
		if (error_.ok()) {
			error_ = std::move(error);
		}
	}

	bool AddStored(RawEntry entry)
	{
		if (!error_.ok()) {
			return false;
		}
		if (stored_.size() + omitted_.size() >= Limits::kEntries) {
			Fail(MakeError(ErrorCode::Limit, "entries"));
			return false;
		}
		stored_.push_back(std::move(entry));
		return true;
	}

	[[nodiscard]] bool HasEntryCapacity() const { return stored_.size() + omitted_.size() < Limits::kEntries; }

	[[nodiscard]] bool HasMetadataCapacity() const
	{
		return collections_.size() + profiles_.size() < Limits::kEntries;
	}

	[[nodiscard]] bool ReserveMetadata()
	{
		if (!HasMetadataCapacity()) {
			Fail(MakeError(ErrorCode::Limit, "entries"));
			return false;
		}
		return true;
	}

	[[nodiscard]] bool CanStore(std::uint64_t size) const { return size <= Limits::kBlobBytes; }

	[[nodiscard]] bool AddStoredRaw(RawEntry entry) { return AddStored(std::move(entry)); }

	bool AddOmitted(std::string root, std::string path, std::uint64_t size, std::string reason,
			std::string kind = {}, std::uint64_t count = 0U)
	{
		if (!error_.ok()) {
			return false;
		}
		const std::string key = root + '\0' + path;
		if (reason == "unrepresentable" && kind == "names") {
			const auto existing_index = omitted_indices_.find(key);
			if (existing_index != omitted_indices_.end()) {
				PlannedEntry &existing = omitted_[existing_index->second];
				if (existing.reason == "unknown" && existing.kind == "dir") {
					existing.size = SaturatingAdd(existing.size, size);
					existing.count = SaturatingAdd(existing.count, count);
					return true;
				}
			}
		}
		if (reason == "unknown" && kind == "dir") {
			const auto existing_index = omitted_indices_.find(key);
			if (existing_index != omitted_indices_.end()) {
				PlannedEntry &existing = omitted_[existing_index->second];
				if (existing.reason == "unrepresentable" && existing.kind == "names") {
					existing.reason = "unknown";
					existing.kind = "dir";
					existing.size = SaturatingAdd(existing.size, size);
					existing.count = SaturatingAdd(existing.count, count);
					return true;
				}
			}
		}
		if (stored_.size() + omitted_.size() >= Limits::kEntries) {
			Fail(MakeError(ErrorCode::Limit, "entries"));
			return false;
		}
		PlannedEntry entry;
		entry.status = "omitted";
		entry.root = std::move(root);
		entry.path = std::move(path);
		entry.size = size;
		entry.reason = std::move(reason);
		entry.kind = std::move(kind);
		entry.count = count;
		omitted_.push_back(std::move(entry));
		omitted_indices_[key] = omitted_.size() - 1U;
		return true;
	}

	Summary Summarize(std::string_view directory) const;

	void AddUnknown(const std::string &root, const std::string &container, const Child &child,
			UnknownAccumulator &unknown)
	{
		if (!unknown.Add(fs_, child)) {
			AddListingFailure(root, child.relative_path, unknown.error, false);
		}
		(void)container;
	}

	void FinishUnknown(const std::string &root, const std::string &container, UnknownAccumulator &unknown)
	{
		if (unknown.file_count > kUnknownCollapseCount) {
			AddOmitted(root, container, unknown.size, "unknown", "dir", unknown.file_count);
			return;
		}
		for (const Child &child : unknown.sample) {
			AddOmitted(root, child.relative_path, child.size, "unknown",
				   child.kind == FsEntryKind::Directory ? std::string("dir") : std::string{});
		}
	}

	void AddUnrepresentable(const std::string &root, const std::string &container, const Child &child,
				std::uint64_t &count, std::uint64_t &size)
	{
		++count;
		if (child.kind == FsEntryKind::Directory && !child.path.empty() && !child.name.empty()) {
			const Summary summary = Summarize(child.path);
			if (!summary.error.ok()) {
				// The folder's own name is unrepresentable, so it is never recorded; the warning names the
				// folder that contains it.
				warnings_.push_back({"listing-error", container});
			} else {
				size = SaturatingAdd(size, summary.size);
			}
		} else if (child.kind == FsEntryKind::File) {
			size = SaturatingAdd(size, child.size);
		}
		(void)root;
	}

	void FinishUnrepresentable(const std::string &root, const std::string &container, std::uint64_t count,
				   std::uint64_t size)
	{
		if (count != 0U) {
			AddOmitted(root, container, size, "unrepresentable", "names", count);
		}
	}

	using ChildHandler = std::function<bool(const Child &)>;
	void ProcessDirectory(const std::string &fs_directory, const std::string &root, const std::string &relative,
			      const ChildHandler &handler, bool sort_entries = false);
	void ProcessOwnedRoot(const std::string &fs_directory, const std::string &root, const std::string &relative,
			      const ChildHandler &handler);

	void AddListingFailure(const std::string &root, const std::string &path, const Error &error,
			       bool missing_is_absent)
	{
		if (!error_.ok() || (missing_is_absent && error.detail == "missing")) {
			return;
		}
		if (error.detail == "symlink") {
			AddOmitted(root, path, 0U, "symlink");
			warnings_.push_back({"root-symlink", path});
		} else {
			AddOmitted(root, path, 0U, "unreadable", "dir");
			warnings_.push_back({"listing-error", path});
		}
	}

	void AddRootSymlink(const std::string &root, const std::string &path)
	{
		AddOmitted(root, path, 0U, "symlink");
		warnings_.push_back({"root-symlink", path});
	}

	void AddSpecial(const std::string &root, const Child &child, std::string_view reason)
	{
		std::uint64_t size = child.size;
		if (child.kind == FsEntryKind::Directory) {
			const Summary summary = Summarize(child.path);
			if (!summary.error.ok()) {
				AddListingFailure(root, child.relative_path, summary.error, false);
				return;
			}
			size = summary.size;
		}
		AddOmitted(root, child.relative_path, size, std::string(reason),
			   child.kind == FsEntryKind::Directory ? "dir" : std::string{});
	}

	[[nodiscard]] bool AddStoredFile(const std::string &root, const std::string &path, std::string_view role,
					 std::uint64_t size, RawEntry::Category category, std::string identity_uid = {},
					 std::string pair_key = {}, bool pair_main = false)
	{
		if (!HasEntryCapacity()) {
			Fail(MakeError(ErrorCode::Limit, "entries"));
			return false;
		}
		if (!CanStore(size)) {
			AddOmitted(root, path, size, "too-large");
			return false;
		}
		RawEntry entry;
		entry.root = root;
		entry.path = path;
		entry.role = role;
		entry.size = size;
		entry.category = category;
		entry.identity_uid = std::move(identity_uid);
		entry.pair_key = std::move(pair_key);
		entry.pair_main = pair_main;
		return AddStoredRaw(std::move(entry));
	}

	void ProcessConfigModule(const std::string &fs_directory, const std::string &root, const std::string &relative,
				 std::string_view module);
	void ProcessPluginConfig(const std::string &fs_directory, const std::string &root, const std::string &relative);
	void ProcessConfigRoot(const std::string &fs_directory);

	void AddCollectionFile(const std::string &root, const std::string &relative, const Child &child);
	void ProcessCollectionDirectory(const std::string &fs_directory, const std::string &root,
					const std::string &relative);
	void ProcessScenesRoot(const std::string &fs_directory);

	void AddProfileFile(std::size_t profile_index, const Child &child);
	void ProcessProfileDirectory(std::size_t profile_index, const std::string &fs_directory,
				     const std::string &root, const std::string &relative);
	void ProcessProfilesRoot(const std::string &fs_directory);

	void ProcessUserRoot(const std::string &fs_directory);
	void ProcessPluginManagerRoot(const std::string &fs_directory);

	void AssignCollectionUids();
	void AssignProfileUids();
	void BuildCollectionEntries();
	void BuildProfileEntryIdentities();
	void ApplyStoredBudget();
	void MarkCollisions(Plan &plan);

public:
	[[nodiscard]] PlanResult Run();

	PlannerImpl(const Fs &fs, const Roots &roots, const Manifest *parent, UidSource &uid_source)
		: fs_(fs),
		  roots_(roots),
		  parent_(parent),
		  uid_source_(uid_source)
	{
	}
};

Summary PlannerImpl::Summarize(std::string_view directory) const
{
	Summary result;
	std::vector<std::string> pending;
	pending.emplace_back(directory);
	while (!pending.empty()) {
		std::string current = std::move(pending.back());
		pending.pop_back();
		const Error list_error = fs_.ListEach(current, [&](const FsEntry &entry) {
			const std::string child_path = JoinPath(current, entry.name);
			if (entry.kind == FsEntryKind::File) {
				++result.files;
				result.size = SaturatingAdd(result.size, entry.size);
			} else if (entry.kind == FsEntryKind::Directory) {
				pending.push_back(child_path);
			}
			return true;
		});
		if (!list_error.ok()) {
			result.error = list_error;
			return result;
		}
	}
	return result;
}

bool UnknownAccumulator::Add(const Fs &fs, const Child &child)
{
	Summary summary;
	if (child.kind == FsEntryKind::File) {
		summary.files = 1U;
		summary.size = child.size;
	} else if (child.kind == FsEntryKind::Directory) {
		std::vector<std::string> pending;
		pending.push_back(child.path);
		while (!pending.empty()) {
			std::string current = std::move(pending.back());
			pending.pop_back();
			const Error list_error = fs.ListEach(current, [&](const FsEntry &entry) {
				const std::string child_path = JoinPath(current, entry.name);
				if (entry.kind == FsEntryKind::File) {
					++summary.files;
					summary.size = SaturatingAdd(summary.size, entry.size);
				} else if (entry.kind == FsEntryKind::Directory) {
					pending.push_back(child_path);
				}
				return true;
			});
			if (!list_error.ok()) {
				error = list_error;
				return false;
			}
		}
	}
	file_count = SaturatingAdd(file_count, summary.files);
	size = SaturatingAdd(size, summary.size);
	if (file_count <= kUnknownCollapseCount) {
		Child sample_child = child;
		if (child.kind == FsEntryKind::Directory) {
			sample_child.size = summary.size;
		}
		sample.push_back(std::move(sample_child));
	} else {
		sample.clear();
	}
	return true;
}

void PlannerImpl::ProcessDirectory(const std::string &fs_directory, const std::string &root,
				   const std::string &relative, const ChildHandler &handler, bool sort_entries)
{
	const StateSizes state = SaveState();
	UnknownAccumulator unknown;
	std::uint64_t unrepresentable_count = 0U;
	std::uint64_t unrepresentable_size = 0U;
	auto process_entry = [&](const FsEntry &entry) {
		const Child child{entry.name,
				  JoinPath(fs_directory, entry.name),
				  JoinPath(relative, entry.name),
				  entry.kind,
				  entry.size,
				  entry.name_representable};
		const std::string child_relative = JoinRelative(relative, entry.name);
		Child relative_child = child;
		if (!entry.name_representable || !IsRepresentablePath(child_relative)) {
			AddUnrepresentable(root, relative, relative_child, unrepresentable_count, unrepresentable_size);
			return error_.ok();
		}
		if (entry.kind == FsEntryKind::Symlink) {
			AddOmitted(root, child_relative, entry.size, "symlink");
			return error_.ok();
		}
		if (entry.kind == FsEntryKind::File &&
		    (EndsWith(entry.name, ".json.v1") || EndsWith(entry.name, ".json.invalid"))) {
			AddOmitted(root, child_relative, entry.size, "not-loaded");
			return error_.ok();
		}
		if (entry.kind == FsEntryKind::File && IsTransient(entry.name)) {
			AddOmitted(root, child_relative, entry.size, "transient");
			return error_.ok();
		}
		if (handler(relative_child)) {
			return error_.ok();
		}
		AddUnknown(root, relative, child, unknown);
		return error_.ok();
	};
	Error list_error;
	if (sort_entries) {
		FsListResult listed = fs_.List(fs_directory);
		if (listed.ok()) {
			std::sort(listed.entries.begin(), listed.entries.end(),
				  [](const FsEntry &left, const FsEntry &right) { return left.name < right.name; });
			for (const FsEntry &entry : listed.entries) {
				if (!process_entry(entry)) {
					break;
				}
			}
		}
		list_error = std::move(listed.error);
	} else {
		list_error = fs_.ListEach(fs_directory, process_entry);
	}
	if (!list_error.ok()) {
		RestoreState(state);
		AddListingFailure(root, relative, list_error, false);
		return;
	}
	FinishUnrepresentable(root, relative, unrepresentable_count, unrepresentable_size);
	FinishUnknown(root, relative, unknown);
}

void PlannerImpl::ProcessOwnedRoot(const std::string &fs_directory, const std::string &root,
				   const std::string &relative, const ChildHandler &handler)
{
	const StateSizes state = SaveState();
	const Error list_error = fs_.ListEach(fs_directory, [&](const FsEntry &entry) {
		const Child child{entry.name, JoinPath(fs_directory, entry.name),
				  entry.name, entry.kind,
				  entry.size, entry.name_representable};
		if (!entry.name_representable || !IsRepresentablePath(entry.name)) {
			return error_.ok();
		}
		(void)handler(child);
		return error_.ok();
	});
	if (!list_error.ok()) {
		RestoreState(state);
		AddListingFailure(root, relative, list_error, true);
		return;
	}
}

void PlannerImpl::ProcessConfigModule(const std::string &fs_directory, const std::string &root,
				      const std::string &relative, std::string_view module)
{
	ProcessDirectory(fs_directory, root, relative, [&](const Child &child) {
		const std::string_view name = child.name;
		std::string_view role;
		if (module == "obs-websocket" && name == "config.json") {
			role = "module.obs-websocket";
		} else if (module == "decklink-output-ui" &&
			   (name == "decklinkOutputProps.json" || name == "decklinkPreviewOutputProps.json")) {
			role = "module.decklink-output-ui";
		} else if (module == "aja-output-ui" &&
			   (name == "ajaOutputProps.json" || name == "ajaPreviewOutputProps.json" ||
			    name == "ajaMiscProps.json")) {
			role = "module.aja-output-ui";
		} else if ((module == "decklink-output-ui" || module == "aja-output-ui") &&
			   EndsWith(name, ".json.bak")) {
			AddSpecial(root, child, "not-loaded");
			return true;
		} else if (module == "obs-vst3" && (name == "vst3list.json" || name == "vst3list.json.bak")) {
			AddSpecial(root, child, "out-of-process-writer");
			return true;
		}
		if (role.empty() || child.kind != FsEntryKind::File) {
			return false;
		}
		(void)AddStoredFile(root, child.relative_path, role, child.size, RawEntry::Category::Module);
		return true;
	});
}

void PlannerImpl::ProcessPluginConfig(const std::string &fs_directory, const std::string &root,
				      const std::string &relative)
{
	ProcessDirectory(fs_directory, root, relative, [&](const Child &child) {
		if (child.kind != FsEntryKind::Directory) {
			return false;
		}
		if (child.name == "obs-browser") {
			AddSpecial(root, child, "secret-store");
			return true;
		}
		if (child.name == "rtmp-services" || child.name == "text-freetype2" || child.name == "win-capture") {
			AddSpecial(root, child, "cache");
			return true;
		}
		if (child.name == "obs-websocket" || child.name == "decklink-output-ui" ||
		    child.name == "aja-output-ui" || child.name == "obs-vst3") {
			ProcessConfigModule(child.path, root, JoinRelative(relative, child.name), child.name);
			return true;
		}
		AddSpecial(root, child, "unknown");
		return true;
	});
}

void PlannerImpl::ProcessConfigRoot(const std::string &fs_directory)
{
	ProcessOwnedRoot(fs_directory, "config", roots_.app_dir, [&](const Child &child) {
		if (child.name == "plugin_config") {
			if (child.kind == FsEntryKind::Symlink) {
				AddRootSymlink("config", "plugin_config");
			} else if (child.kind == FsEntryKind::Directory) {
				ProcessPluginConfig(child.path, "config", "plugin_config");
			}
			return true;
		}
		if (child.name == "logs" || child.name == "crashes" || child.name == "profiler_data" ||
		    child.name == "updates" || child.name == ".sentinel") {
			if (child.kind == FsEntryKind::Symlink) {
				AddOmitted("config", child.relative_path, 0U, "symlink");
			} else {
				AddSpecial("config", child, "runtime");
			}
		}
		return true;
	});
}

void PlannerImpl::AddCollectionFile(const std::string &root, const std::string &relative, const Child &child)
{
	const bool is_bak = EndsWith(child.name, ".json.bak");
	const bool is_main = EndsWith(child.name, ".json") && !is_bak && child.name.size() > 5U;
	if (!is_main && !is_bak) {
		return;
	}
	const std::string main_name = is_bak ? child.name.substr(0U, child.name.size() - 4U) : child.name;
	if (main_name.size() <= 5U || !EndsWith(main_name, ".json") || child.kind != FsEntryKind::File) {
		return;
	}
	const std::string collection_name = main_name.substr(0U, main_name.size() - 5U);
	if (collection_name.empty() || !IsRepresentablePath(collection_name)) {
		return;
	}
	if (!HasEntryCapacity()) {
		Fail(MakeError(ErrorCode::Limit, "entries"));
		return;
	}
	if (!CanStore(child.size)) {
		AddOmitted(root, relative, child.size, "too-large");
		return;
	}
	const std::string main_path = JoinRelative(relative.substr(0U, relative.size() - child.name.size()), main_name);
	const std::string collection_key = root + '\0' + main_path;
	auto index_iterator = collection_indices_.find(collection_key);
	if (index_iterator == collection_indices_.end()) {
		if (!ReserveMetadata()) {
			return;
		}
		CollectionInfo info;
		info.root = root;
		info.main_path = is_main ? relative : main_path;
		info.bak_path = is_bak ? relative : std::string{};
		info.main_fs_path = is_main ? child.path : std::string{};
		info.bak_fs_path = is_bak ? child.path : std::string{};
		info.main_size = is_main ? child.size : 0U;
		info.bak_size = is_bak ? child.size : 0U;
		info.has_main = is_main;
		info.has_bak = is_bak;
		collections_.push_back(std::move(info));
		const std::size_t index = collections_.size() - 1U;
		collection_indices_[collection_key] = index;
		index_iterator = collection_indices_.find(collection_key);
	} else if (is_main) {
		CollectionInfo &collection = collections_[index_iterator->second];
		collection.main_path = relative;
		collection.main_fs_path = child.path;
		collection.main_size = child.size;
		collection.has_main = true;
	} else {
		CollectionInfo &collection = collections_[index_iterator->second];
		collection.bak_path = relative;
		collection.bak_fs_path = child.path;
		collection.bak_size = child.size;
		collection.has_bak = true;
	}

	if (is_main) {
		const BytesResult bytes = fs_.ReadBounded(child.path, Limits::kDocumentBytes);
		const JsonScanResult scan = bytes.ok() ? JsonScan::Scan(bytes.bytes) : JsonScanResult{};
		if (!bytes.ok() || !scan.ok() || scan.values.empty() ||
		    scan.values.front().kind != JsonValueKind::Object) {
			warnings_.push_back({"collection-unparseable", relative});
		} else {
			struct SourceFields {
				std::size_t id = kNoIndex;
				std::size_t uuid = kNoIndex;
			};
			std::size_t sources_array_index = kNoIndex;
			for (const JsonMemberOccurrence &member : scan.members) {
				if (member.parent_index == 0U && member.key == "sources" &&
				    member.value_index < scan.values.size() &&
				    scan.values[member.value_index].kind == JsonValueKind::Array) {
					sources_array_index = member.value_index;
					break;
				}
			}
			std::vector<std::size_t> source_indices;
			std::unordered_set<std::size_t> source_index_set;
			if (sources_array_index != kNoIndex) {
				for (std::size_t index = 0U; index < scan.values.size(); ++index) {
					const JsonValueOccurrence &value = scan.values[index];
					if (value.parent_index == sources_array_index &&
					    value.array_index != kJsonNoIndex && value.kind == JsonValueKind::Object) {
						source_indices.push_back(index);
						source_index_set.insert(index);
					}
				}
			}

			std::unordered_map<std::size_t, SourceFields> fields;
			fields.reserve(source_indices.size());
			for (const JsonMemberOccurrence &member : scan.members) {
				if (!source_index_set.count(member.parent_index)) {
					continue;
				}
				SourceFields &source = fields[member.parent_index];
				if (member.key == "id") {
					source.id = member.value_index;
				} else if (member.key == "uuid") {
					source.uuid = member.value_index;
				}
			}

			std::vector<std::string> scene_uuids;
			std::unordered_set<std::string> seen;
			seen.reserve(std::min<std::size_t>(source_indices.size(), 100000U));
			bool scene_uuid_limit = false;
			for (const std::size_t source_index : source_indices) {
				const auto fields_iterator = fields.find(source_index);
				if (fields_iterator == fields.end() ||
				    fields_iterator->second.id >= scan.values.size() ||
				    fields_iterator->second.uuid >= scan.values.size()) {
					continue;
				}
				const JsonValueOccurrence &id_value = scan.values[fields_iterator->second.id];
				const JsonValueOccurrence &uuid_value = scan.values[fields_iterator->second.uuid];
				if (id_value.kind != JsonValueKind::String || id_value.string_value != "scene" ||
				    uuid_value.kind != JsonValueKind::String || uuid_value.string_value.size() > 256U) {
					continue;
				}
				if (!seen.insert(uuid_value.string_value).second) {
					continue;
				}
				if (scene_uuids.size() >= 100000U) {
					scene_uuid_limit = true;
					break;
				}
				scene_uuids.push_back(uuid_value.string_value);
			}
			CollectionInfo &collection = collections_[index_iterator->second];
			if (scene_uuid_limit) {
				collection.scene_uuids.clear();
				warnings_.push_back({"scene-uuids-limit", relative});
			} else {
				std::sort(scene_uuids.begin(), scene_uuids.end());
				collection.scene_uuids = std::move(scene_uuids);
			}
		}
	}
}

void PlannerImpl::ProcessCollectionDirectory(const std::string &fs_directory, const std::string &root,
					     const std::string &relative)
{
	ProcessDirectory(
		fs_directory, root, relative,
		[&](const Child &child) {
			if (child.kind == FsEntryKind::File &&
			    (EndsWith(child.name, ".json") || EndsWith(child.name, ".json.bak"))) {
				const std::string main_name = EndsWith(child.name, ".json.bak")
								      ? child.name.substr(0U, child.name.size() - 4U)
								      : child.name;
				if (main_name.size() <= 5U || !EndsWith(main_name, ".json")) {
					return false;
				}
				const std::string collection_name = main_name.substr(0U, main_name.size() - 5U);
				if (collection_name.empty() || !IsRepresentablePath(collection_name)) {
					return false;
				}
				AddCollectionFile(root, JoinRelative(relative, child.name), child);
				return true;
			}
			if (child.kind == FsEntryKind::File &&
			    (EndsWith(child.name, ".json.v1") || EndsWith(child.name, ".json.invalid"))) {
				AddSpecial(root, child, "not-loaded");
				return true;
			}
			if (IsTransient(child.name)) {
				AddSpecial(root, child, "transient");
				return true;
			}
			return false;
		},
		true);
}

void PlannerImpl::ProcessScenesRoot(const std::string &fs_directory)
{
	ProcessOwnedRoot(fs_directory, "scenes", roots_.app_dir, [&](const Child &child) {
		if (child.name != "basic") {
			return false;
		}
		if (child.kind == FsEntryKind::Symlink) {
			AddRootSymlink("scenes", "basic");
		} else if (child.kind == FsEntryKind::Directory) {
			ProcessOwnedRoot(child.path, "scenes", "basic", [&](const Child &basic_child) {
				if (basic_child.name != "scenes") {
					return true;
				}
				if (basic_child.kind == FsEntryKind::Symlink) {
					AddRootSymlink("scenes", "basic/scenes");
				} else if (basic_child.kind == FsEntryKind::Directory) {
					ProcessCollectionDirectory(basic_child.path, "scenes", "basic/scenes");
				}
				return true;
			});
		}
		return true;
	});
}

void PlannerImpl::AddProfileFile(std::size_t profile_index, const Child &child)
{
	ProfileInfo &profile = profiles_[profile_index];
	const std::string &root = profile.root;
	const std::string profile_prefix = "basic/profiles/" + profile.dir + "/";
	std::string_view role;
	bool main = false;
	std::string main_name;
	if (child.name == "basic.ini") {
		role = "profile.basic";
		main = true;
		main_name = child.name;
	} else if (child.name == "service.json" || child.name == "service.json.bak") {
		role = child.name == "service.json" ? "profile.service" : "profile.service.bak";
		main = child.name == "service.json";
		main_name = "service.json";
	} else if (child.name == "streamEncoder.json" || child.name == "streamEncoder.json.bak") {
		role = child.name == "streamEncoder.json" ? "profile.encoder.stream" : "profile.encoder.stream.bak";
		main = child.name == "streamEncoder.json";
		main_name = "streamEncoder.json";
	} else if (child.name == "recordEncoder.json" || child.name == "recordEncoder.json.bak") {
		role = child.name == "recordEncoder.json" ? "profile.encoder.record" : "profile.encoder.record.bak";
		main = child.name == "recordEncoder.json";
		main_name = "recordEncoder.json";
	} else {
		return;
	}
	if (child.kind != FsEntryKind::File) {
		return;
	}
	const std::string path = profile_prefix + child.name;
	const std::string pair_key = profile_prefix + main_name;
	const bool stored = AddStoredFile(root, path, role, child.size, RawEntry::Category::Profile, profile.uid,
					  pair_key, main && child.name != "basic.ini");
	if (stored && main && profile.identity_path.empty()) {
		profile.identity_path = path;
	}
}

void PlannerImpl::ProcessProfileDirectory(std::size_t profile_index, const std::string &fs_directory,
					  const std::string &root, const std::string &relative)
{
	ProcessDirectory(fs_directory, root, relative, [&](const Child &child) {
		if (child.kind == FsEntryKind::File &&
		    (child.name == "basic.ini" || child.name == "service.json" || child.name == "service.json.bak" ||
		     child.name == "streamEncoder.json" || child.name == "streamEncoder.json.bak" ||
		     child.name == "recordEncoder.json" || child.name == "recordEncoder.json.bak")) {
			AddProfileFile(profile_index, child);
			return true;
		}
		if (child.kind == FsEntryKind::File &&
		    (EndsWith(child.name, ".json.v1") || EndsWith(child.name, ".json.invalid"))) {
			AddSpecial(root, child, "not-loaded");
			return true;
		}
		if (IsTransient(child.name)) {
			AddSpecial(root, child, "transient");
			return true;
		}
		return false;
	});
}

void PlannerImpl::ProcessProfilesRoot(const std::string &fs_directory)
{
	ProcessOwnedRoot(fs_directory, "profiles", roots_.app_dir, [&](const Child &child) {
		if (child.name != "basic") {
			return false;
		}
		if (child.kind == FsEntryKind::Symlink) {
			AddRootSymlink("profiles", "basic");
		} else if (child.kind == FsEntryKind::Directory) {
			ProcessOwnedRoot(child.path, "profiles", "basic", [&](const Child &basic_child) {
				if (basic_child.name != "profiles") {
					return true;
				}
				if (basic_child.kind == FsEntryKind::Symlink) {
					AddRootSymlink("profiles", "basic/profiles");
				} else if (basic_child.kind == FsEntryKind::Directory) {
					ProcessDirectory(
						basic_child.path, "profiles", "basic/profiles",
						[&](const Child &profile_child) {
							if (profile_child.kind != FsEntryKind::Directory) {
								return false;
							}
							const FsListResult profile_listing =
								fs_.List(profile_child.path);
							if (!profile_listing.ok()) {
								AddListingFailure("profiles",
										  profile_child.relative_path,
										  profile_listing.error, false);
								return true;
							}
							const bool has_basic_ini = std::any_of(
								profile_listing.entries.begin(),
								profile_listing.entries.end(),
								[](const FsEntry &entry) {
									return entry.name_representable &&
									       entry.kind == FsEntryKind::File &&
									       entry.name == "basic.ini";
								});
							if (!has_basic_ini) {
								AddSpecial("profiles", profile_child, "unknown");
								return true;
							}
							if (!ReserveMetadata()) {
								return true;
							}
							ProfileInfo profile;
							profile.root = "profiles";
							profile.dir = profile_child.name;
							profiles_.push_back(std::move(profile));
							const std::size_t profile_index = profiles_.size() - 1U;
							ProcessProfileDirectory(profile_index, profile_child.path,
										"profiles",
										"basic/profiles/" + profile_child.name);
							return true;
						});
				}
				return true;
			});
		}
		return true;
	});
}

void PlannerImpl::ProcessUserRoot(const std::string &fs_directory)
{
	ProcessOwnedRoot(fs_directory, "user", roots_.app_dir, [&](const Child &child) {
		if (child.name != "user.ini") {
			return true;
		}
		if (child.kind == FsEntryKind::Symlink) {
			AddOmitted("user", "user.ini", 0U, "symlink");
		} else if (child.kind == FsEntryKind::File) {
			(void)AddStoredFile("user", "user.ini", "app.user", child.size, RawEntry::Category::User);
		}
		return true;
	});
}

void PlannerImpl::ProcessPluginManagerRoot(const std::string &fs_directory)
{
	ProcessOwnedRoot(fs_directory, "plugin_manager", roots_.app_dir, [&](const Child &child) {
		if (child.name != "plugin_manager") {
			return false;
		}
		if (child.kind == FsEntryKind::Symlink) {
			AddRootSymlink("plugin_manager", "plugin_manager");
		} else if (child.kind == FsEntryKind::Directory) {
			ProcessDirectory(child.path, "plugin_manager", "plugin_manager", [&](const Child &nested) {
				if (nested.kind == FsEntryKind::File && nested.name == "modules.json") {
					(void)AddStoredFile("plugin_manager", "plugin_manager/modules.json",
							    "app.modules", nested.size,
							    RawEntry::Category::PluginManager);
					return true;
				}
				if (IsTransient(nested.name)) {
					AddSpecial("plugin_manager", nested, "transient");
					return true;
				}
				return false;
			});
		}
		return true;
	});
}

void PlannerImpl::BuildProfileEntryIdentities()
{
	std::map<std::string, std::string> uid_by_profile_path;
	for (const ProfileInfo &profile : profiles_) {
		uid_by_profile_path[profile.root + '\0' + "basic/profiles/" + profile.dir + "/"] = profile.uid;
	}
	for (RawEntry &entry : stored_) {
		if (entry.category != RawEntry::Category::Profile) {
			continue;
		}
		const std::size_t prefix_end = entry.path.find('/', std::string("basic/profiles/").size());
		if (prefix_end == std::string::npos) {
			continue;
		}
		const auto uid = uid_by_profile_path.find(entry.root + '\0' + entry.path.substr(0U, prefix_end + 1U));
		if (uid != uid_by_profile_path.end()) {
			entry.identity_uid = uid->second;
		}
	}
}

std::string SceneKey(const std::vector<std::string> &uuids)
{
	std::string key;
	for (const std::string &uuid : uuids) {
		key += std::to_string(uuid.size());
		key.push_back(':');
		key += uuid;
		key.push_back('|');
	}
	return key;
}

int ProfileFileRank(std::string_view path)
{
	const std::size_t slash = path.rfind('/');
	const std::string_view name = slash == std::string_view::npos ? path : path.substr(slash + 1U);
	if (name == "basic.ini") {
		return 0;
	}
	if (name == "service.json") {
		return 1;
	}
	if (name == "service.json.bak") {
		return 2;
	}
	if (name == "streamEncoder.json") {
		return 3;
	}
	if (name == "streamEncoder.json.bak") {
		return 4;
	}
	if (name == "recordEncoder.json") {
		return 5;
	}
	if (name == "recordEncoder.json.bak") {
		return 6;
	}
	return 7;
}

void PlannerImpl::AssignCollectionUids()
{
	std::sort(collections_.begin(), collections_.end(),
		  [](const CollectionInfo &left, const CollectionInfo &right) {
			  return left.main_path < right.main_path;
		  });
	std::map<std::string, std::string> parent_by_path;
	std::map<std::string, std::vector<std::string>> parent_by_scene_set;
	std::set<std::string> claimed;
	if (parent_ != nullptr && parent_->mode == ManifestMode::Local) {
		for (const Json &entry : parent_->document.at("entries")) {
			if (!entry.is_object() || !entry.contains("root") || !entry.contains("path") ||
			    !entry.contains("collection") || !entry.at("collection").is_string()) {
				continue;
			}
			const std::string key =
				entry.at("root").get<std::string>() + '\0' + entry.at("path").get<std::string>();
			parent_by_path[key] = entry.at("collection").get<std::string>();
			if (entry.contains("scene_uuids") && entry.at("scene_uuids").is_array() &&
			    !entry.at("scene_uuids").empty()) {
				std::vector<std::string> uuids;
				for (const Json &uuid : entry.at("scene_uuids")) {
					if (uuid.is_string()) {
						uuids.push_back(uuid.get<std::string>());
					}
				}
				std::sort(uuids.begin(), uuids.end());
				uuids.erase(std::unique(uuids.begin(), uuids.end()), uuids.end());
				if (!uuids.empty()) {
					parent_by_scene_set[SceneKey(uuids)].push_back(
						entry.at("collection").get<std::string>());
				}
			}
		}
	}

	for (CollectionInfo &collection : collections_) {
		if (!collection.has_main) {
			continue;
		}
		const auto path_match = parent_by_path.find(collection.root + '\0' + collection.main_path);
		if (path_match != parent_by_path.end() && claimed.insert(path_match->second).second) {
			collection.uid = path_match->second;
		}
	}

	std::map<std::string, std::size_t> current_set_count;
	for (const CollectionInfo &collection : collections_) {
		if (!collection.has_main || !collection.uid.empty()) {
			continue;
		}
		if (!collection.scene_uuids.empty()) {
			++current_set_count[SceneKey(collection.scene_uuids)];
		}
	}
	for (CollectionInfo &collection : collections_) {
		if (!collection.has_main || !collection.uid.empty()) {
			continue;
		}
		if (collection.scene_uuids.empty() || current_set_count[SceneKey(collection.scene_uuids)] != 1U) {
			continue;
		}
		std::vector<std::string> candidates;
		const auto scene_match = parent_by_scene_set.find(SceneKey(collection.scene_uuids));
		if (scene_match != parent_by_scene_set.end()) {
			for (const std::string &candidate : scene_match->second) {
				if (!claimed.count(candidate)) {
					candidates.push_back(candidate);
				}
			}
		}
		if (candidates.size() == 1U) {
			collection.uid = candidates.front();
			claimed.insert(collection.uid);
		}
	}
	for (CollectionInfo &collection : collections_) {
		if (collection.has_main && collection.uid.empty()) {
			collection.uid = uid_source_.NextUid();
		}
	}
}

void PlannerImpl::AssignProfileUids()
{
	std::sort(profiles_.begin(), profiles_.end(),
		  [](const ProfileInfo &left, const ProfileInfo &right) { return left.dir < right.dir; });
	std::map<std::string, std::string> parent_by_path;
	std::set<std::string> claimed;
	if (parent_ != nullptr && parent_->mode == ManifestMode::Local) {
		for (const Json &entry : parent_->document.at("entries")) {
			if (entry.is_object() && entry.contains("root") && entry.contains("path") &&
			    entry.contains("profile") && entry.at("profile").is_string()) {
				parent_by_path[entry.at("root").get<std::string>() + '\0' +
					       entry.at("path").get<std::string>()] =
					entry.at("profile").get<std::string>();
			}
		}
	}

	std::map<std::string, std::set<std::string>> candidates_by_profile;
	for (const RawEntry &entry : stored_) {
		if (entry.category != RawEntry::Category::Profile) {
			continue;
		}
		const std::size_t prefix_end = entry.path.find('/', std::string("basic/profiles/").size());
		if (prefix_end == std::string::npos) {
			continue;
		}
		const std::string prefix = entry.path.substr(0U, prefix_end + 1U);
		const auto match = parent_by_path.find(entry.root + '\0' + entry.path);
		if (match != parent_by_path.end()) {
			candidates_by_profile[entry.root + '\0' + prefix].insert(match->second);
		}
	}
	for (ProfileInfo &profile : profiles_) {
		const std::string prefix = "basic/profiles/" + profile.dir + "/";
		const auto candidates = candidates_by_profile.find(profile.root + '\0' + prefix);
		if (candidates != candidates_by_profile.end() && candidates->second.size() == 1U &&
		    claimed.insert(*candidates->second.begin()).second) {
			profile.uid = *candidates->second.begin();
			continue;
		}
		profile.uid = uid_source_.NextUid();
	}
}

void PlannerImpl::BuildCollectionEntries()
{
	std::vector<RawEntry> collection_entries;
	for (CollectionInfo &collection : collections_) {
		const std::string &uid = collection.uid;
		if (collection.has_main) {
			RawEntry entry;
			entry.root = collection.root;
			entry.path = collection.main_path;
			entry.role = "collection";
			entry.size = collection.main_size;
			entry.category = RawEntry::Category::Collection;
			entry.identity_uid = uid;
			entry.pair_key = collection.root + '\0' + collection.main_path;
			entry.pair_main = true;
			collection_entries.push_back(std::move(entry));
		}
		if (collection.has_main && collection.has_bak) {
			RawEntry entry;
			entry.root = collection.root;
			entry.path = collection.bak_path;
			entry.role = "collection.bak";
			entry.size = collection.bak_size;
			entry.category = RawEntry::Category::Collection;
			entry.identity_uid = uid;
			entry.pair_key = collection.root + '\0' + collection.main_path;
			collection_entries.push_back(std::move(entry));
		}
		if (!collection.has_main && collection.has_bak) {
			RawEntry entry;
			entry.root = collection.root;
			entry.path = collection.bak_path;
			entry.role = "collection.bak";
			entry.size = collection.bak_size;
			entry.category = RawEntry::Category::Collection;
			collection_entries.push_back(std::move(entry));
		}
	}
	for (RawEntry &entry : collection_entries) {
		if (!AddStoredRaw(std::move(entry))) {
			return;
		}
	}
}

void PlannerImpl::ApplyStoredBudget()
{
	std::vector<RawEntry> accepted;
	std::vector<RawEntry> too_large;
	accepted.reserve(stored_.size());
	too_large.reserve(stored_.size());

	std::uint64_t stored_bytes = 0U;
	for (RawEntry &entry : stored_) {
		if (stored_bytes <= Limits::kRevisionBytes && entry.size <= Limits::kRevisionBytes - stored_bytes) {
			stored_bytes = SaturatingAdd(stored_bytes, entry.size);
			accepted.push_back(std::move(entry));
		} else {
			too_large.push_back(std::move(entry));
		}
	}
	stored_ = std::move(accepted);
	stored_bytes_ = stored_bytes;

	for (const RawEntry &entry : too_large) {
		AddOmitted(entry.root, entry.path, entry.size, "too-large");
	}
}

void PlannerImpl::MarkCollisions(Plan &plan)
{
	std::map<std::string, std::vector<std::size_t>> groups;
	for (std::size_t index = 0U; index < plan.entries.size(); ++index) {
		groups[EntryCollisionKey(plan.entries[index].root, plan.entries[index].path)].push_back(index);
	}
	for (const auto &[key, indices] : groups) {
		if (indices.size() < 2U) {
			continue;
		}
		std::vector<std::string> paths;
		for (const std::size_t index : indices) {
			plan.entries[index].collision = true;
			paths.push_back(plan.entries[index].path);
		}
		std::sort(paths.begin(), paths.end());
		std::string joined;
		for (const std::string &path : paths) {
			if (!joined.empty()) {
				joined.push_back('|');
			}
			joined += path;
		}
		plan.warnings.push_back({"case-collision", joined});
	}
	std::sort(plan.warnings.begin(), plan.warnings.end(), [](const PlanWarning &left, const PlanWarning &right) {
		return left.code == right.code ? left.path < right.path : left.code < right.code;
	});
}

PlanResult PlannerImpl::Run()
{
	if (roots_.app_dir.empty() || roots_.app_dir.find('/') != std::string::npos ||
	    roots_.app_dir.find('\\') != std::string::npos || !IsRepresentablePath(roots_.app_dir)) {
		return {std::nullopt, MakeError(ErrorCode::Schema, "app-dir")};
	}
	if (parent_ != nullptr && parent_->mode != ManifestMode::Local) {
		return {std::nullopt, MakeError(ErrorCode::Schema, "parent")};
	}

	ProcessUserRoot(JoinPath(roots_.user.path, roots_.app_dir));
	ProcessPluginManagerRoot(JoinPath(roots_.plugin_manager.path, roots_.app_dir));
	ProcessScenesRoot(JoinPath(roots_.scenes.path, roots_.app_dir));
	ProcessProfilesRoot(JoinPath(roots_.profiles.path, roots_.app_dir));
	ProcessConfigRoot(JoinPath(roots_.config.path, roots_.app_dir));
	if (!error_.ok()) {
		return {std::nullopt, error_};
	}

	AssignCollectionUids();
	AssignProfileUids();
	BuildCollectionEntries();
	if (!error_.ok()) {
		return {std::nullopt, error_};
	}
	BuildProfileEntryIdentities();
	if (stored_.size() + omitted_.size() > Limits::kEntries) {
		return {std::nullopt, MakeError(ErrorCode::Limit, "entries")};
	}

	std::sort(stored_.begin(), stored_.end(), [](const RawEntry &left, const RawEntry &right) {
		if (left.category != right.category) {
			return static_cast<int>(left.category) < static_cast<int>(right.category);
		}
		if (left.category == RawEntry::Category::Profile) {
			const std::size_t left_slash = left.path.rfind('/');
			const std::size_t right_slash = right.path.rfind('/');
			const std::string left_dir = left.path.substr(0U, left_slash);
			const std::string right_dir = right.path.substr(0U, right_slash);
			if (left_dir != right_dir) {
				return left_dir < right_dir;
			}
			const int left_rank = ProfileFileRank(left.path);
			const int right_rank = ProfileFileRank(right.path);
			if (left_rank != right_rank) {
				return left_rank < right_rank;
			}
		}
		return left.path < right.path;
	});
	ApplyStoredBudget();
	std::sort(omitted_.begin(), omitted_.end(), [](const PlannedEntry &left, const PlannedEntry &right) {
		if (left.path != right.path) {
			return left.path < right.path;
		}
		return left.root < right.root;
	});

	Plan plan;
	plan.roots = roots_;
	for (std::size_t index = 0U; index < collections_.size(); ++index) {
		const CollectionInfo &collection = collections_[index];
		if (collection.has_main) {
			plan.collections.push_back({collection.uid, collection.main_path, collection.scene_uuids});
		}
	}
	for (const ProfileInfo &profile : profiles_) {
		plan.profiles.push_back({profile.uid, profile.dir});
	}
	plan.warnings = warnings_;

	std::map<std::string, std::string> entry_ids_by_path;
	std::map<std::string, std::size_t> raw_index_by_pair;
	std::vector<RawEntry> all_stored = stored_;
	for (std::size_t index = 0U; index < all_stored.size(); ++index) {
		if (all_stored[index].pair_main) {
			raw_index_by_pair[all_stored[index].pair_key] = index;
		}
	}
	std::vector<std::size_t> raw_to_plan(all_stored.size(), kNoIndex);
	for (std::size_t index = 0U; index < all_stored.size(); ++index) {
		const RawEntry &raw = all_stored[index];
		PlannedEntry entry;
		entry.id = "e" + std::to_string(plan.entries.size() + 1U);
		entry.status = "stored";
		entry.root = raw.root;
		entry.path = raw.path;
		entry.role = raw.role;
		entry.size = raw.size;
		if ((raw.role == "collection" || raw.role == "collection.bak") && !raw.identity_uid.empty()) {
			entry.collection = raw.identity_uid;
		}
		entry.profile = raw.category == RawEntry::Category::Profile ? raw.identity_uid
									    : std::optional<std::string>{};
		raw_to_plan[index] = plan.entries.size();
		entry_ids_by_path[raw.root + '\0' + raw.path] = entry.id;
		plan.entries.push_back(std::move(entry));
	}
	for (const PlannedEntry &omitted : omitted_) {
		PlannedEntry entry = omitted;
		entry.id = "e" + std::to_string(plan.entries.size() + 1U);
		plan.entries.push_back(std::move(entry));
	}

	for (std::size_t index = 0U; index < all_stored.size(); ++index) {
		const RawEntry &raw = all_stored[index];
		if (!raw.pair_main) {
			continue;
		}
		const auto main_plan = raw_to_plan[index];
		const auto bak_id = entry_ids_by_path.find(raw.root + '\0' + raw.path + ".bak");
		if (bak_id != entry_ids_by_path.end()) {
			plan.entries[main_plan].bak = bak_id->second;
			plan.entries[main_plan].load_rule =
				raw.role == "collection" || raw.role == "profile.service" ? "bak-fallback" : "mixed";
		}
	}

	MarkCollisions(plan);
	return {std::move(plan), {}};
}

} // namespace

std::string RandomUidSource::NextUid()
{
	std::array<unsigned char, 16U> bytes{};
	try {
		std::random_device random;
		for (unsigned char &byte : bytes) {
			byte = static_cast<unsigned char>(random());
		}
	} catch (...) {
		std::mt19937 generator{std::random_device{}()};
		for (unsigned char &byte : bytes) {
			byte = static_cast<unsigned char>(generator());
		}
	}
	bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0fU) | 0x40U);
	bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3fU) | 0x80U);
	std::ostringstream output;
	output << std::hex << std::setfill('0');
	for (std::size_t index = 0U; index < bytes.size(); ++index) {
		if (index == 4U || index == 6U || index == 8U || index == 10U) {
			output << '-';
		}
		output << std::setw(2) << static_cast<unsigned int>(bytes[index]);
	}
	return output.str();
}

SeededUidSource::SeededUidSource(std::string seed) : seed_(std::move(seed)) {}

SeededUidSource::SeededUidSource(std::uint64_t seed) : seed_(std::to_string(seed)) {}

std::string SeededUidSource::NextUid()
{
	++sequence_;
	std::ostringstream output;
	output << "uid-" << seed_ << '-' << std::setfill('0') << std::setw(4) << sequence_;
	return output.str();
}

PlanResult CapturePlanner::Plan(const Fs &fs, const Roots &roots, const Manifest *parent)
{
	RandomUidSource source;
	return Plan(fs, roots, parent, source);
}

PlanResult CapturePlanner::Plan(const Fs &fs, const Roots &roots, const Manifest *parent, UidSource &uid_source)
{
	try {
		return PlannerImpl(fs, roots, parent, uid_source).Run();
	} catch (...) {
		return {std::nullopt, MakeError(ErrorCode::Json)};
	}
}

Json PlanToJson(const Plan &plan)
{
	Json result = Json::object();
	Json roots = Json::object();
	roots["config"] = plan.roots.config.custom ? "custom" : "default";
	roots["user"] = plan.roots.user.custom ? "custom" : "default";
	roots["scenes"] = plan.roots.scenes.custom ? "custom" : "default";
	roots["profiles"] = plan.roots.profiles.custom ? "custom" : "default";
	roots["plugin_manager"] = plan.roots.plugin_manager.custom ? "custom" : "default";
	result["roots"] = std::move(roots);

	Json entries = Json::array();
	for (const PlannedEntry &entry : plan.entries) {
		Json value = Json::object();
		value["id"] = entry.id;
		value["status"] = entry.status;
		value["root"] = entry.root;
		value["path"] = entry.path;
		if (entry.status == "stored") {
			value["role"] = entry.role;
			value["size"] = entry.size;
			value["fidelity"] = nullptr;
			if (entry.collection.has_value()) {
				value["collection"] = *entry.collection;
			}
			if (entry.profile.has_value()) {
				value["profile"] = *entry.profile;
			}
			if (entry.bak.has_value()) {
				value["bak"] = *entry.bak;
			}
			if (entry.load_rule.has_value()) {
				value["load_rule"] = *entry.load_rule;
			}
		} else {
			value["size"] = entry.size;
			value["reason"] = entry.reason;
			if (!entry.kind.empty()) {
				value["kind"] = entry.kind;
			}
			if (entry.count != 0U) {
				value["count"] = entry.count;
			}
		}
		if (entry.collision) {
			value["collision"] = true;
		}
		entries.push_back(std::move(value));
	}
	result["entries"] = std::move(entries);

	Json collections = Json::array();
	for (const PlannedCollection &collection : plan.collections) {
		collections.push_back(
			{{"uid", collection.uid}, {"path", collection.path}, {"scene_uuids", collection.scene_uuids}});
	}
	result["collections"] = std::move(collections);

	Json profiles = Json::array();
	for (const PlannedProfile &profile : plan.profiles) {
		profiles.push_back({{"uid", profile.uid}, {"dir", profile.dir}});
	}
	result["profiles"] = std::move(profiles);

	Json warnings = Json::array();
	for (const PlanWarning &warning : plan.warnings) {
		warnings.push_back({{"code", warning.code}, {"path", warning.path}});
	}
	result["warnings"] = std::move(warnings);
	return result;
}

} // namespace Jeiobs::Workspace
