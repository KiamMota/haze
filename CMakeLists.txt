cmake_minimum_required(VERSION 3.20)
project(haze
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Caminhos
# ============================================================

set(HAZE_SOURCE_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(HAZE_MODULES_DIR     "${HAZE_SOURCE_DIR}/modules")
set(HAZE_CORE_DIR        "${HAZE_MODULES_DIR}/core")
set(HAZE_EDITOR_DIR      "${HAZE_MODULES_DIR}/editor")
set(HAZE_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# ============================================================
# Dependências
# ============================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)

# ============================================================
# Helper
# ============================================================

function(haze_collect_sources OUTPUT DIRECTORY)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${DIRECTORY}/*.c"
        "${DIRECTORY}/*.cpp"
    )
    set(${OUTPUT} ${SOURCES} PARENT_SCOPE)
endfunction()

# ============================================================
# Third party (opcional)
# ============================================================

if(EXISTS "${HAZE_THIRD_PARTY_DIR}")
    haze_collect_sources(HAZE_THIRD_PARTY_SOURCES "${HAZE_THIRD_PARTY_DIR}")
    if(HAZE_THIRD_PARTY_SOURCES)
        add_library(haze_third_party STATIC ${HAZE_THIRD_PARTY_SOURCES})
        target_include_directories(haze_third_party PUBLIC "${HAZE_THIRD_PARTY_DIR}")
    endif()
endif()

# ============================================================
# Core
# ============================================================

haze_collect_sources(HAZE_CORE_SOURCES "${HAZE_CORE_DIR}")
if(NOT HAZE_CORE_SOURCES)
    message(FATAL_ERROR "Nenhum fonte encontrado em ${HAZE_CORE_DIR}")
endif()

add_library(haze_core STATIC ${HAZE_CORE_SOURCES})
target_include_directories(haze_core PUBLIC
    "${HAZE_CORE_DIR}"
    "${HAZE_SOURCE_DIR}"
    ${LIBUV_INCLUDE_DIRS}
)

target_link_libraries(haze_core PUBLIC
    ${LIBUV_LIBRARIES}
)
target_compile_definitions(haze_core PRIVATE _XOPEN_SOURCE=700)

if(TARGET haze_third_party)
    target_link_libraries(haze_core PUBLIC haze_third_party)
endif()

# ============================================================
# Editor
# ============================================================

haze_collect_sources(HAZE_EDITOR_SOURCES "${HAZE_EDITOR_DIR}")
if(NOT HAZE_EDITOR_SOURCES)
    message(FATAL_ERROR "Nenhum fonte encontrado em ${HAZE_EDITOR_DIR}")
endif()

add_library(haze_editor STATIC ${HAZE_EDITOR_SOURCES})

target_include_directories(haze_editor PUBLIC
    "${HAZE_EDITOR_DIR}"
    "${HAZE_SOURCE_DIR}"
)

target_link_libraries(haze_editor PUBLIC haze_core)

# ============================================================
# UI (executável)
# ============================================================

set(HAZE_MAIN "${HAZE_SOURCE_DIR}/main.cpp")
if(NOT EXISTS "${HAZE_MAIN}")
    message(FATAL_ERROR "Entry point não encontrado: ${HAZE_MAIN}")
endif()

add_executable(haze "${HAZE_MAIN}")

target_include_directories(haze PRIVATE
    "${HAZE_SOURCE_DIR}"
)

target_link_libraries(haze PRIVATE haze_editor)

# ============================================================
# Warnings
# ============================================================

if(MSVC)
    foreach(T haze_core haze_editor haze)
        target_compile_options(${T} PRIVATE /W4)
    endforeach()
else()
    foreach(T haze_core haze_editor haze)
        target_compile_options(${T} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()
