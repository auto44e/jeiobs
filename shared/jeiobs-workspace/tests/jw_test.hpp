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
#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

namespace JwTest {

inline int failures = 0;

inline void Check(bool condition, const char *expression, const char *file, int line)
{
	if (!condition) {
		std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
		++failures;
	}
}

} // namespace JwTest

#define JW_CHECK(expression) JwTest::Check((expression), #expression, __FILE__, __LINE__)
#define JW_CHECK_EQ(actual, expected) JW_CHECK((actual) == (expected))
