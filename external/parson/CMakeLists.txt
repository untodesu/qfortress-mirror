add_library(external_parson STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/parson.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/parson.c")
target_include_directories(external_parson PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(external::parson ALIAS external_parson)
