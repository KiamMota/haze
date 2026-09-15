file(GLOB UNIT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

foreach(TEST_FILE ${UNIT_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE hazecore)

    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /W4)
    else()
        target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
