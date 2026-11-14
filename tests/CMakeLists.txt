message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Configution Types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")

# Add Google Test
include(FetchContent)
FetchContent_Declare(
	googletest
    URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Disable building tests for tinyxml2
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "Do not build tests for tinyxml2")

# Add tinyxml2
FetchContent_Declare(
    tinyxml2
    URL https://github.com/leethomason/tinyxml2/archive/refs/tags/9.0.0.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(tinyxml2)

# Disable building tests for tinyxml2
set(PUGIXML_BUILD_TESTS OFF CACHE BOOL "Do not build tests for pugixml")

# Fetch pugixml
FetchContent_Declare(
    pugixml
    URL https://github.com/zeux/pugixml/releases/download/v1.15/pugixml-1.15.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(pugixml)

# Enable testing
enable_testing()

# Add the test executable
add_executable(musxdomtests
	musxdomtests.cpp
    dom/document.cpp
    dom/document_graphics.cpp
    dom/header.cpp
    dom/instrument.cpp
    dom/pool.cpp
    # entries
    entries/accidentals.cpp
    entries/beam_detection.cpp
    entries/cross_staffs.cpp
    entries/entries.cpp
    entries/layer_settings.cpp
    entries/slurs.cpp
    entries/keysigs.cpp
    entries/ties.cpp
    entries/tie_contours.cpp
    # options
    options/accidental_options.cpp
    options/alternate_notation_options.cpp
    options/augementation_dot_options.cpp
    options/barline_options.cpp
    options/beam_options.cpp
    options/chord_options.cpp
    options/clef_options.cpp
    options/flag_options.cpp
    options/grace_note_options.cpp
    options/key_signature_options.cpp
    options/line_curve_options.cpp
    options/lyric_options.cpp
    options/misc_options.cpp
    options/multimeasure_rest_options.cpp
    options/music_spacing_options.cpp
    options/music_symbol_options.cpp
    options/note_rest_options.cpp
    options/page_options.cpp
    options/piano_brace_bracket_options.cpp
    options/repeat_options.cpp
    options/smart_shape_options.cpp
    options/staff_options.cpp
    options/stem_options.cpp
    options/text_options.cpp
    options/tie_options.cpp
    options/time_signature_options.cpp
    options/tuplet_options.cpp
    # others
	others/articulation.cpp
	others/beat_chart.cpp
	others/enclosure.cpp
	others/expression.cpp
	others/fonts.cpp
    others/frame.cpp
    others/fret_instrument.cpp
    others/graphics.cpp
	others/layer_attributes.cpp
	others/instrument_used.cpp
    others/measure.cpp
    others/measure_number_region.cpp
    others/miscellaneous.cpp
    others/mmrest.cpp
    others/multistaff_inst.cpp
    others/ossia.cpp
    others/page.cpp
    others/part_definition.cpp
	others/part_globals.cpp
	others/part_voicing.cpp
	others/percussion_notes.cpp
    others/playback.cpp
    others/repeats.cpp
    others/shape.cpp
    others/smart_shape.cpp
    others/staff.cpp
    others/staff_style.cpp
    others/staff_system.cpp
    others/text_block.cpp
    others/text.cpp
    # details
	details/baselines.cpp
	details/beam_stubs.cpp
	details/chords.cpp
    details/entry_details.cpp
	details/floats.cpp
    details/fretboards_diagrams.cpp
	details/gfhold.cpp
    details/lyrics.cpp
	details/secondary_beams.cpp
    details/shape_note.cpp
	details/staff_group.cpp
    details/stems.cpp
    details/tuplet_def.cpp
    # music_theory
    music_theory/transpose.cpp
    # util
    util/arpeggio.cpp
    util/cue.cpp
    util/entry_geometry.cpp
    util/fraction.cpp
    util/fretboard.cpp
    util/svg_arrowheads.cpp
    util/svg_convert.cpp
    util/svg_ext_graphics.cpp
)

# Enable Unity/Jumbo build for this target
set_target_properties(musxdomtests PROPERTIES UNITY_BUILD ON)

set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "Suppress deprecation warnings for external projects" FORCE)
if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    # Extra warnings for Clang/AppleClang
    target_compile_options(musxdomtests PRIVATE -Wall -Wextra -Werror -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Extra warnings for MSVC
    target_compile_options(musxdomtests PRIVATE /bigobj /Zc:__cplusplus /W4 /WX)
endif()

add_dependencies(musxdomtests musx)
target_sources(musxdomtests PUBLIC "${MUSX_ROOT_DIR}/src/musx/musx.h")

# Add the include paths
target_include_directories(musxdomtests PRIVATE "${MUSX_ROOT_DIR}/tests")
target_include_directories(musxdomtests PRIVATE "${MUSX_ROOT_DIR}/third_party/rapidxml")

# Link Google Test and the library to the test executable
target_link_libraries(musxdomtests PRIVATE
    gtest_main
	tinyxml2
    pugixml
    musx
)

add_executable(factory_client_compile client_factory_compile.cpp)
target_link_libraries(factory_client_compile PRIVATE musx)
add_test(NAME FactoryClientCompileBoundary COMMAND factory_client_compile)

# Add Precompiled Headers (PCH) for the test suite
target_precompile_headers(musxdomtests PRIVATE
    "${gtest_SOURCE_DIR}/include/gtest/gtest.h"
    "${MUSX_ROOT_DIR}/src/musx/musx.h"
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(musxdomtests
    DISCOVERY_TIMEOUT 60 # Set timeout to 60 seconds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    PROPERTIES
        ENVIRONMENT "GTEST_COLOR=1"
    EXTRA_ARGS --gtest_color=yes
)

# Allow users to disable building benchmarks
option(musx_BUILD_BENCHMARKS "Build musx benchmark binary" ON)
if(musx_BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled for musx_BUILD_BENCHMARKS.")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Benchmarks not enabled for musx_BUILD_BENCHMARKS.")
endif()
