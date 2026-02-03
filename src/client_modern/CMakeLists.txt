find_program(GLSLANG_VALIDATOR glslangValidator)

add_library(client_modern STATIC
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/render.cc")
target_compile_features(client_modern PUBLIC cxx_std_20)
target_include_directories(client_modern PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_include_directories(client_modern PRIVATE "${PROJECT_BINARY_DIR}/shaders")
target_link_libraries(client_modern PUBLIC client)
target_precompile_headers(client_modern PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
