add_library(external_eigen INTERFACE)
target_include_directories(external_eigen INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(external::eigen ALIAS external_eigen)
