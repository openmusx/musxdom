cmake_minimum_required(VERSION 3.16)

project(musx)

# Set the C++ standard
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()

# Enforce at least C++17, allow higher if set by the parent
if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Musx requires at least C++17. Current: C++${CMAKE_CXX_STANDARD}")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Musx C++ standard: ${CMAKE_CXX_STANDARD}")

set(MUSX_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Root directory of musx")

# Allow users to disable building tests
option(musx_BUILD_TESTING "Enable testing for your library" ON)
if(musx_BUILD_TESTING)
    message(STATUS "Testing enabled for musx_BUILD_TESTING.")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Testing not enabled for musx_BUILD_TESTING.")
endif()

# Add the include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(MUSX_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/CommonClasses.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Details.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Document.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Entries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Graphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Instrument.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/MusxInstance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Options.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Others.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Ossia.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/PercussionNoteType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/ShapeDesigner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/SmartShape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Staff.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/dom/Texts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/FieldPopulatorsCommon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/FieldPopulatorsDetails.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/FieldPopulatorsEntries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/FieldPopulatorsOptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/FieldPopulatorsOthers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/DocumentFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/HeaderFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/factory/PoolFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Arpeggio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Cue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/EnigmaString.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Fretboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/PseudoTieUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/ShapeRecognize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/SmartShapeRecognize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/SvgConvert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/TestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/EntryGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Stem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Slur.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/musx/util/Tie.cpp
)

# Create a static library for the project
add_library(musx STATIC ${MUSX_SRC_FILES})

if (MSVC)
    target_compile_options(musx PRIVATE /bigobj /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    target_compile_options(musx PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Detect if we are building standalone or as part of a consuming project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(MUSX_STANDALONE_BUILD TRUE)
else()
    set(MUSX_STANDALONE_BUILD FALSE)
endif()

# Define options for compile-time macros
option(MUSX_THROW_ON_UNKNOWN_XML "Enable throwing on unknown XML" OFF)
option(MUSX_THROW_ON_INTEGRITY_CHECK_FAIL "Enable throwing integrity check failures" OFF)
option(MUSX_USE_TINYXML2 "Enable tinyxml2 parsing classes" OFF)
option(MUSX_USE_RAPIDXML "Enable rapidxml parsing classes" OFF)
option(MUSX_USE_PUGIXML "Enable pugixml parsing classes" OFF)
option(MUSX_USE_QTXML "Enable Qt xml parsing classes" OFF)
option(MUSX_DISPLAY_NODE_NAMES "Write node names to std::cout as they are processed" OFF)

# Override defaults for stand-alone builds
if(MUSX_STANDALONE_BUILD)
    set(MUSX_THROW_ON_UNKNOWN_XML ON CACHE BOOL "Enable throwing on unknown XML" FORCE)
    set(MUSX_THROW_ON_INTEGRITY_CHECK_FAIL ON CACHE BOOL "Enable throwing integrity check failures" FORCE)
    set(MUSX_USE_TINYXML2 ON CACHE BOOL "Enable tinyxml2 parsing classes" FORCE)
    set(MUSX_USE_RAPIDXML ON CACHE BOOL "Enable rapidxml parsing classes" FORCE)
    set(MUSX_USE_PUGIXML ON CACHE BOOL "Enable pugixml parsing classes" FORCE)
    set(MUSX_DISPLAY_NODE_NAMES OFF CACHE BOOL "Write node names to std::cout as they are processed" FORCE)
endif()

target_compile_definitions(musx PUBLIC
    $<$<BOOL:${MUSX_THROW_ON_UNKNOWN_XML}>:MUSX_THROW_ON_UNKNOWN_XML>
    $<$<BOOL:${MUSX_DISPLAY_NODE_NAMES}>:MUSX_DISPLAY_NODE_NAMES>
    $<$<BOOL:${MUSX_THROW_ON_INTEGRITY_CHECK_FAIL}>:MUSX_THROW_ON_INTEGRITY_CHECK_FAIL>
)

target_compile_definitions(musx INTERFACE
    $<$<BOOL:${MUSX_USE_TINYXML2}>:MUSX_USE_TINYXML2>
    $<$<BOOL:${MUSX_USE_RAPIDXML}>:MUSX_USE_RAPIDXML>
    $<$<BOOL:${MUSX_USE_PUGIXML}>:MUSX_USE_PUGIXML>
    $<$<BOOL:${MUSX_USE_QTXML}>:MUSX_USE_QTXML>
)

# Ensure the library uses the correct include paths
target_include_directories(musx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_precompile_headers(musx PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/musx/musx.h"
)

if(APPLE)
    target_link_libraries(musx PRIVATE "-framework CoreFoundation")
endif()
