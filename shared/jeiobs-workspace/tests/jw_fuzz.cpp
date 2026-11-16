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
#include "jeiobs/workspace/Redactor.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
	const std::string_view bytes(reinterpret_cast<const char *>(data), size);
	const char *roles[] = {"profile.service",        "profile.basic",          "module.obs-websocket", "collection",
			       "profile.encoder.stream", "profile.encoder.record", "app.modules",          "app.user"};
	for (const char *role : roles) {
		(void)Jeiobs::Workspace::Redact(role, bytes);
	}
	return 0;
}
