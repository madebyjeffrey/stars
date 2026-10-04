cmake_minimum_required(VERSION 3.23)
# The version this line is building toward. Tags (vX.Y.Z) and build numbers
# come from git; see cmake/version.cmake and docs/VERSIONING.md.
project(stars VERSION 2.9.0 LANGUAGES C RC)

if(NOT MINGW)
    message(FATAL_ERROR "Configure using the MinGW toolchain: cmake --preset mingw-debug or cmake --preset mingw-release")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The game: rules, turn generation, AI and file I/O. A source file belongs
# here only if the command-line host needs it; Windows dialogs and drawing go
# in STARS_UI_SOURCES. Some of these files still hold UI functions; the 2.9
# split moves those out (docs/ROADMAP.md).
set(STARS_CORE_SOURCES
    ai.c ai2.c ai3.c ai4.c aiutil.c
    battle.c build.c create.c file.c globals.c log.c memory.c mine.c msg.c
    native.c parts.c planet.c produce.c race.c research.c save.c ship.c
    ship2.c strings.c strings_uncompressed.c thing.c turn.c turn2.c turn3.c
    util.c utilgen.c
)

# The Windows game's user interface.
set(STARS_UI_SOURCES
    battleui.c buildui.c createui.c fileui.c globalsui.c init.c logui.c mdi.c
    mineui.c msgui.c nativeui.c planetui.c popup.c produceui.c raceui.c
    report.c researchui.c scan.c ship2ui.c shipui.c stars.c tb.c thingui.c
    tutor.c tutor2.c utilgenui.c utilui.c vcr.c
)

# Settings every target that compiles game code shares.
add_library(stars_options INTERFACE)

# Each set is built once for stars.exe and the unit tests.
add_library(stars_core OBJECT ${STARS_CORE_SOURCES})
target_link_libraries(stars_core PUBLIC stars_options)
add_library(stars_ui OBJECT ${STARS_UI_SOURCES})
target_link_libraries(stars_ui PUBLIC stars_options)

add_executable(stars WIN32 res/stars.rc)
target_link_libraries(stars PRIVATE stars_core stars_ui)

# version.h is regenerated on every build so the build number follows HEAD;
# it is rewritten only when the version changes.
set(STARS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(STARS_VERSION_COMMAND "${CMAKE_COMMAND}"
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DOUTPUT=${STARS_GENERATED_DIR}/version.h
    -DVERSION_BASE=${PROJECT_VERSION}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake")
file(MAKE_DIRECTORY "${STARS_GENERATED_DIR}")
execute_process(COMMAND ${STARS_VERSION_COMMAND})
add_custom_target(stars_version
    COMMAND ${STARS_VERSION_COMMAND}
    BYPRODUCTS "${STARS_GENERATED_DIR}/version.h"
    COMMENT "Checking Stars! version"
    VERBATIM
)
add_dependencies(stars_core stars_version)
add_dependencies(stars_ui stars_version)

set_target_properties(stars PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(stars_options INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "${STARS_GENERATED_DIR}"
)

# Match the existing MinGW compile-analysis flags. Plain char is signed, as
# in the original MSC build; the decompiler relies on it to drop char casts.
target_compile_options(stars_options INTERFACE
    $<$<COMPILE_LANGUAGE:C>:-Wno-pointer-sign>
    $<$<COMPILE_LANGUAGE:C>:-fsigned-char>
)
target_link_libraries(stars_options INTERFACE user32 gdi32 comdlg32)

# MinGW runtime objects can contain debug data even in a Release build.
target_link_options(stars PRIVATE
    $<$<CONFIG:Release>:LINKER:--strip-debug>
)

# The tutorial runner reads this observer to drive real UI controls.
option(STARS_TEST_TUTORIAL "Expose read-only tutorial state for AutoHotkey tests" OFF)
if(STARS_TEST_TUTORIAL)
    target_sources(stars PRIVATE tests/scaffold/tutorial/observer.c)
    target_link_options(stars PRIVATE "LINKER:--wrap=InitInstance,--wrap=ScannerWndProc")
endif()

# Trace builds wrap cross-file calls and log when STARS_TRACE names a file.
option(STARS_TEST_TRACE "Log Random and other selected calls to $STARS_TRACE for regression tests" OFF)
if(STARS_TEST_TRACE)
    target_sources(stars PRIVATE tests/scaffold/regression_trace.c)
    target_link_options(stars PRIVATE "LINKER:--wrap=Random,--wrap=PctPlanetCapacity")
endif()

# Recompile resources when an embedded bitmap, icon, cursor, or data file changes.
file(GLOB_RECURSE STARS_RESOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/res/*"
)
list(REMOVE_ITEM STARS_RESOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/res/stars.rc"
)
set_source_files_properties(res/stars.rc PROPERTIES
    OBJECT_DEPENDS "${STARS_RESOURCE_FILES};${STARS_GENERATED_DIR}/version.h"
)

# Wine is needed only to run the Windows executable on the build host.
find_program(WINE_EXECUTABLE NAMES wine)
if(WINE_EXECUTABLE)
    set_property(TARGET stars PROPERTY CROSSCOMPILING_EMULATOR "${WINE_EXECUTABLE}")
    add_custom_target(run_in_wine
        COMMAND "${WINE_EXECUTABLE}" "$<TARGET_FILE:stars>"
        DEPENDS stars
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        USES_TERMINAL
        VERBATIM
    )
else()
    message(STATUS "Wine not found; stars.exe can still be built, but run_in_wine is unavailable")
endif()

# Unit tests (tests/unit): one acutest executable per test_*.c, linked with
# the game code and its resources. They are Windows console programs; CTest
# runs them through Wine when it is available.
option(STARS_BUILD_TESTS "Build the unit tests in tests/unit" ON)
if(STARS_BUILD_TESTS)
    enable_testing()
    set(STARS_TEST_DIR "${CMAKE_BINARY_DIR}/tests")
    file(MAKE_DIRECTORY "${STARS_TEST_DIR}/data")
    configure_file(tests/scaffold/fixtures/newgame/tiny/humanoid.r1 "${STARS_TEST_DIR}/data/humanoid.r1" COPYONLY)
    file(GLOB STARS_UNIT_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/test_*.c")
    foreach(STARS_UNIT_TEST ${STARS_UNIT_TESTS})
        get_filename_component(STARS_TEST_NAME "${STARS_UNIT_TEST}" NAME_WE)
        add_executable(${STARS_TEST_NAME} "${STARS_UNIT_TEST}" tests/unit/stars_test.c res/stars.rc)
        target_link_libraries(${STARS_TEST_NAME} PRIVATE stars_core stars_ui)
        # Message boxes would wait for a click; stars_test.c records them.
        target_link_options(${STARS_TEST_NAME} PRIVATE "LINKER:--wrap=AlertSz")
        target_include_directories(${STARS_TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit")
        set_target_properties(${STARS_TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${STARS_TEST_DIR}")
        if(WINE_EXECUTABLE)
            set_property(TARGET ${STARS_TEST_NAME} PROPERTY CROSSCOMPILING_EMULATOR "${WINE_EXECUTABLE}")
        endif()
        add_test(NAME ${STARS_TEST_NAME} COMMAND ${STARS_TEST_NAME})
        set_tests_properties(${STARS_TEST_NAME} PROPERTIES
            WORKING_DIRECTORY "${STARS_TEST_DIR}"
            ENVIRONMENT "WINEDEBUG=-all")
    endforeach()
    # Games to open by hand (tests/scenarios, make scenario).
    add_executable(stars_scenario tests/scenarios/scenarios.c tests/unit/stars_test.c res/stars.rc)
    target_link_libraries(stars_scenario PRIVATE stars_core stars_ui)
    target_link_options(stars_scenario PRIVATE "LINKER:--wrap=AlertSz")
    target_include_directories(stars_scenario PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit")
    set_target_properties(stars_scenario PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${STARS_TEST_DIR}")
    # The native-port test replaces file I/O with in-memory records.
    target_link_options(test_native_ports PRIVATE
        "LINKER:--wrap=WriteRt,--wrap=ReadRt,--wrap=FCreateFile,--wrap=StreamOpen,--wrap=StreamClose,--wrap=DirtyGame")
endif()
