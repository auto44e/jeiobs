cmake_minimum_required(VERSION 3.28)

project(jeiobs-workspace LANGUAGES C CXX)

if(NOT TARGET OBS::blake2)
	add_subdirectory(../../deps/blake2 ${CMAKE_CURRENT_BINARY_DIR}/blake2)
endif()

find_package(nlohmann_json 3.11 REQUIRED)

set(JW_LIBRARY_SOURCES
	src/Error.cpp
	src/Hash.cpp
	src/IniScan.cpp
	src/JsonScan.cpp
	src/Manifest.cpp
	src/PathRules.cpp
	src/Redactor.cpp
)

add_library(jeiobs-workspace STATIC ${JW_LIBRARY_SOURCES})
add_library(JeiOBS::workspace ALIAS jeiobs-workspace)

target_compile_features(jeiobs-workspace PUBLIC cxx_std_17)
target_include_directories(jeiobs-workspace
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(jeiobs-workspace
	PUBLIC
		OBS::blake2
		nlohmann_json::nlohmann_json
)

if(MSVC)
	target_compile_options(jeiobs-workspace PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jeiobs-workspace PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(jw-check src/jw-check.cpp)
target_compile_features(jw-check PRIVATE cxx_std_17)
target_link_libraries(jw-check PRIVATE jeiobs-workspace)

if(MSVC)
	target_compile_options(jw-check PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jw-check PRIVATE -Wall -Wextra -Werror)
endif()

enable_testing()
add_executable(jw-unit
	tests/jw_unit.cpp
)
target_compile_features(jw-unit PRIVATE cxx_std_17)
target_link_libraries(jw-unit PRIVATE jeiobs-workspace)
target_include_directories(jw-unit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")

if(MSVC)
	target_compile_options(jw-unit PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jw-unit PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME jw-unit COMMAND jw-unit)

add_executable(jw-fuzz-smoke
	tests/jw_fuzz_smoke.cpp
)
target_compile_features(jw-fuzz-smoke PRIVATE cxx_std_17)
target_link_libraries(jw-fuzz-smoke PRIVATE jeiobs-workspace)
target_compile_definitions(jw-fuzz-smoke PRIVATE
	JW_CASES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/cases"
)

if(MSVC)
	target_compile_options(jw-fuzz-smoke PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jw-fuzz-smoke PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME jw-fuzz-smoke COMMAND jw-fuzz-smoke)

option(JW_ENABLE_LIBFUZZER "Build the optional Clang libFuzzer target" OFF)
if(JW_ENABLE_LIBFUZZER AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_executable(jw-fuzz tests/jw_fuzz.cpp)
	target_compile_features(jw-fuzz PRIVATE cxx_std_17)
	target_link_libraries(jw-fuzz PRIVATE jeiobs-workspace)
	target_compile_options(jw-fuzz PRIVATE -Wall -Wextra -Werror -fsanitize=fuzzer,address,undefined)
	target_link_options(jw-fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/run-cases.cmake")
	add_test(
		NAME jw-cases
		COMMAND ${CMAKE_COMMAND}
			-DJW_CHECK=$<TARGET_FILE:jw-check>
			-DCASES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/cases
			-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/cases-work
			-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/run-cases.cmake
	)
endif()
