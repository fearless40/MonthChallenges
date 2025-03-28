cmake_minimum_required(VERSION 3.3...4.0)

add_executable(tester03
  src/main.cpp
)

if(MSVC_VERSION GREATER_EQUAL "1900")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(/std:c++20 _cpp_20_flag_supported)
  if(_cpp_20_flag_supported)
    target_compile_options(tester03 PRIVATE
      /std:c++20
    )
  endif()
  target_compile_options(tester03 PRIVATE /MP)
else()
  target_compile_options(tester03 PRIVATE
  -std=c++23
  #-Wall
  #-Werror
  -Wshadow
  -Wconversion
  #-Wextra
  )
endif()

target_include_directories(tester03 PUBLIC include)

target_link_libraries(tester03
  INTERFACE clipp
  PRIVATE reproc++
)

