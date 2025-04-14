cmake_minimum_required(VERSION 3.5...3.8)


### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

project(ChallengeTests VERSION 1.0
        DESCRIPTION "Multiple small programs to run tests for each challenge."
        LANGUAGES CXX)

set_property(GLOBAL PROPERTY CMAKE_CXX_STANDARD  23)
set_property(GLOBAL PROPERTY CMAKE_CXX_STANDARD_REQUIRED On)
set_property(GLOBAL PROPERTY CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

add_subdirectory(external)

set_property(GLOBAL PROPERTY CMAKE_EXPORT_COMPILE_COMMANDS On)


add_subdirectory(Challenge01)
add_subdirectory(Challenge03)
