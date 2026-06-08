include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4)
FetchContent_MakeAvailable(Catch2)

add_executable(unit_tests test_board.cpp)
target_link_libraries(unit_tests PRIVATE game_core Catch2::Catch2WithMain)

# Register the suite with CTest. Catch2 ships a helper that creates one CTest
# entry per TEST_CASE for finer-grained reporting.
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(unit_tests)
