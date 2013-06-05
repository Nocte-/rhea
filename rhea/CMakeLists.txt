cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME rhea)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

set (LIBNAME_S "${LIBNAME}-s")
add_library(${LIBNAME_S} STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_library(${LIBNAME}   SHARED ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(${LIBNAME} PROPERTIES VERSION ${VERSION})

install(TARGETS ${LIBNAME_S} ${LIBNAME} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/rhea)

