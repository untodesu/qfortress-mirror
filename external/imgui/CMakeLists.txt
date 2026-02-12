add_library(external_imgui STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imconfig.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_stdlib.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_rectpack.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_textedit.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imstb_truetype.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_internal.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_stdlib.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_internal.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_widgets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui.cpp")
target_include_directories(external_imgui PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(external_imgui PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(external_imgui_sdl3 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdl3.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_impl_sdl3.cpp")
target_link_libraries(external_imgui_sdl3 PUBLIC external_imgui)
target_link_libraries(external_imgui_sdl3 PUBLIC external_SDL3)

add_library(external_imgui_opengl3 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_opengl3_loader.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_impl_opengl3.cpp")
target_link_libraries(external_imgui_opengl3 PUBLIC external_imgui)

add_library(external_imgui_sdlgpu3 STATIC
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdlgpu3.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/imgui_impl_sdlgpu3_shaders.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/imgui_impl_sdlgpu3.cpp")
target_link_libraries(external_imgui_sdlgpu3 PUBLIC external_imgui)
target_link_libraries(external_imgui_sdlgpu3 PUBLIC external_SDL3)

add_library(external::imgui ALIAS external_imgui)
add_library(external::imgui-backend-sdl3 ALIAS external_imgui_sdl3)
add_library(external::imgui-backend-opengl3 ALIAS external_imgui_opengl3)
add_library(external::imgui-backend-sdlgpu3 ALIAS external_imgui_sdlgpu3)
