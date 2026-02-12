find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

add_library(render_modern STATIC
    "${CMAKE_CURRENT_LIST_DIR}/gpu/static_buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/static_buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/stream_buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/gpu/stream_buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/experimental.cc"
    "${CMAKE_CURRENT_LIST_DIR}/experimental.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/impl_backend.cc"
    "${CMAKE_CURRENT_LIST_DIR}/impl_frontend.cc"
    "${CMAKE_CURRENT_LIST_DIR}/impl_texture2D.cc"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh")
target_compile_features(render_modern PUBLIC cxx_std_20)
target_compile_definitions(render_modern PRIVATE QF_CLIENT_MODERN)
target_include_directories(render_modern PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(render_modern PUBLIC
    external::imgui-backend-sdlgpu3)
target_link_libraries(render_modern PUBLIC game_client)
target_precompile_headers(render_modern PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

function(compile_spirv_shader shader_source)
    get_filename_component(shader_source_name "${shader_source}" NAME)
    set(shader_binary_path "${PROJECT_BINARY_DIR}/generated.${shader_source_name}.spirv")
    set(shader_cxxres_path "${PROJECT_BINARY_DIR}/generated.${shader_source_name}.cc")

    # Create a valid C++ identifier for the shader binary
    set(shader_cxx_identifier "spirv_${shader_source_name}")
    string(REGEX REPLACE "\\.glsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(REGEX REPLACE "\\.hlsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(MAKE_C_IDENTIFIER "${shader_cxx_identifier}" shader_cxx_identifier)

    add_custom_command(OUTPUT "${shader_binary_path}"
        COMMAND ${GLSLANG_VALIDATOR} -e main -V -o "${shader_binary_path}" "${shader_source}"
        DEPENDS ${shader_source}
        COMMENT "Compiling SPIR-V shader: ${shader_source_name}"
        VERBATIM)

    add_custom_command(OUTPUT "${shader_cxxres_path}"
        COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/generate-binarray.py" "${shader_binary_path}" "${shader_cxxres_path}" "${shader_cxx_identifier}"
        DEPENDS "${shader_binary_path}"
        COMMENT "Generating C++ source file for shader binary: ${shader_source_name} [${shader_cxx_identifier}]"
        VERBATIM)

    target_sources(render_modern PRIVATE "${shader_cxxres_path}")
    set_source_files_properties("${shader_cxxres_path}" PROPERTIES GENERATED TRUE)
endfunction()

compile_spirv_shader("${CMAKE_CURRENT_LIST_DIR}/hlsl/experimental.vert.hlsl")
compile_spirv_shader("${CMAKE_CURRENT_LIST_DIR}/hlsl/experimental.frag.hlsl")
