cmake_minimum_required(VERSION 3.20)
project(haze
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# klib
include(FetchContent)
FetchContent_Declare(
    klib
    GIT_REPOSITORY https://github.com/KiamMota/klib.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(klib)

# Sources
file(GLOB_RECURSE HAZE_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/*.c
)

add_executable(haze
    main.c
    ${HAZE_SOURCES}
)

# Includes
target_include_directories(haze PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party 
)

# Links
target_link_libraries(haze PRIVATE
    klib
    m
    dl
    pthread
)

# Warnings
target_compile_options(haze PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
