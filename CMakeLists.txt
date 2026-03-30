#
# Copyright (c) 2026 Luiz Gustavo P Feldmann
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

# Read project version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON)
string(JSON CMAKE_PROJECT_NAME GET "${PACKAGE_JSON}" "name")
string(JSON CMAKE_PROJECT_VERSION GET "${PACKAGE_JSON}" "version")
string(JSON CMAKE_PROJECT_DESCRIPTION GET "${PACKAGE_JSON}" "description")

PROJECT(${CMAKE_PROJECT_NAME}
    LANGUAGES CXX
    VERSION ${CMAKE_PROJECT_VERSION}
    DESCRIPTION "${CMAKE_PROJECT_DESCRIPTION}")

# Link to find modules
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/generators")

# Default directories
include(GNUInstallDirs)

# Configure version information
SET(VERSION_FILE_SRC "${CMAKE_BINARY_DIR}/version_${CMAKE_PROJECT_NAME}_.cpp")
configure_file("src/version.in.c" ${VERSION_FILE_SRC})

# Find dependencies
find_package(cxxopts)
find_package(PcapPlusPlus)

# Add executable
add_executable(${CMAKE_PROJECT_NAME}
    # Generated files
    ${VERSION_FILE_SRC}
    # App Sources
    "src/parsecli.cpp"
    "src/main.cpp"
    # App Headers
    "include/ELimitMode.h"
    "include/ERepeatMode.h"
    "include/ESpeedMode.h"
    "include/IJobArguments.h"
    "include/ICommandLineArguments.h"
    "include/parsecli.h"
    "include/version.h" )

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    "include" )

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    PcapPlusPlus::PcapPlusPlus
    cxxopts::cxxopts )
