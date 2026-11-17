/*
 * JeiOBS control graph support.
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

namespace JcTest {

inline int failures = 0;

inline void Check(bool condition, const char *expression, const char *file, int line)
{
	if (!condition) {
		std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
		++failures;
	}
}

} // namespace JcTest

#define JC_CHECK(expression) JcTest::Check((expression), #expression, __FILE__, __LINE__)
#define JC_CHECK_EQ(actual, expected) JC_CHECK((actual) == (expected))
