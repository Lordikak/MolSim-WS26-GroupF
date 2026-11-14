message(STATUS "Loading tests.cmake")
#include google test framework
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(
        Tests
        "${PROJECT_SOURCE_DIR}/tests/LennardJonesTest.cpp"
        "${PROJECT_SOURCE_DIR}/tests/SimulationTest.cpp"
)

target_link_libraries(
        Tests
        PRIVATE
        Lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Tests)
