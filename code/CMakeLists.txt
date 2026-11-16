# Limit the generated solution to supported configurations.
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

#
# ---------------------------------------------------------
# Directories that build as their own project
# ---------------------------------------------------------
#
set(SUBPROJECT_DIRS
    "language"
    "vqalib"
)

#
# ---------------------------------------------------------
# Gather all potential OpenTS files
# ---------------------------------------------------------
#
file(GLOB_RECURSE OPENTS_RAW CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.rc"
)

if(NOT WIN32)
    list(FILTER OPENTS_RAW EXCLUDE REGEX "\\.rc$")
endif()
if(NOT APPLE)
    list(FILTER OPENTS_RAW EXCLUDE REGEX "/platform/macos/")
endif()

#
# ---------------------------------------------------------
# Remove files that belong to a subproject
# ---------------------------------------------------------
#
set(OPENTS_SRC "")
foreach(f ${OPENTS_RAW})
    set(EXCLUDED FALSE)
    foreach(dir ${SUBPROJECT_DIRS})
        if(f MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}/${dir}/.*")
            set(EXCLUDED TRUE)
        endif()
    endforeach()

    if(NOT EXCLUDED)
        list(APPEND OPENTS_SRC "${f}")
    endif()
endforeach()

#
# ---------------------------------------------------------
# Create actual OpenTS executable (Win32)
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Creating '${PROJECT_NAME}' executable...")

if(WIN32)
    add_executable(OpenTS WIN32 ${OPENTS_SRC})
elseif(APPLE)
    add_executable(OpenTS MACOSX_BUNDLE ${OPENTS_SRC})
else()
    add_executable(OpenTS ${OPENTS_SRC})
endif()
target_compile_features(OpenTS PRIVATE cxx_std_20)

#
# ---------------------------------------------------------
# Output paths
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Setting binary output paths...")

set_target_properties(OpenTS PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"

    # Built under the shipped names, so the executable, the symbol file the debug record
    # names, and the map reach the run directory without a rename.
    OUTPUT_NAME_DEBUG "GameD"
    OUTPUT_NAME_RELEASE "Game"
    PDB_NAME_DEBUG "GameD"
    PDB_NAME_RELEASE "Game"
)

set_target_properties(OpenTS PROPERTIES
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:OpenTS>"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenTS>"
    VS_DEBUGGER_COMMAND_ARGUMENTS "-DATADIR=${OPENTS_GAME_DIR}"
)

if(APPLE)
    # The bundle name and executable name stay apart: the folder is GameD.app or Game.app
    # after the configuration's output name, while the menu reads OpenTS. The version comes
    # from the header GitStamp writes.
    set(OPENTS_BUNDLE_NAME "OpenTS")
    set(OPENTS_BUNDLE_IDENTIFIER "org.opents.OpenTS")
    set(OPENTS_ICON_FILE "OpenTS.icns")

    file(STRINGS "${OPENTS_VERSION_HEADER}" OPENTS_VERSION_DEFINE
        REGEX "^#define OPENTS_VERSION[ ]+\"")
    string(REGEX REPLACE "^#define OPENTS_VERSION[ ]+\"([^\"]*)\".*$"
        "\\1" OPENTS_APP_VERSION "${OPENTS_VERSION_DEFINE}")
    if(OPENTS_APP_VERSION STREQUAL "")
        set(OPENTS_APP_VERSION "${PROJECT_VERSION}")
    endif()

    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        set(OPENTS_MINIMUM_SYSTEM_VERSION "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    else()
        set(OPENTS_MINIMUM_SYSTEM_VERSION "15.0")
    endif()

    set_target_properties(OpenTS PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "${OPENTS_BUNDLE_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${OPENTS_BUNDLE_IDENTIFIER}"
        MACOSX_BUNDLE_ICON_FILE "${OPENTS_ICON_FILE}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${OPENTS_APP_VERSION}"
    )

    find_program(OPENTS_SIPS_EXECUTABLE sips REQUIRED)
    find_program(OPENTS_ICONUTIL_EXECUTABLE iconutil REQUIRED)
    find_program(OPENTS_CODESIGN_EXECUTABLE codesign REQUIRED)

    add_custom_command(TARGET OpenTS POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-DOPENTS_ICON_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/resources/app-icon/opents.ico"
            "-DOPENTS_ICON_OUTPUT=$<TARGET_BUNDLE_CONTENT_DIR:OpenTS>/Resources/${OPENTS_ICON_FILE}"
            "-DOPENTS_ICON_WORK_DIR=${CMAKE_BINARY_DIR}/iconset"
            "-DSIPS_EXECUTABLE=${OPENTS_SIPS_EXECUTABLE}"
            "-DICONUTIL_EXECUTABLE=${OPENTS_ICONUTIL_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/GenerateIcns.cmake"
        COMMENT "Generating the application icon"
        VERBATIM
    )
endif()

#
# ---------------------------------------------------------
# Compile flags
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Applying compiler flags...")

set(OPENTS_COMPILE_OPTIONS

    # ================= DEBUG =================
    $<$<CONFIG:Debug>:

        # ---------- C++ ----------
        # /arch:SSE2 -- IEEE-754 single precision, no x87 excess precision.
        # /fp:precise -- no reassociation of the engine's accumulations.
        # /utf-8 -- sources and literals are UTF-8 whatever the build machine's code page.
        $<$<COMPILE_LANGUAGE:CXX>:
            /Zi /Od /RTC1 /GR $<$<CXX_COMPILER_ID:MSVC>:/MP> /EHsc /Oy- /MTd /Zc:__cplusplus /arch:SSE2 /fp:precise /utf-8
        >

        # ---------- C ----------
        $<$<COMPILE_LANGUAGE:C>:
            /Zi /Od /RTC1 $<$<C_COMPILER_ID:MSVC>:/MP> /Oy- /arch:SSE2 /fp:precise /utf-8
        >
    >

    # ================= RELEASE =================
    $<$<CONFIG:Release>:

        # ---------- C++ ----------
        # /arch:SSE2 -- IEEE-754 single precision, no x87 excess precision.
        # /fp:precise -- no reassociation of the engine's accumulations.
        # /utf-8 -- sources and literals are UTF-8 whatever the build machine's code page.
        $<$<COMPILE_LANGUAGE:CXX>:
            /Zi /O2 /GF /GR $<$<CXX_COMPILER_ID:MSVC>:/MP> /EHsc /MT /Zc:__cplusplus /arch:SSE2 /fp:precise /utf-8
        >

        # ---------- C ----------
        $<$<COMPILE_LANGUAGE:C>:
            /Zi /O2 /GF $<$<C_COMPILER_ID:MSVC>:/MP> /arch:SSE2 /fp:precise /utf-8
        >
    >
)

if(WIN32)
    target_compile_options(OpenTS PRIVATE ${OPENTS_COMPILE_OPTIONS})
else()
    target_compile_options(OpenTS PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
        -fms-extensions
        -ffp-contract=off
    )
endif()

#
# ---------------------------------------------------------
# Subprojects
# ---------------------------------------------------------
#
# The VQA player carries its own project, and hands OpenTS its include path along with the
# library, so the movie code is reached the same way from anywhere in the solution.
add_subdirectory(vqalib)

message(STATUS "${PROJECT_NAME}: Adding include paths...")
target_include_directories(OpenTS PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${OPENTS_GENERATED_DIR}"
)

if(APPLE)
    target_include_directories(OpenTS BEFORE PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos"
    )
endif()

# The generated build stamp has to exist before anything compiles.
add_dependencies(OpenTS OpenTSBuildStamp)

# bgfx's include paths and settings apply only to the two files that include it: the frame
# renderer and the UI overlay renderer.
set(BGFX_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx")
set(OPENTS_BGFX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/bgfxbackend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/rml/rmlrender.cpp"
)
set_source_files_properties(${OPENTS_BGFX_SOURCES} PROPERTIES
    INCLUDE_DIRECTORIES
        "${BGFX_ROOT}/include;${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bx/include"
    COMPILE_DEFINITIONS
        "BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,$<BOOL:${BX_CONFIG_DEBUG}>>"
)
if(MSVC OR OPENTS_EXPERIMENTAL_CLANG_CL)
    set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/bgfxbackend.cpp" APPEND PROPERTY
        COMPILE_OPTIONS "/Zc:preprocessor"
    )
endif()

# The frame quad's shaders come from the imgui example and the overlay's from the debug
# draw example. The imgui example directory also holds an imgui.h that would shadow Dear
# ImGui's, so only the file that needs those shaders gets that path.
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/bgfxbackend.cpp" APPEND PROPERTY
    INCLUDE_DIRECTORIES "${BGFX_ROOT}/examples/common/imgui"
)
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ui/rml/rmlrender.cpp" APPEND PROPERTY
    INCLUDE_DIRECTORIES "${BGFX_ROOT}/examples/common/debugdraw"
)

# writepng.cpp takes miniz from bimg, which the link already carries.
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/writepng.cpp" PROPERTIES
    INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bimg/3rdparty/tinyexr/deps"
)

# bx rewrites __stdcall while its headers are being parsed by clang-cl.  Force the
# compatibility header into the renderer translation units as well as bx/bgfx so the
# MSVC standard-library headers that follow still see the Win32 calling convention.
if(OPENTS_EXPERIMENTAL_CLANG_CL AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_property(SOURCE ${OPENTS_BGFX_SOURCES} APPEND PROPERTY
        COMPILE_OPTIONS "/FI${CMAKE_SOURCE_DIR}/thirdparty/bx-clang-compat.h"
    )
endif()

# The texture loader decodes images with the stb_image header from bimg; no other file needs it.
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ui/rml/rmltexture.cpp" APPEND PROPERTY
    INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bimg/3rdparty/stb"
)

message(STATUS "${PROJECT_NAME}: Adding compilier definitions...")
if(WIN32)
    target_compile_definitions(OpenTS PRIVATE WIN32 _WINDOWS NOMINMAX)
else()
    # The UI sources take MSVC's case-insensitive compares from <string.h> without the engine headers.
    target_compile_definitions(OpenTS PRIVATE OPENTS_MACOS $<$<CONFIG:Debug>:_DEBUG>
        stricmp=strcasecmp strnicmp=strncasecmp)
endif()

#
# ---------------------------------------------------------
# Windows libraries and linker options
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Applying linker options...")

# Required libraries:
#  Comctl32.lib
#  DbgHelp.lib
#  DSound.lib
#  Version.lib
#  Winmm.lib
#  wsock32.lib
#  + all Win32 libs
#

target_link_libraries(OpenTS PRIVATE
    VQALib

    bgfx
    bx
    bimg
    miniaudio
    lzo
    RmlUi::Core
    imgui
    SDL3::SDL3-static
)

if(WIN32)
    target_link_libraries(OpenTS PRIVATE
        comctl32 dbghelp iphlpapi psapi version winmm ws2_32
        kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32
        odbc32 odbccp32
    )
elseif(APPLE)
    target_link_libraries(OpenTS PRIVATE
        "-framework AppKit"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreMedia"
        "-framework IOKit"
        "-framework Metal"
        "-framework QuartzCore"
        "-framework VideoToolbox"
        iconv
    )
endif()

if(MSVC)
    target_link_options(OpenTS PRIVATE
        /SUBSYSTEM:WINDOWS
        /DEBUG
        /MAP
        # Raises the user address space of a 32-bit process from 2GB to 4GB under 64-bit
        # Windows. Pointers may then have the high bit set, so anything holding one in a
        # signed integer must only round-trip the bits and never compare or scale them. A
        # 64-bit image carries the flag already.
        /LARGEADDRESSAWARE
        # Declares the process per-monitor DPI aware, so a scaled desktop does not blur
        # the frame by stretching the whole window.
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/sun.manifest"
        /MANIFEST:EMBED
    )
endif()

#
# ---------------------------------------------------------
# Visual Studio source groups
# ---------------------------------------------------------
#
message(STATUS "${PROJECT_NAME}: Setting up Visual Studio filters...")

# Init Source Files (_*.cpp)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "[/\\\\]_.*\\.cpp$")
        source_group("Source Files\\Init Source Files" FILES ${f})
    endif()
endforeach()

# Init Header Files (_*.h)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "[/\\\\]_.*\\.h$")
        source_group("Header Files\\Init Header Files" FILES ${f})
    endif()
endforeach()

# Define Header Files (*.hh)
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.hh$")
        source_group("Header Files\\Define Header Files" FILES ${f})
    endif()
endforeach()

# Resource Files
foreach(f ${OPENTS_SRC})
    if(f MATCHES "\\.(ico|cur|bmp|gif|jpe?g|rc)$")
        source_group("Resource Files" FILES ${f})
    endif()
endforeach()

# The interface headers the locomotors are written against.
source_group("Interface Files" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/iflyctrl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/iloco.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ipiggy.h"
)

# General source files
source_group("Source Files" REGULAR_EXPRESSION ".*\\.(c|cpp)$")
source_group("Header Files" REGULAR_EXPRESSION ".*\\.(h|hpp)$")

#
# ---------------------------------------------------------
# UI files
# ---------------------------------------------------------
#
# The game reads the ui directory from beside the executable, or from the bundle's
# Resources, which is where a macOS application keeps files it ships. A separate target
# keeps the copy current without a relink.
if(APPLE)
    add_custom_target(OpenTSUIFiles ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/ui" "$<TARGET_BUNDLE_CONTENT_DIR:OpenTS>/Resources/ui"
        # The linker signs the executable before the resources exist, so signing again here
        # gives the bundle the resource envelope its verification expects.
        COMMAND "${OPENTS_CODESIGN_EXECUTABLE}" --force --sign - "$<TARGET_BUNDLE_DIR:OpenTS>"
        DEPENDS OpenTS
        COMMENT "Copying the UI files into the application bundle"
        VERBATIM
    )
else()
    add_custom_target(OpenTSUIFiles ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/ui" "$<TARGET_FILE_DIR:OpenTS>/ui"
        COMMENT "Copying the UI files beside the executable"
        VERBATIM
    )
    add_dependencies(OpenTS OpenTSUIFiles)
endif()

#
# ---------------------------------------------------------
# Visual Studio startup project
# ---------------------------------------------------------
#
if(WIN32)
    set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT OpenTS)
endif()

# ---------------------------------------------------------
# Add the .natvis file for Visual Studio debugging
# ---------------------------------------------------------
if(WIN32)
    set(NATVIS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sun.natvis")
    source_group("Natvis Files" FILES ${NATVIS_FILE})
    target_sources(OpenTS PRIVATE ${NATVIS_FILE})
    target_link_options(OpenTS PRIVATE "/NATVIS:${NATVIS_FILE}")
endif()
