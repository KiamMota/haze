cmake_minimum_required(VERSION 3.20)

project(haze
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Paths
# ============================================================

set(HAZE_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

set(HAZE_MODULES_DIR
    "${HAZE_SOURCE_DIR}/modules"
)

set(HAZE_CORE_DIR
    "${HAZE_MODULES_DIR}/core"
)

set(HAZE_EDITOR_DIR
    "${HAZE_MODULES_DIR}/editor"
)

set(HAZE_UI_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

set(HAZE_THIRD_PARTY_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)

# ============================================================
# Dependencies
# ============================================================

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB
    REQUIRED
    IMPORTED_TARGET
    glib-2.0
)

# ============================================================
# Helper
# ============================================================

function(haze_collect_sources OUTPUT DIRECTORY)

    file(GLOB_RECURSE SOURCES
        CONFIGURE_DEPENDS
        "${DIRECTORY}/*.c"
    )

    set(${OUTPUT} ${SOURCES} PARENT_SCOPE)

endfunction()

# ============================================================
# THIRD PARTY
# ============================================================

if(EXISTS "${HAZE_THIRD_PARTY_DIR}")

    haze_collect_sources(
        HAZE_THIRD_PARTY_SOURCES
        "${HAZE_THIRD_PARTY_DIR}"
    )

    if(HAZE_THIRD_PARTY_SOURCES)

        add_library(haze_third_party STATIC
            ${HAZE_THIRD_PARTY_SOURCES}
        )

        target_include_directories(haze_third_party
            PUBLIC
                "${HAZE_THIRD_PARTY_DIR}"
        )

    endif()

endif()

# ============================================================
# CORE
# ============================================================

haze_collect_sources(
    HAZE_CORE_SOURCES
    "${HAZE_CORE_DIR}"
)

if(NOT HAZE_CORE_SOURCES)
    message(FATAL_ERROR
        "No source files found in ${HAZE_CORE_DIR}"
    )
endif()

add_library(haze_core STATIC
    ${HAZE_CORE_SOURCES}
)

target_include_directories(haze_core
    PUBLIC
        "${HAZE_CORE_DIR}"
        "${HAZE_SOURCE_DIR}"
)

target_link_libraries(haze_core
    PUBLIC
        PkgConfig::GLIB
)

if(TARGET haze_third_party)
    target_link_libraries(haze_core
        PUBLIC
            haze_third_party
    )
endif()

# ============================================================
# EDITOR
# ============================================================

haze_collect_sources(
    HAZE_EDITOR_SOURCES
    "${HAZE_EDITOR_DIR}"
)

if(NOT HAZE_EDITOR_SOURCES)
    message(FATAL_ERROR
        "No source files found in ${HAZE_EDITOR_DIR}"
    )
endif()

add_library(haze_editor STATIC
    ${HAZE_EDITOR_SOURCES}
)

target_include_directories(haze_editor
    PUBLIC
        "${HAZE_EDITOR_DIR}"
        "${HAZE_CORE_DIR}"
        "${HAZE_SOURCE_DIR}"
)

target_link_libraries(haze_editor
    PUBLIC
        haze_core
)

# ============================================================
# UI
# ============================================================

set(HAZE_MAIN
    "${HAZE_UI_DIR}/main.cpp"
)

if(NOT EXISTS "${HAZE_MAIN}")
    message(FATAL_ERROR
        "UI entry point not found: ${HAZE_MAIN}"
    )
endif()

add_executable(haze
    "${HAZE_MAIN}"
)

target_include_directories(haze
    PRIVATE
        "${HAZE_UI_DIR}"
        "${HAZE_SOURCE_DIR}"
        "${HAZE_CORE_DIR}"
        "${HAZE_EDITOR_DIR}"
)

target_link_libraries(haze
    PRIVATE
        haze_editor
)

# ============================================================
# Platform
# ============================================================

if(WIN32)

    target_link_libraries(haze_core
        PUBLIC
            ws2_32
    )

elseif(UNIX AND NOT APPLE)

    target_link_libraries(haze_core
        PUBLIC
            m
            dl
            pthread
    )

elseif(APPLE)

    target_link_libraries(haze_core
        PUBLIC
            dl
            pthread
    )

endif()

# ============================================================
# Warnings
# ============================================================

if(MSVC)

    target_compile_options(haze_core PRIVATE /W4)
    target_compile_options(haze_editor PRIVATE /W4)
    target_compile_options(haze PRIVATE /W4)

else()

    target_compile_options(haze_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_compile_options(haze_editor PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_compile_options(haze PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()

message(STATUS "")
message(STATUS "========== Haze ==========")
message(STATUS "Platform : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler : ${CMAKE_C_COMPILER_ID}")
message(STATUS "GLib     : ${GLIB_VERSION}")
message(STATUS "Core     : ${HAZE_CORE_DIR}")
message(STATUS "Editor   : ${HAZE_EDITOR_DIR}")
message(STATUS "UI       : ${HAZE_MAIN}")
message(STATUS "==========================")
message(STATUS "")
