# Toolchain check: a C99 function called from a C++20 test through a plain C
# ABI, the same shape the lab_api.h boundary will use.
add_executable(toolchain_test
    toolchain/toolchain_c.c
    toolchain/toolchain_test.cpp)
target_link_libraries(toolchain_test PRIVATE ac_warnings)
add_test(NAME toolchain COMMAND toolchain_test)

# Safety checks. These need bash and git.
find_program(BASH_EXECUTABLE bash REQUIRED)

add_test(NAME forbidden_api_guard
    COMMAND ${BASH_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check-forbidden-apis.sh ${PROJECT_SOURCE_DIR})

add_test(NAME forbidden_api_selftest
    COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/safety/forbidden_apis_selftest.sh ${PROJECT_SOURCE_DIR})
