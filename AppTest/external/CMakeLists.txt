include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ftxui"
  GIT_PROGRESS True
)


FetchContent_MakeAvailable(ftxui)


FetchContent_Declare( clipp 
    GIT_REPOSITORY https://github.com/GerHobbelt/clipp
    GIT_TAG master
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/clipp"
    GIT_PROGRESS True
    EXCLUDE_FROM_ALL "examples/"
)

FetchContent_MakeAvailable( clipp )
