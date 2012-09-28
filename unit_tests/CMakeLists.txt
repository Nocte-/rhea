project (unit-tests)
cmake_minimum_required (VERSION 2.8.3)
set(EXE unittests)

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_executable(${EXE} ${SOURCE_FILES} ${HEADER_FILES})

link_directories()
include_directories(../src ../build/src)

find_package(Boost 1.46 REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${EXE} rhea ${Boost_LIBRARIES})

