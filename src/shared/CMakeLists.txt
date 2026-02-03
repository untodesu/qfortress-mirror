add_library(shared STATIC
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(shared PUBLIC cxx_std_20)
target_include_directories(shared PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(shared PUBLIC core)
target_precompile_headers(shared PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
