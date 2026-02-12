cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(QFortress LANGUAGES C CXX VERSION 0.0.1)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

option(BUILD_CLIENT "Build client" ON)
option(BUILD_SERVER "Build server" ON)

if(NOT BUILD_CLIENT AND NOT BUILD_SERVER)
    message(FATAL_ERROR "Neither client or server is enabled; nothing to build")
endif()

## Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

if(MSVC)
    add_compile_options(/MP)
    add_compile_options(/W3)
endif()

execute_process(COMMAND git rev-parse --short=8 HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git rev-parse --abbrev-ref HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

## Third-party dependencies
add_subdirectory(external)

## Core engine library
add_subdirectory(core)

## Add game code subdirectories
add_subdirectory(game/client)
add_subdirectory(game/server)
add_subdirectory(game/shared)

## Add rendering implementations
add_subdirectory(render/compat)
add_subdirectory(render/modern)

## Game launcher
add_subdirectory(main)

## Game development tools
add_subdirectory(tools/qfgeomp)
add_subdirectory(tools/qflight)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "doc/qfortress")

set(CPACK_PACKAGE_NAME "QFortress")
set(CPACK_PACKAGE_VENDOR "untodesu")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}-${GIT_BRANCH}-${GIT_COMMIT}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/packages")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-installer.ico")
set(CPACK_NSIS_MUI_HEADERIMAGE "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-header.bmp")
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_SOURCE_DIR}\\\\media\\\\nsis-splash.bmp")

unset(CPACK_NSIS_MENU_LINKS)
list(APPEND CPACK_NSIS_MENU_LINKS "bin/qfortress.exe" "QFortress")
list(APPEND CPACK_NSIS_MENU_LINKS "bin/qfserver.exe" "QFortress server")

include(CPack)
