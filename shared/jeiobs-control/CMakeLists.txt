cmake_minimum_required(VERSION 3.28)

project(jeiobs-control LANGUAGES CXX)

find_package(nlohmann_json 3.11 REQUIRED)

set(JC_LIBRARY_SOURCES
	src/Control.cpp
	src/Value.cpp
	src/Mapping.cpp
)

add_library(jeiobs-control STATIC ${JC_LIBRARY_SOURCES})
add_library(JeiOBS::control ALIAS jeiobs-control)

target_compile_features(jeiobs-control PUBLIC cxx_std_17)
target_include_directories(jeiobs-control
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(jeiobs-control PUBLIC nlohmann_json::nlohmann_json)

if(MSVC)
	target_compile_options(jeiobs-control PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jeiobs-control PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(jc-check src/jc-check.cpp)
target_compile_features(jc-check PRIVATE cxx_std_17)
target_link_libraries(jc-check PRIVATE jeiobs-control)

if(MSVC)
	target_compile_options(jc-check PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jc-check PRIVATE -Wall -Wextra -Werror)
endif()

enable_testing()

add_executable(jc-unit tests/jc_unit.cpp)
target_compile_features(jc-unit PRIVATE cxx_std_17)
target_link_libraries(jc-unit PRIVATE jeiobs-control)
target_include_directories(jc-unit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")

if(MSVC)
	target_compile_options(jc-unit PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jc-unit PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME jc-unit COMMAND jc-unit)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/run-cases.cmake")
	add_test(
		NAME jc-cases
		COMMAND ${CMAKE_COMMAND}
			-DJC_CHECK=$<TARGET_FILE:jc-check>
			-DCASES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/cases
			-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/cases-work
			-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/run-cases.cmake
	)
endif()
