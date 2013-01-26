project(rhea)
cmake_minimum_required(VERSION 2.8.3)
set(CPACK_PACKAGE_NAME "rhea")

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

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
    set(CMAKE_LIB_LINKER_FLAGS  "${CMAKE_LIB_LINKER_FLAGS} /OPT:REF /SUBSYSTEM:WINDOWS")
    
else() # Most likely gcc or clang
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "-std=c++11")
    elseif ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7")
        set(CMAKE_CXX_FLAGS "-std=c++0x")
    else()
        set(CMAKE_CXX_FLAGS "-std=c++11")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pedantic -Wall -Wno-strict-aliasing")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_LIB_LINKER_FLAGS "-s")    
endif()

add_subdirectory(rhea)

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

elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

include(CPack)

