cmake_minimum_required(VERSION 3.2...3.8)

add_executable( test01
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

target_include_directories( test01
 PUBLIC include
 PRIVATE ${PROJECT_SOURCE_DIR}/external/clipp/include
 PRIVATE ${PROJECT_SOURCE_DIR}/external/tomlplusplus/include
)


target_link_libraries( test01
    PRIVATE
      subprocess
      reproc++
)
