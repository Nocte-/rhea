
cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME rhea)

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_library(${LIBNAME} ${SOURCE_FILES} ${HEADER_FILES})

if(UNIX)
    install(TARGETS rhea DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/")
    install(FILES ${HEADER_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rhea/")
endif()

