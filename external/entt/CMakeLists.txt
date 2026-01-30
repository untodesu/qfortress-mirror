add_library(external_entt INTERFACE)
target_include_directories(external_entt INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(external::entt ALIAS external_entt)
