/*
 * jeiobs-workspace: internal helper (not installed).
 * Paths reach the library as UTF-8; on Windows std::ifstream(std::string) would read them in the
 * ANSI code page instead.
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace Jeiobs::Workspace::Detail {

// Opens a UTF-8 path for binary reading; on a path the platform cannot represent, the stream stays
// closed and the caller reports E_IO.
inline void OpenUtf8(std::ifstream &input, const std::string &path)
{
	try {
		input.open(std::filesystem::u8path(path), std::ios::in | std::ios::binary);
	} catch (const std::exception &) {
		input.close();
	}
}

} // namespace Jeiobs::Workspace::Detail
