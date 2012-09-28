project (tests)
cmake_minimum_required (VERSION 2.8.3)

add_executable(unit_tests unit_tests.cpp)
add_executable(speed_test speed_test.cpp)

include_directories(../src ../build/src)

find_package(Boost 1.46 REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(unit_tests rhea ${Boost_LIBRARIES})
target_link_libraries(speed_test rhea)

