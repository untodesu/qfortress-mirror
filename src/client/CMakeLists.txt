add_library(client STATIC
    "${CMAKE_CURRENT_LIST_DIR}/res/texture2D.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/main.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/video.cc"
    "${CMAKE_CURRENT_LIST_DIR}/video.hh")
target_compile_features(client PUBLIC cxx_std_20)
target_include_directories(client PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_precompile_headers(client PUBLIC "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(client PUBLIC shared external::SDL3)
