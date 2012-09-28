project(rhea)
cmake_minimum_required(VERSION 2.8.3)
set(CPACK_PACKAGE_NAME "rhea")
set(VERSION "0.1.0")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(BUILD_UNITTESTS 0 CACHE BOOL "Build the unit tests")
set(BUILD_DOCUMENTATION 0 CACHE BOOL "Generate Doxygen documentation")

# Set up the compiler
#
if(MSVC)
    add_definitions(/D_WIN32_WINNT=0x0501 /D_CRT_SECURE_NO_WARNINGS /DSFML_STATIC)
	set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /MP /EHsc /wd4244 /wd4996 ")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /MP /GS- /Ox /Ob2 /Oi /Oy /arch:SSE /fp:fast /Zi")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /SUBSYSTEM:WINDOWS")
    
else() # Most likely gcc or clang
    set(CMAKE_CXX_FLAGS "-std=c++0x -pedantic -Wall -Wno-strict-aliasing")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(LINK_FLAGS "-s")    
endif()

add_subdirectory(src)

if(BUILD_UNITTESTS)
  add_subdirectory(unit_tests)
endif()

# Doxygen documentation
#
if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if (DOXYGEN_FOUND STREQUAL "YES")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES api-doc)
      get_target_property(DOC_TARGET doc TYPE)
      if (NOT DOC_TARGET)
          add_custom_target(doc)
      endif()
      add_dependencies(doc doxygen)
  endif()
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An implementation of the Cassowary solver")
set(CPACK_PACKAGE_VENDOR "Nocte")
set(CPACK_PACKAGE_CONTACT "Nocte <nocte@hippie.nu>")
set(CPACK_SOURCE_IGNORE_FILES "CMakefiles;Makefile;CMakeCache.txt;_CPack_Packages;/.git/;.gitignore;")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${VERSION}")
    
if(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
    
    set(CPACK_DEBIAN_PACKAGE_SECTION "Development")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.0), libstdc++6 (>= 4.0)")
    add_custom_target(deb dpkg-buildpackage)
    add_dependencies(deb dist)
endif()

include(CPack)

