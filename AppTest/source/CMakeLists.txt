add_executable( apptest 
    main.cpp 
    commandline.cpp
)

target_include_directories( apptest 
 PUBLIC ${PROJECT_SOURCE_DIR}/include
 PRIVATE ${PROJECT_SOURCE_DIR}/external/clipp/include
)

target_link_libraries( apptest
    PUBLIC ftxui::component
    PUBLIC ftxui::dom
    PUBLIC ftxui::screen 
    
)