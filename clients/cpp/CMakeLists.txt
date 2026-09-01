cmake_minimum_required(VERSION 3.20)

project(Client
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Paths
# ============================================================

set(HAZE_THIRD_PARTY_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party"
)

# ============================================================
# Third party
# ============================================================

file(GLOB_RECURSE MPACK_SOURCES
    CONFIGURE_DEPENDS
    "${HAZE_THIRD_PARTY_DIR}/*.c"
    "${HAZE_THIRD_PARTY_DIR}/*.cpp"
)

add_library(client_third_party STATIC
    ${MPACK_SOURCES}
)

target_include_directories(client_third_party
    PUBLIC
        "${HAZE_THIRD_PARTY_DIR}"
)

# ============================================================
# Client sources
# ============================================================

file(GLOB_RECURSE CLIENT_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_library(Client STATIC
    ${CLIENT_SOURCES}
)

target_include_directories(Client
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(Client
    PRIVATE
        client_third_party
)
