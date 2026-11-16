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
#include "jeiobs/workspace/Error.hpp"

namespace Jeiobs::Workspace {

const char *ErrorCodeName(ErrorCode code) noexcept
{
	switch (code) {
	case ErrorCode::None:
		return "";
	case ErrorCode::Io:
		return "E_IO";
	case ErrorCode::Limit:
		return "E_LIMIT";
	case ErrorCode::Json:
		return "E_JSON";
	case ErrorCode::DuplicateKey:
		return "E_DUPKEY";
	case ErrorCode::Utf8:
		return "E_UTF8";
	case ErrorCode::Nul:
		return "E_NUL";
	case ErrorCode::Nonfinite:
		return "E_NONFINITE";
	case ErrorCode::Bom:
		return "E_BOM";
	case ErrorCode::Schema:
		return "E_SCHEMA";
	case ErrorCode::Path:
		return "E_PATH";
	case ErrorCode::Collision:
		return "E_COLLISION";
	case ErrorCode::Incompat:
		return "E_INCOMPAT";
	case ErrorCode::Major:
		return "E_MAJOR";
	case ErrorCode::RoCompat:
		return "E_RO_COMPAT";
	case ErrorCode::Undecided:
		return "E_UNDECIDED";
	case ErrorCode::Name:
		return "E_NAME";
	case ErrorCode::Revid:
		return "E_REVID";
	case ErrorCode::Seq:
		return "E_SEQ";
	}
	return "E_JSON";
}

} // namespace Jeiobs::Workspace
