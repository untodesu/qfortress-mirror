add_library(external_uulog STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/uulog.hh"
    "${CMAKE_CURRENT_LIST_DIR}/src/uulog.cc")
target_compile_features(external_uulog PUBLIC cxx_std_20)
target_include_directories(external_uulog PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(external::uulog ALIAS external_uulog)
