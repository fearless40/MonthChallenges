cmake_minimum_required(VERSION 3.2...3.8)

add_executable(test01
    source/main.cpp
    source/commandline.cpp
    source/generatetest.cpp
    source/TestConfig.cpp
    source/RowCol.cpp
    source/TestDefinition.cpp
    source/TestConfigTOML.cpp
    source/runtests.cpp
    source/base26.cpp
)

if(MSVC_VERSION GREATER_EQUAL "1900")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(/std:c++20 _cpp_20_flag_supported)
  if(_cpp_20_flag_supported)
    target_compile_options(test01 PRIVATE
      /std:c++20
    )
  endif()
  target_compile_options(test01 PRIVATE /MP)
else()
  target_compile_options(test01 PRIVATE
  -std=c++20
  #-Wall
  #-Werror
  -Wshadow
  -Wconversion
  #-Wextra
  )
endif()

target_include_directories(test01
  PUBLIC include
  PRIVATE ${PROJECT_SOURCE_DIR}/external/clipp/include
  PRIVATE ${PROJECT_SOURCE_DIR}/external/tomlplusplus/include
)


target_link_libraries(test01
    PRIVATE
      subprocess
      reproc++
    INTERFACE
      clipp
      tomlplusplus
)
