/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or later.
 */
#include "jeiobs/workspace/Fs.hpp"

#include "Utf8Path.hpp"
#include "jeiobs/workspace/Limits.hpp"

#include <limits>
#include <filesystem>
#include <fstream>
#include <map>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Jeiobs::Workspace {
namespace {

std::vector<std::string> SplitPath(std::string_view path)
{
	std::vector<std::string> components;
	std::size_t start = 0U;
	while (start < path.size()) {
		const std::size_t slash = path.find('/', start);
		const std::size_t end = slash == std::string_view::npos ? path.size() : slash;
		if (end > start) {
			components.emplace_back(path.substr(start, end - start));
		}
		if (slash == std::string_view::npos) {
			break;
		}
		start = slash + 1U;
	}
	return components;
}

bool IsRegular(const std::filesystem::file_status &status)
{
	return std::filesystem::is_regular_file(status);
}

bool IsReparsePoint(const std::filesystem::path &path)
{
#ifdef _WIN32
	const DWORD attributes = GetFileAttributesW(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U;
#else
	(void)path;
	return false;
#endif
}

FsEntryKind EntryKind(const std::filesystem::path &path, const std::filesystem::file_status &status)
{
	if (std::filesystem::is_symlink(status) || IsReparsePoint(path)) {
		return FsEntryKind::Symlink;
	}
	if (std::filesystem::is_regular_file(status)) {
		return FsEntryKind::File;
	}
	if (std::filesystem::is_directory(status)) {
		return FsEntryKind::Directory;
	}
	return FsEntryKind::Other;
}

std::uint64_t FileSize(const std::filesystem::path &path, const std::filesystem::file_status &status)
{
	if (!IsRegular(status)) {
		return 0U;
	}
	std::error_code error;
	const std::uintmax_t size = std::filesystem::file_size(path, error);
	if (error || size > std::numeric_limits<std::uint64_t>::max()) {
		return 0U;
	}
	return static_cast<std::uint64_t>(size);
}

} // namespace

Error Fs::ListEach(std::string_view directory, const FsListCallback &callback) const
{
	const FsListResult listed = List(directory);
	if (!listed.ok()) {
		return listed.error;
	}
	for (const FsEntry &entry : listed.entries) {
		if (!callback(entry)) {
			break;
		}
	}
	return {};
}

struct MemFs::Node {
	FsEntryKind kind = FsEntryKind::Directory;
	std::string bytes;
	std::map<std::string, Node> children;
};

namespace {

MemFs::Node *FindNode(MemFs::Node *root, std::string_view path)
{
	MemFs::Node *node = root;
	for (const std::string &component : SplitPath(path)) {
		const auto iterator = node->children.find(component);
		if (iterator == node->children.end()) {
			return nullptr;
		}
		node = &iterator->second;
	}
	return node;
}

Error AddMemNode(MemFs::Node *root, std::string_view path, FsEntryKind kind, std::string_view bytes)
{
	const std::vector<std::string> components = SplitPath(path);
	if (components.empty()) {
		return {ErrorCode::Schema, "path"};
	}
	MemFs::Node *parent = root;
	for (std::size_t index = 0U; index + 1U < components.size(); ++index) {
		MemFs::Node &child = parent->children[components[index]];
		if (child.kind != FsEntryKind::Directory) {
			return {ErrorCode::Io, "not-directory"};
		}
		parent = &child;
	}
	MemFs::Node &node = parent->children[components.back()];
	if (!node.children.empty() && kind != FsEntryKind::Directory) {
		return {ErrorCode::Io, "not-empty"};
	}
	if (kind == FsEntryKind::Directory && node.kind != FsEntryKind::Directory && !node.children.empty()) {
		return {ErrorCode::Io, "not-directory"};
	}
	node.kind = kind;
	if (kind == FsEntryKind::File) {
		node.bytes.assign(bytes);
		node.children.clear();
	} else {
		node.bytes.clear();
	}
	return {};
}

FsListResult ListMemNode(const MemFs::Node *node)
{
	if (node == nullptr) {
		return {{}, {ErrorCode::Io, "missing"}};
	}
	if (node->kind == FsEntryKind::Symlink) {
		return {{}, {ErrorCode::Io, "symlink"}};
	}
	if (node->kind != FsEntryKind::Directory) {
		return {{}, {ErrorCode::Io, "not-directory"}};
	}
	FsListResult result;
	result.entries.reserve(node->children.size());
	for (const auto &[name, child] : node->children) {
		result.entries.push_back(
			{name, child.kind,
			 child.kind == FsEntryKind::File ? static_cast<std::uint64_t>(child.bytes.size()) : 0U});
	}
	return result;
}

} // namespace

MemFs::MemFs() : root_(std::make_unique<Node>()) {}

MemFs::~MemFs() = default;

FsListResult MemFs::List(std::string_view directory) const
{
	return ListMemNode(FindNode(root_.get(), directory));
}

BytesResult MemFs::ReadBounded(std::string_view path, std::size_t max_bytes) const
{
	const Node *node = FindNode(root_.get(), path);
	if (node == nullptr || node->kind != FsEntryKind::File) {
		return {{}, {ErrorCode::Io, {}}};
	}
	if (node->bytes.size() > max_bytes) {
		return {{}, {ErrorCode::Limit, "blob"}};
	}
	return {node->bytes, {}};
}

Error MemFs::ListEach(std::string_view directory, const FsListCallback &callback) const
{
	const Node *node = FindNode(root_.get(), directory);
	if (node == nullptr) {
		return {ErrorCode::Io, "missing"};
	}
	if (node->kind == FsEntryKind::Symlink) {
		return {ErrorCode::Io, "symlink"};
	}
	if (node->kind != FsEntryKind::Directory) {
		return {ErrorCode::Io, "not-directory"};
	}
	for (const auto &[name, child] : node->children) {
		const FsEntry entry{name, child.kind,
				    child.kind == FsEntryKind::File ? static_cast<std::uint64_t>(child.bytes.size())
								    : 0U};
		if (!callback(entry)) {
			break;
		}
	}
	return {};
}

Error MemFs::AddDirectory(std::string_view path)
{
	return AddMemNode(root_.get(), path, FsEntryKind::Directory, {});
}

Error MemFs::AddFile(std::string_view path, std::string_view bytes)
{
	return AddMemNode(root_.get(), path, FsEntryKind::File, bytes);
}

Error MemFs::AddSymlink(std::string_view path)
{
	return AddMemNode(root_.get(), path, FsEntryKind::Symlink, {});
}

FsListResult RealFs::List(std::string_view directory) const
{
	FsListResult result;
	result.error = ListEach(directory, [&result](const FsEntry &entry) {
		result.entries.push_back(entry);
		return true;
	});
	return result;
}

Error RealFs::ListEach(std::string_view directory, const FsListCallback &callback) const
{
	try {
		const std::filesystem::path path = std::filesystem::u8path(std::string(directory));
		std::error_code status_error;
		const std::filesystem::file_status directory_status =
			std::filesystem::symlink_status(path, status_error);
		if (status_error) {
			return {ErrorCode::Io, "missing"};
		}
		if (EntryKind(path, directory_status) == FsEntryKind::Symlink) {
			return {ErrorCode::Io, "symlink"};
		}
		if (!std::filesystem::is_directory(directory_status)) {
			return {ErrorCode::Io, "not-directory"};
		}

		std::error_code iterator_error;
		std::filesystem::directory_iterator iterator(path, iterator_error);
		if (iterator_error) {
			return {ErrorCode::Io, "listing"};
		}
		while (iterator != std::filesystem::directory_iterator{}) {
			const std::filesystem::directory_entry entry = *iterator;
			std::error_code entry_error;
			const std::filesystem::file_status status = entry.symlink_status(entry_error);
			if (entry_error) {
				return {ErrorCode::Io, "listing"};
			}
			std::string name;
			bool name_representable = true;
			try {
				name = entry.path().filename().u8string();
			} catch (...) {
				name_representable = false;
			}
			const FsEntry item{name, EntryKind(entry.path(), status), FileSize(entry.path(), status),
					   name_representable};
			if (!callback(item)) {
				break;
			}
			iterator.increment(iterator_error);
			if (iterator_error) {
				return {ErrorCode::Io, "listing"};
			}
		}
		return {};
	} catch (...) {
		return {ErrorCode::Io, {}};
	}
}

BytesResult RealFs::ReadBounded(std::string_view path, std::size_t max_bytes) const
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(std::string(path));
		std::error_code status_error;
		const std::filesystem::file_status status =
			std::filesystem::symlink_status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return {{}, {ErrorCode::Io, {}}};
		}
		std::error_code size_error;
		const std::uintmax_t file_size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error) {
			return {{}, {ErrorCode::Io, {}}};
		}
		if (file_size > max_bytes || file_size > std::numeric_limits<std::size_t>::max()) {
			return {{}, {ErrorCode::Limit, "blob"}};
		}

		std::ifstream input;
		Detail::OpenUtf8(input, std::string(path));
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

} // namespace Jeiobs::Workspace
