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
# Paths
# ============================================================
set(HAZE_SOURCE_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(HAZE_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(HAZE_TESTS_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/tests")

# ============================================================
# Dependencies
# ============================================================
include(FetchContent)
enable_testing()

FetchContent_Declare(
    libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_TAG        v1.51.0
)
FetchContent_MakeAvailable(libuv)

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
# Third party
# ============================================================
if(EXISTS "${HAZE_THIRD_PARTY_DIR}")
    haze_collect_sources(HAZE_THIRD_PARTY_SOURCES "${HAZE_THIRD_PARTY_DIR}")

    if(HAZE_THIRD_PARTY_SOURCES)
        add_library(haze_third_party STATIC ${HAZE_THIRD_PARTY_SOURCES})
        target_include_directories(haze_third_party PUBLIC "${HAZE_THIRD_PARTY_DIR}")
    endif()
endif()

# ============================================================
# Core sources
# ============================================================
haze_collect_sources(HAZE_CORE_SOURCES "${HAZE_SOURCE_DIR}")

if(NOT HAZE_CORE_SOURCES)
    message(FATAL_ERROR "No source files found in ${HAZE_SOURCE_DIR}")
endif()

# ============================================================
# Library do core (sem o main)
# ============================================================
# Filtra o arquivo que contém o main para não duplicar símbolo
set(HAZE_LIB_SOURCES ${HAZE_CORE_SOURCES})
list(FILTER HAZE_LIB_SOURCES EXCLUDE REGEX ".*main\\.c$")

add_library(haze_core STATIC ${HAZE_LIB_SOURCES})

target_include_directories(haze_core PUBLIC "${HAZE_SOURCE_DIR}")
target_link_libraries(haze_core PUBLIC uv)
target_compile_definitions(haze_core PRIVATE _XOPEN_SOURCE=700)

if(TARGET haze_third_party)
    target_link_libraries(haze_core PUBLIC haze_third_party)
endif()

# ============================================================
# Executável principal
# ============================================================
add_executable(haze ${HAZE_CORE_SOURCES})

target_include_directories(haze PUBLIC "${HAZE_SOURCE_DIR}")
target_link_libraries(haze PUBLIC uv)
target_compile_definitions(haze PRIVATE _XOPEN_SOURCE=700)

if(TARGET haze_third_party)
    target_link_libraries(haze PUBLIC haze_third_party)
endif()

set_target_properties(haze PROPERTIES WIN32_EXECUTABLE FALSE)

if(MSVC)
    target_link_options(haze PRIVATE /SUBSYSTEM:CONSOLE)
elseif(MINGW)
    target_link_options(haze PRIVATE -mconsole)
endif()

if(WIN32)
    add_custom_command(TARGET haze POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:uv>"
            "$<TARGET_FILE_DIR:haze>"
    )
endif()

# ============================================================
# Teste proto
# ============================================================
add_executable(proto "${HAZE_TESTS_DIR}/proto.c")

target_link_libraries(proto PRIVATE haze_core)
target_compile_definitions(proto PRIVATE _XOPEN_SOURCE=700)

# ============================================================
# Warnings
# ============================================================
set(HAZE_TARGETS haze_core haze proto)

if(MSVC)
    foreach(T ${HAZE_TARGETS})
        target_compile_options(${T} PRIVATE /W4)
    endforeach()
else()
    foreach(T ${HAZE_TARGETS})
        target_compile_options(${T} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()
