add_executable(geomp
    "${CMAKE_CURRENT_LIST_DIR}/main.cc"
    "${CMAKE_CURRENT_LIST_DIR}/mapfile.cc"
    "${CMAKE_CURRENT_LIST_DIR}/mapfile.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(geomp PUBLIC cxx_std_20)
target_include_directories(geomp PUBLIC "${PROJECT_SOURCE_DIR}")
target_precompile_headers(geomp PUBLIC "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_link_libraries(geomp PUBLIC core)
