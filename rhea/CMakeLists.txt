
cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME rhea)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_library(${LIBNAME} ${SOURCE_FILES} ${HEADER_FILES})

install(TARGETS rhea DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/rhea)

