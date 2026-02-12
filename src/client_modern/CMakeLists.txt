find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

add_library(client_modern STATIC
    "${CMAKE_CURRENT_LIST_DIR}/utils/buffer.cc"
    "${CMAKE_CURRENT_LIST_DIR}/utils/buffer.hh"
    "${CMAKE_CURRENT_LIST_DIR}/experimental.cc"
    "${CMAKE_CURRENT_LIST_DIR}/experimental.hh"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cc"
    "${CMAKE_CURRENT_LIST_DIR}/globals.hh"
    "${CMAKE_CURRENT_LIST_DIR}/pch.hh"
    "${CMAKE_CURRENT_LIST_DIR}/render.cc")
target_compile_features(client_modern PUBLIC cxx_std_20)
target_include_directories(client_modern PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(client_modern PUBLIC client)
target_precompile_headers(client_modern PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pch.hh")

function(compile_spirv_shader shader_source)
    get_filename_component(shader_source_name "${shader_source}" NAME)
    set(shader_binary_path "${PROJECT_BINARY_DIR}/generated.${shader_source_name}.spirv")
    set(shader_cxxres_path "${PROJECT_BINARY_DIR}/generated.${shader_source_name}.cc")

    # Create a valid C++ identifier for the shader binary
    set(shader_cxx_identifier "spirv_${shader_source_name}")
    string(REGEX REPLACE "\\.glsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(REGEX REPLACE "\\.hlsl$" "" shader_cxx_identifier "${shader_cxx_identifier}")
    string(REGEX REPLACE "[-.]" "_" shader_cxx_identifier "${shader_cxx_identifier}")

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

    target_sources(client_modern PRIVATE "${shader_cxxres_path}")
    set_source_files_properties("${shader_cxxres_path}" PROPERTIES GENERATED TRUE)
endfunction()

compile_spirv_shader("${CMAKE_CURRENT_LIST_DIR}/shaders/experimental.vert.hlsl")
compile_spirv_shader("${CMAKE_CURRENT_LIST_DIR}/shaders/experimental.frag.hlsl")
