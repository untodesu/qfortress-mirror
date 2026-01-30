function(add_main_executable target_name)
    add_executable(${target_name} "${CMAKE_CURRENT_LIST_DIR}/main.cc")
    target_compile_features(${target_name} PUBLIC cxx_std_20)
    target_include_directories(${target_name} PUBLIC "${PROJECT_SOURCE_DIR}/src")
endfunction()

add_main_executable(qfortress)
target_compile_definitions(qfortress PUBLIC QF_CLIENT)
target_link_libraries(qfortress PUBLIC core client client_modern)
