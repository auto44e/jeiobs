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

#include "jeiobs/workspace/Manifest.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Jeiobs::Workspace {

enum class FsEntryKind {
	File,
	Directory,
	Symlink,
	Other,
};

struct FsEntry {
	std::string name;
	FsEntryKind kind = FsEntryKind::Other;
	std::uint64_t size = 0U;
	bool name_representable = true;
};

struct FsListResult {
	std::vector<FsEntry> entries;
	Error error;

	[[nodiscard]] bool ok() const noexcept { return error.ok(); }
};

using FsListCallback = std::function<bool(const FsEntry &)>;

class Fs {
public:
	virtual ~Fs() = default;

	[[nodiscard]] virtual FsListResult List(std::string_view directory) const = 0;
	[[nodiscard]] virtual BytesResult ReadBounded(std::string_view path, std::size_t max_bytes) const = 0;

	// Returning false from the callback stops enumeration successfully. This lets callers collapse
	// large directories without materializing all their entries.
	[[nodiscard]] virtual Error ListEach(std::string_view directory, const FsListCallback &callback) const;
};

class MemFs final : public Fs {
public:
	struct Node;

	MemFs();
	~MemFs() override;

	[[nodiscard]] FsListResult List(std::string_view directory) const override;
	[[nodiscard]] BytesResult ReadBounded(std::string_view path, std::size_t max_bytes) const override;
	[[nodiscard]] Error ListEach(std::string_view directory, const FsListCallback &callback) const override;

	[[nodiscard]] Error AddDirectory(std::string_view path);
	[[nodiscard]] Error AddFile(std::string_view path, std::string_view bytes);
	[[nodiscard]] Error AddSymlink(std::string_view path);

	[[nodiscard]] Error PutDirectory(std::string_view path) { return AddDirectory(path); }
	[[nodiscard]] Error PutFile(std::string_view path, std::string_view bytes) { return AddFile(path, bytes); }
	[[nodiscard]] Error PutSymlink(std::string_view path) { return AddSymlink(path); }

private:
	std::unique_ptr<Node> root_;
};

class RealFs final : public Fs {
public:
	[[nodiscard]] FsListResult List(std::string_view directory) const override;
	[[nodiscard]] BytesResult ReadBounded(std::string_view path, std::size_t max_bytes) const override;
	[[nodiscard]] Error ListEach(std::string_view directory, const FsListCallback &callback) const override;
};

} // namespace Jeiobs::Workspace
