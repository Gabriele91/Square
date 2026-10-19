cmake_minimum_required(VERSION 3.14...3.31)

# The root of the engine: this folder, also when the engine is a subdirectory of a game (its
# submodule, add_subdirectory)
set(SQUARE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# The examples: built when the engine is the project, not when it is a subdirectory of a game
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SQUARE_TOP_LEVEL ON)
else()
    set(SQUARE_TOP_LEVEL OFF)
endif()
option(SQUARE_BUILD_EXAMPLES "Build the examples of the engine" ${SQUARE_TOP_LEVEL})

# The configurations (as the games do):
# - Debug: not optimized, with the debug tools of the engine;
# - Release: optimized, with the debug tools (the debug panel F1, the render profiler, the debug
#   pass, the logs);
# - Retail: optimized as Release, without any debug tool (SQUARE_RETAIL; only warnings and errors
#   in the log). Single configuration generators: -DCMAKE_BUILD_TYPE=Retail (its own build folder)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Retail" CACHE STRING "The configurations" FORCE)
# (the configuration of a single configuration generator: none with Visual Studio, Ninja Multi-Config)
if(DEFINED CACHE{CMAKE_BUILD_TYPE})
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release Retail)
endif()
foreach(language C CXX)
    set(CMAKE_${language}_FLAGS_RETAIL "${CMAKE_${language}_FLAGS_RELEASE}" CACHE STRING "Flags of ${language} in Retail (as Release)" FORCE)
endforeach()
foreach(kind EXE SHARED MODULE STATIC)
    set(CMAKE_${kind}_LINKER_FLAGS_RETAIL "${CMAKE_${kind}_LINKER_FLAGS_RELEASE}" CACHE STRING "Linker flags of ${kind} in Retail (as Release)" FORCE)
endforeach()
# imported libraries without a Retail configuration: their Release, else the one without a
# configuration (the empty one: Python3::Interpreter of SPIRV-Tools)
set(CMAKE_MAP_IMPORTED_CONFIG_RETAIL Release "")

# Force flags and paths

# GLM
set(GLM_IS_MASTER_PROJECT OFF CACHE BOOL "Do not install GLM" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "Do not install GLM" FORCE)
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/glm)

# Android: the optimized configurations with the link time optimization of every library (ThinLTO
# of clang: the calls across the files inlined), -O3 by the flags of Release
if(ANDROID)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RETAIL ON)
endif()

# The static libraries in the shared ones (HLSL2ALL, Square): position independent code (also in
# Retail, its flags without the -fPIC of the Release ones of HLSL2ALL)
if(UNIX AND NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# HLSL2ALL
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "Do not install GLLANG" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Do not include CSLANG tests" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "Do not install SPIRV CROSS" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Do not compile SPIRV testes" FORCE)
set(ENABLE_HLSL2ALL_TEST OFF CACHE BOOL "Skip HLSL2ALL compilation" FORCE)
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/HLSL2ALL)
# its new/delete on the heap of Square (they share their containers): the hooks Square installs
target_sources(HLSL2ALL PRIVATE ${SQUARE_ROOT_DIR}/source/Core/Allocator/MemoryOperators.cpp)
target_include_directories(HLSL2ALL PRIVATE ${SQUARE_ROOT_DIR}/include)
target_compile_definitions(HLSL2ALL PRIVATE SQUARE_MEMORY_HOOKED)
# its containers: the ones of Square (Square/Resource/HLSL2ALLConfig.h), also for Square
target_compile_definitions(HLSL2ALL PUBLIC "HLSL2ALL_CUSTOM_CONFIGURATION_FILE=\"Square/Resource/HLSL2ALLConfig.h\"")

# Zlib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static zlib" FORCE)
set(SKIP_INSTALL_ALL TRUE CACHE BOOL "Skip install step for zlib" FORCE)
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/zlib EXCLUDE_FROM_ALL)
# its streams on the heap of Square (zcalloc/zcfree: Square/Core/Allocator/MemoryC.cpp)
target_compile_definitions(zlibstatic PRIVATE MY_ZCALLOC)

# minizip of zlib (the archives of Square::Filesystem, .sqz/.zip): its reader and its writer
set(MINIZIP_DIR ${SQUARE_ROOT_DIR}/dependencies/zlib/contrib/minizip)
add_library(square_minizip STATIC ${MINIZIP_DIR}/ioapi.c ${MINIZIP_DIR}/unzip.c ${MINIZIP_DIR}/zip.c)
target_include_directories(square_minizip PUBLIC ${MINIZIP_DIR})
target_link_libraries(square_minizip PUBLIC zlibstatic)
if(MSVC)
    target_compile_definitions(square_minizip PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ASTC encoder (Data::Image::compress_astc, in Square): only the static library, with the SIMD
# of the compiler defaults (one target on every platform: astcenc-native-static)
set(ASTCENC_CLI OFF CACHE BOOL "Do not build the astcenc command line tools" FORCE)
set(ASTCENC_WERROR OFF CACHE BOOL "Do not treat the astcenc warnings as errors" FORCE)
set(ASTCENC_UNIVERSAL_BUILD OFF CACHE BOOL "One astcenc library also on macOS" FORCE)
# (Android: cross compiled, no -march=native of the host; NEON, its baseline on arm64)
if(ANDROID)
    set(ASTCENC_ISA_NATIVE OFF CACHE BOOL "astcenc with NEON on Android" FORCE)
    set(ASTCENC_ISA_NEON ON CACHE BOOL "astcenc with NEON on Android" FORCE)
    set(SQUARE_ASTCENC_LIBRARY astcenc-neon-static)
else()
    set(ASTCENC_ISA_NATIVE ON CACHE BOOL "astcenc with the SIMD of the compiler defaults" FORCE)
    set(SQUARE_ASTCENC_LIBRARY astcenc-native-static)
endif()
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/astc-encoder EXCLUDE_FROM_ALL)

# FreeType (the fonts of RmlUi): static, with our zlib (above, found by its find_package as
# ZLIB::ZLIB), without the other optional dependencies (libpng: only the color emoji fonts)
if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
set(ZLIB_INCLUDE_DIR ${SQUARE_ROOT_DIR}/dependencies/zlib CACHE PATH "Our zlib" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "Our zlib" FORCE)
set(FT_DISABLE_ZLIB OFF CACHE BOOL "FreeType with zlib" FORCE)
set(FT_REQUIRE_ZLIB ON CACHE BOOL "FreeType with our zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "FreeType without bzip2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "FreeType without libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "FreeType without HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "FreeType without Brotli" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip install step" FORCE)
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/freetype EXCLUDE_FROM_ALL)
# its memory on the heap of Square (its standard library: Square/UI/FreeTypeConfig.h)
target_compile_definitions(freetype PRIVATE "FT_CONFIG_STANDARD_LIBRARY_H=\"Square/UI/FreeTypeConfig.h\"")
target_include_directories(freetype PRIVATE ${SQUARE_ROOT_DIR}/include)
# (its system file of Windows, HeapAlloc, and of Unix, malloc: the standard one, on its standard
# library; the fonts of RmlUi from memory, no mapped file to lose)
get_target_property(SQUARE_FREETYPE_SOURCES freetype SOURCES)
list(FILTER SQUARE_FREETYPE_SOURCES EXCLUDE REGEX "builds/(windows|unix)/ftsystem\.c$")
list(APPEND SQUARE_FREETYPE_SOURCES ${SQUARE_ROOT_DIR}/dependencies/freetype/src/base/ftsystem.c)
list(REMOVE_DUPLICATES SQUARE_FREETYPE_SOURCES)
set_property(TARGET freetype PROPERTY SOURCES ${SQUARE_FREETYPE_SOURCES})
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()
# Our FreeType also for the find_package of RmlUi (otherwise it looks for the system one)
set(FREETYPE_INCLUDE_DIR_ft2build ${SQUARE_ROOT_DIR}/dependencies/freetype/include CACHE PATH "Our FreeType" FORCE)
set(FREETYPE_INCLUDE_DIR_freetype2 ${SQUARE_ROOT_DIR}/dependencies/freetype/include CACHE PATH "Our FreeType" FORCE)
set(FREETYPE_LIBRARY freetype CACHE STRING "Our FreeType" FORCE)

# RmlUi (Square::UI, in Square): the static core and the debugger, FreeType fonts, no samples
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "RmlUi with the FreeType font engine" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "No RmlUi samples" FORCE)
set(RMLUI_BACKEND "native" CACHE STRING "No RmlUi backend (the one of Square)" FORCE)
# its containers of the STL: the ones of Square (Square/UI/RmlUiConfig.h, the heap of the engine)
set(RMLUI_CUSTOM_CONFIGURATION ON CACHE BOOL "RmlUi on the containers of Square" FORCE)
set(RMLUI_CUSTOM_CONFIGURATION_FILE "Square/UI/RmlUiConfig.h" CACHE STRING "RmlUi on the containers of Square" FORCE)
set(RMLUI_CUSTOM_INCLUDE_DIRS "${SQUARE_ROOT_DIR}/include" CACHE STRING "The headers of Square" FORCE)
add_subdirectory(${SQUARE_ROOT_DIR}/dependencies/RmlUi EXCLUDE_FROM_ALL)
set(SQUARE_RMLUI_LIBRARIES RmlUi::Core RmlUi::Debugger)

# Driver texture/render target introspection (the tab Textures of the debug panel):
# defined here so EVERY target in the tree inherits it (Square, the render
# backends — C++ and Objective-C++ — and the examples/game), in Debug and Release, never in Retail.
add_compile_definitions($<$<NOT:$<CONFIG:Retail>>:TEXTURE_INTROSPECTION>)

# The debug tools (the debug panel, the debug pass...): every configuration but Retail
add_compile_definitions($<$<NOT:$<CONFIG:Retail>>:SQUARE_DEBUG_TOOLS>)
add_compile_definitions($<$<CONFIG:Retail>:SQUARE_RETAIL>)

# Render profiler (Render/Profiler.h: CPU/GPU times of the render passes and of the shaders):
# on in Debug and Release (never in Retail), off (-DSQUARE_RENDER_PROFILER=OFF) it is not
# compiled at all
option(SQUARE_RENDER_PROFILER "Compile the render profiler (CPU/GPU times of the passes)" ON)
if(SQUARE_RENDER_PROFILER)
    add_compile_definitions($<$<NOT:$<CONFIG:Retail>>:RENDER_PROFILER>)
endif()

# The modules of the engine (engine/cmake): the games on Android (square_add_game,
# square_android_app), for the engine and for the game that has it as its subdirectory
list(APPEND CMAKE_MODULE_PATH ${SQUARE_ROOT_DIR}/cmake)
if(NOT SQUARE_TOP_LEVEL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif()
include(SquareAndroid)

# Add square
add_subdirectory(source)
# Add examples
# Add the tools of the engine (Model: the importer of the models)
add_subdirectory(tools)
# Add examples
if(SQUARE_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()