#
# Copyright (c) 2026 Luiz Gustavo P Feldmann
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

# Read project version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON)
string(JSON CMAKE_PROJECT_NAME GET "${PACKAGE_JSON}" "name")
string(JSON CMAKE_PROJECT_VERSION GET "${PACKAGE_JSON}" "version")
string(JSON CMAKE_PROJECT_DESCRIPTION GET "${PACKAGE_JSON}" "description")
string(JSON CMAKE_PROJECT_HOMEPAGE_URL GET "${PACKAGE_JSON}" "repository" "url")
string(JSON CMAKE_PROJECT_SPDX_LICENSE GET "${PACKAGE_JSON}" "license")

PROJECT(${CMAKE_PROJECT_NAME}
    LANGUAGES CXX
    VERSION ${CMAKE_PROJECT_VERSION}
    DESCRIPTION "${CMAKE_PROJECT_DESCRIPTION}"
    HOMEPAGE_URL "${CMAKE_PROJECT_HOMEPAGE_URL}")

# Default directories
include(GNUInstallDirs)

# Configure version information
SET(VERSION_FILE_SRC "${CMAKE_BINARY_DIR}/version_${CMAKE_PROJECT_NAME}_.cpp")
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")
configure_file("src/version.in.c" ${VERSION_FILE_SRC})

# Find dependencies
find_package(cxxopts)
find_package(PcapPlusPlus)
find_package(Boost CONFIG REQUIRED)

# Add helper library
add_library(${CMAKE_PROJECT_NAME}-lib STATIC
    # Generated files
    ${VERSION_FILE_SRC}
    # Sources
    "src/EAppError.cpp"
    "src/CAppErrorCategory.cpp"
    "src/CCommandLineArguments.cpp"
    "src/CReplay.cpp"
    # Headers
    "include/ELimitMode.h"
    "include/ERepeatMode.h"
    "include/ESpeedMode.h"
    "include/IJobArguments.h"
    "include/ICommandLineArguments.h"
    "include/CCommandLineArguments.h"
    "include/EAppError.h"
    "include/CAppErrorCategory.h"
    "include/CReplay.h"
    "include/version.h" )

target_include_directories(${CMAKE_PROJECT_NAME}-lib PUBLIC 
    "include" )

target_link_libraries(${CMAKE_PROJECT_NAME}-lib PUBLIC
    Boost::boost
    PcapPlusPlus::PcapPlusPlus
    cxxopts::cxxopts )

# Add executable
add_executable(${CMAKE_PROJECT_NAME}
    "src/main.cpp" )

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_PROJECT_NAME}-lib )

# Only build tests if enabled
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(unit-tests 
        "test/cli.test.cpp" )
    target_link_libraries(unit-tests PRIVATE 
        ${CMAKE_PROJECT_NAME}-lib 
        GTest::gtest_main )

    include(GoogleTest)
    gtest_discover_tests(unit-tests)
endif()

# Install target
install(TARGETS ${CMAKE_PROJECT_NAME})
