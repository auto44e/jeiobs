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
#include "jeiobs/workspace/Hash.hpp"

#include <blake2.h>

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Jeiobs::Workspace {
namespace {

constexpr std::size_t kDigestBytes = 32U;
constexpr std::size_t kReadBufferBytes = 64U * 1024U;

std::string DigestToHex(const std::array<unsigned char, kDigestBytes> &digest)
{
	std::ostringstream output;
	output << std::hex << std::setfill('0');
	for (const unsigned char byte : digest) {
		output << std::setw(2) << static_cast<unsigned int>(byte);
	}
	return output.str();
}

HashResult Finalize(blake2b_state &state)
{
	std::array<unsigned char, kDigestBytes> digest{};
	if (blake2b_final(&state, digest.data(), digest.size()) != 0) {
		return {{}, {ErrorCode::Io, {}}};
	}
	return {DigestToHex(digest), {}};
}

} // namespace

HashResult HashBytes(std::string_view bytes)
{
	blake2b_state state{};
	if (blake2b_init(&state, kDigestBytes) != 0 || blake2b_update(&state, bytes.data(), bytes.size()) != 0) {
		return {{}, {ErrorCode::Io, {}}};
	}
	return Finalize(state);
}

HashResult HashFile(const std::string &path)
{
	std::ifstream input(path, std::ios::in | std::ios::binary);
	if (!input.is_open()) {
		return {{}, {ErrorCode::Io, {}}};
	}

	blake2b_state state{};
	if (blake2b_init(&state, kDigestBytes) != 0) {
		return {{}, {ErrorCode::Io, {}}};
	}

	std::array<char, kReadBufferBytes> buffer{};
	while (input.good()) {
		input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		const std::streamsize count = input.gcount();
		if (count > 0 && blake2b_update(&state, buffer.data(), static_cast<std::size_t>(count)) != 0) {
			return {{}, {ErrorCode::Io, {}}};
		}
	}
	if (!input.eof()) {
		return {{}, {ErrorCode::Io, {}}};
	}
	return Finalize(state);
}

} // namespace Jeiobs::Workspace
