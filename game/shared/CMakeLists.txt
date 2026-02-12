add_library(game_shared STATIC
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(game_shared PUBLIC cxx_std_20)
target_include_directories(game_shared PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(game_shared PUBLIC core)
target_precompile_headers(game_shared PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
