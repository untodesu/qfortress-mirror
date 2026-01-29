add_library(core STATIC
    "${CMAKE_CURRENT_LIST_DIR}/config/arithmetic.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config/boolean.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/boolean.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config/map.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/map.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config/string.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/string.hh"
    "${CMAKE_CURRENT_LIST_DIR}/config/value.cc"
    "${CMAKE_CURRENT_LIST_DIR}/config/value.hh"
    "${CMAKE_CURRENT_LIST_DIR}/res/image.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/image.hh"
    "${CMAKE_CURRENT_LIST_DIR}/res/resource.cc"
    "${CMAKE_CURRENT_LIST_DIR}/res/resource.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/physfs.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/physfs.hh"
    "${CMAKE_CURRENT_LIST_DIR}/utils/string.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/string.hh"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/version.hh")
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(core PUBLIC
    external::enet
    external::stb
    external::physfs
    external::uulog)
target_precompile_headers(core PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

configure_file("${CMAKE_CURRENT_LIST_DIR}/version.cc.in" "${PROJECT_BINARY_DIR}/core.generated_version.cc")
target_sources(core PRIVATE "${PROJECT_BINARY_DIR}/core.generated_version.cc")

if(WIN32)
    target_compile_definitions(core PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(core PUBLIC _USE_MATH_DEFINES)
    target_compile_definitions(core PUBLIC NOMINMAX)
endif()
