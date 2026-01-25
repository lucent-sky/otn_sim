cmake_minimum_required(VERSION 3.16)
project(otn_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# lib
add_library(otn
    src/payload.cpp
    src/opu.cpp
    src/odu.cpp
    src/otu.cpp
    src/otn_types.cpp
    src/fragmentation.cpp
    src/groomed_child.cpp
    src/grooming_planner.cpp
    src/candidate.cpp
)

# main
add_executable(otn_sim
    src/main.cpp
)
target_link_libraries(otn_sim otn)

include(FetchContent)

# thanks gtest for breaking
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(otn_tests
    tests/test_otn_layers.cpp
    tests/test_admission.cpp
)

target_link_libraries(otn_tests
    otn
    gtest
    gtest_main
)

add_test(NAME otn_tests COMMAND otn_tests)
