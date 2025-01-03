include(FetchContent)



#FetchContent_Declare(ftxui
#  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
#  GIT_TAG main
#  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ftxui"
#  GIT_PROGRESS True
#)


#FetchContent_MakeAvailable(ftxui)



FetchContent_Declare( clipp 
    GIT_REPOSITORY https://github.com/GerHobbelt/clipp
    GIT_TAG master
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/clipp"
    GIT_PROGRESS True
    EXCLUDE_FROM_ALL "examples/"
)

FetchContent_MakeAvailable( clipp )

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_PROGRESS True
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus"
    EXCLUDE_FROM_ALL "examples/"
    EXCLUDE_FROM_ALL "tests/"
    EXCLUDE_FROM_ALL "vendor/"
)
FetchContent_MakeAvailable(tomlplusplus)
