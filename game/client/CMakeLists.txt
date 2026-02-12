add_library(game_client STATIC
    "${CMAKE_CURRENT_LIST_DIR}/config/key_binding.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/key_binding.hh"
    "${CMAKE_CURRENT_LIST_DIR}/game.cc"
    "${CMAKE_CURRENT_LIST_DIR}/game.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/main.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/video.cc"
    "${CMAKE_CURRENT_LIST_DIR}/video.hh")
target_compile_features(game_client PUBLIC cxx_std_20)
target_include_directories(game_client PUBLIC "${PROJECT_SOURCE_DIR}")
target_precompile_headers(game_client PUBLIC "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(game_client PUBLIC
    external::imgui
    external::imgui-backend-sdl3
    external::SDL3)
target_link_libraries(game_client PUBLIC game_shared)
