cmake_minimum_required(VERSION 3.14...3.31)

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

# ASTC encoder (Data::Image::compress_astc, in Square): only the static library, with the SIMD
# of the compiler defaults (one target on every platform: astcenc-native-static)
set(ASTCENC_CLI OFF CACHE BOOL "Do not build the astcenc command line tools" FORCE)
set(ASTCENC_WERROR OFF CACHE BOOL "Do not treat the astcenc warnings as errors" FORCE)
set(ASTCENC_UNIVERSAL_BUILD OFF CACHE BOOL "One astcenc library also on macOS" FORCE)
set(ASTCENC_ISA_NATIVE ON CACHE BOOL "astcenc with the SIMD of the compiler defaults" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/astc-encoder EXCLUDE_FROM_ALL)
set(SQUARE_ASTCENC_LIBRARY astcenc-native-static)

# FreeType (the fonts of RmlUi): static, without its optional dependencies
set(FT_DISABLE_ZLIB ON CACHE BOOL "FreeType without zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "FreeType without bzip2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "FreeType without libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "FreeType without HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "FreeType without Brotli" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip install step" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/freetype EXCLUDE_FROM_ALL)
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# RmlUi (Square::UI, in Square): the static core and the debugger, FreeType fonts, no samples
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "RmlUi with the FreeType font engine" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "No RmlUi samples" FORCE)
set(RMLUI_BACKEND "native" CACHE STRING "No RmlUi backend (the one of Square)" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/RmlUi EXCLUDE_FROM_ALL)
set(SQUARE_RMLUI_LIBRARIES RmlUi::Core RmlUi::Debugger)

# Driver texture/render target introspection (debug texture panel):
# defined here so EVERY target in the tree inherits it (Square, the render
# backends — C++ and Objective-C++ — and the examples/game), only in Debug.
add_compile_definitions($<$<CONFIG:Debug>:TEXTURE_INTROSPECTION>)

# Add square
add_subdirectory(source)
# Add examples
add_subdirectory(example)