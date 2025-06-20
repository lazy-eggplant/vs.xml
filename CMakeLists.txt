cmake_minimum_required(VERSION 3.16)

# Generated via LLM and just manually corrected as I don't use CMake myself.
# It only exposes a subset of the meson build script, basically what is needed to generate the library itself an nothing more.

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(WARNING "
  ================================================================================
  WARNING: You are using the CMake build wrapper for vs-xml
           The primary build system for this project is Meson.
           This CMake script exists only for wider compatibility,
           is less fully featured, and is not regularly tested!
  ================================================================================
  ")
endif()

# Read version string from file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _version_lines)
list(GET _version_lines 0 PROJECT_VERSION)
string(REGEX MATCHALL "([0-9]+)" _ver_components "${PROJECT_VERSION}")
list(GET _ver_components 0 PROJECT_VERSION_MAJOR)
list(GET _ver_components 1 PROJECT_VERSION_MINOR)
list(GET _ver_components 2 PROJECT_VERSION_PATCH)

option(VS_XML_NOEXCEPT "Build library with noexcept everywhere" OFF)
option(VS_XML_NO_ASSERT "Disable asserts in vs-xml" OFF)
option(VS_XML_USE_FMT   "Find and use fmt library"     OFF)
option(VS_XML_USE_GTL   "Find and use gtl library"     OFF)
option(VS_XML_NS        "Custom namespace name"     "xml")
option(VS_XML_LAYOUT    "Memory layout"     0)

# dependencies
if(VS_XML_USE_FMT)
  find_package(fmt REQUIRED)
endif()
if(VS_XML_USE_GTL)
  find_package(gtl REQUIRED)
endif()

# configure the header
set(CONFIG_IN  "${CMAKE_CURRENT_SOURCE_DIR}/include/vs-xml/config.hpp.in")
set(CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/include/vs-xml/config.hpp")
configure_file(${CONFIG_IN} ${CONFIG_OUT} @ONLY)

#
# Project declaration
#
project(vs-xml
  VERSION ${PROJECT_VERSION}
  LANGUAGES CXX
)

# Require at least CMake 3.16 for good C++23 support
if (CMAKE_VERSION VERSION_LESS "3.16")
  message(FATAL_ERROR "CMake >= 3.16 is required")
endif()

#
# Compiler settings
#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global warning suppression 
add_compile_options(-Wno-c23-extensions)

#
# Include directories
#
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#
# Library target
#
set(VS_XML_SOURCES
  lib/archive.cpp
  lib/parser.cpp
  lib/serializer.cpp
  lib/tree.cpp
  lib/builder.cpp
  lib/inplace-builder.cpp
  lib/query.cpp
  lib/query-builder.cpp
)

add_library(vs-xml
  ${VS_XML_SOURCES}
)

target_include_directories(vs-xml
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>    # other headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>    # generated config.hpp
    $<INSTALL_INTERFACE:include>
)

if(VS_XML_USE_FMT)
  target_link_libraries(vs-xml PUBLIC fmt::fmt)
endif()
if(VS_XML_USE_GTL)
  target_link_libraries(vs-xml PUBLIC gtl::gtl)
endif()

# Install the library and headers
install(TARGETS vs-xml
        EXPORT vs-xml-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vs-xml   # your shipped headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/vs-xml   # contains config.hpp
  DESTINATION include/
  FILES_MATCHING PATTERN "*"
  PATTERN "config.hpp.in" EXCLUDE              # never install the .in template
  PATTERN "meson.build" EXCLUDE
)

#
# Export targets for downstream use
#
include(CMakePackageConfigHelpers)

# Create a versioned config file for find_package(vs-xml)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/vs-xmlConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Where we want to generate the installed config file
set(_config_dest "${CMAKE_CURRENT_BINARY_DIR}/vs-xmlConfig.cmake")

# Prepare the content that would have lived in vs-xmlConfig.cmake.in
# Use @PACKAGE_INIT@ to bring in CMakePackageConfigHelpers glue;
# include your exported targets file; fix up include dirs as needed.
set(_config_content "  
@PACKAGE_INIT@

# Include the targets exported by install(EXPORT vs-xml-targets)
include(\"\${CMAKE_CURRENT_LIST_DIR}/vs-xmlTargets.cmake\")

# Set the include directory variable for consumers
# so they can do target_include_directories(vs::vs-xml ...).
set(vs-xml_INCLUDE_DIRS \"\${CMAKE_CURRENT_LIST_DIR}/../../include\")
")

# Generate the config file in the build tree
file(GENERATE
  OUTPUT "${_config_dest}"
  CONTENT "${_config_content}"
)

# Now install it alongside the other cmake artifacts
install(FILES
  "${_config_dest}"
  "${CMAKE_CURRENT_BINARY_DIR}/vs-xmlConfigVersion.cmake"
  DESTINATION lib/cmake/vs-xml
)

install(EXPORT vs-xml-targets
        FILE vs-xmlTargets.cmake
        NAMESPACE vs:: 
        DESTINATION lib/cmake/vs-xml
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vs-xmlConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vs-xmlConfigVersion.cmake"
  DESTINATION lib/cmake/vs-xml
)
