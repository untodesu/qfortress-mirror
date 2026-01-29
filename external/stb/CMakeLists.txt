add_library(external_stb STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/stb_image.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/stb_image_write.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/stb_impl.c")
target_compile_definitions(external_stb PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(external_stb PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(external_stb PROPERTIES FOLDER Dependencies)

add_library(external::stb ALIAS external_stb)
