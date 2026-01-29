include(FetchContent)

if(WIN32)
    if(MSVC)
        FetchContent_Declare(download_sdl3 URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-devel-3.4.0-VC.zip DOWNLOAD_EXTRACT_TIMESTAMP)
        FetchContent_MakeAvailable(download_sdl3)
    elseif(MINGW)
        FetchContent_Declare(download_sdl3 URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-devel-3.4.0-mingw.zip DOWNLOAD_EXTRACT_TIMESTAMP)
        FetchContent_MakeAvailable(download_sdl3)
    else()
        message(FATAL_ERROR "SDL3 doesn't provide Windows binaries for your build environment!")
    endif()
    
    set(SDL3_DIR "${download_sdl3_SOURCE_DIR}/cmake")
endif()

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

## Interface the package into a target
## that can be used anywhere in the project
add_library(external_SDL3 INTERFACE)
target_link_libraries(external_SDL3 INTERFACE SDL3::SDL3)

## Add an alias library so dependency list looks nicer
add_library(external::SDL3 ALIAS external_SDL3)
