cmake_minimum_required(VERSION 3.14...3.31)

# The configurations (as the games do):
# - Debug: not optimized, with the debug tools of the engine;
# - Release: optimized, with the debug tools (the debug panel F1, the render profiler, the debug
#   pass, the logs);
# - Retail: optimized as Release, without any debug tool (SQUARE_RETAIL; only warnings and errors
#   in the log). Single configuration generators: -DCMAKE_BUILD_TYPE=Retail (its own build folder)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Retail" CACHE STRING "The configurations" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release Retail)
foreach(language C CXX)
    set(CMAKE_${language}_FLAGS_RETAIL "${CMAKE_${language}_FLAGS_RELEASE}" CACHE STRING "Flags of ${language} in Retail (as Release)" FORCE)
endforeach()
foreach(kind EXE SHARED MODULE STATIC)
    set(CMAKE_${kind}_LINKER_FLAGS_RETAIL "${CMAKE_${kind}_LINKER_FLAGS_RELEASE}" CACHE STRING "Linker flags of ${kind} in Retail (as Release)" FORCE)
endforeach()
# imported libraries without a Retail configuration: their Release
set(CMAKE_MAP_IMPORTED_CONFIG_RETAIL Release)

# Force flags and paths

# GLM
set(GLM_IS_MASTER_PROJECT OFF CACHE BOOL "Do not install GLM" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "Do not install GLM" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/glm)

# HLSL2ALL
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "Do not install GLLANG" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Do not include CSLANG tests" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "Do not install SPIRV CROSS" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Do not compile SPIRV testes" FORCE)
set(ENABLE_HLSL2ALL_TEST OFF CACHE BOOL "Skip HLSL2ALL compilation" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/HLSL2ALL)

# Zlib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static zlib" FORCE)
set(SKIP_INSTALL_ALL TRUE CACHE BOOL "Skip install step for zlib" FORCE)
if(UNIX AND NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/zlib EXCLUDE_FROM_ALL)

# minizip of zlib (the archives of Square::Filesystem, .sqz/.zip): its reader and its writer
set(MINIZIP_DIR ${PROJECT_SOURCE_DIR}/dependencies/zlib/contrib/minizip)
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
set(ASTCENC_ISA_NATIVE ON CACHE BOOL "astcenc with the SIMD of the compiler defaults" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/astc-encoder EXCLUDE_FROM_ALL)
set(SQUARE_ASTCENC_LIBRARY astcenc-native-static)

# FreeType (the fonts of RmlUi): static, with our zlib (above, found by its find_package as
# ZLIB::ZLIB), without the other optional dependencies (libpng: only the color emoji fonts)
if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/zlib CACHE PATH "Our zlib" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "Our zlib" FORCE)
set(FT_DISABLE_ZLIB OFF CACHE BOOL "FreeType with zlib" FORCE)
set(FT_REQUIRE_ZLIB ON CACHE BOOL "FreeType with our zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "FreeType without bzip2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "FreeType without libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "FreeType without HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "FreeType without Brotli" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip install step" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/freetype EXCLUDE_FROM_ALL)
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()
# Our FreeType also for the find_package of RmlUi (otherwise it looks for the system one)
set(FREETYPE_INCLUDE_DIR_ft2build ${PROJECT_SOURCE_DIR}/dependencies/freetype/include CACHE PATH "Our FreeType" FORCE)
set(FREETYPE_INCLUDE_DIR_freetype2 ${PROJECT_SOURCE_DIR}/dependencies/freetype/include CACHE PATH "Our FreeType" FORCE)
set(FREETYPE_LIBRARY freetype CACHE STRING "Our FreeType" FORCE)

# RmlUi (Square::UI, in Square): the static core and the debugger, FreeType fonts, no samples
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "RmlUi with the FreeType font engine" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "No RmlUi samples" FORCE)
set(RMLUI_BACKEND "native" CACHE STRING "No RmlUi backend (the one of Square)" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/RmlUi EXCLUDE_FROM_ALL)
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

# Add square
add_subdirectory(source)
# Add examples
add_subdirectory(example)