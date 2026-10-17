cmake_minimum_required(VERSION 3.14...3.31)
project(Square VERSION 1.0.0 DESCRIPTION "Square render engine")

# C++17
set (CMAKE_CXX_STANDARD 17)
# The libraries of each configuration a name of their own (Square and its drivers: never
# overwritten by another configuration): Debug "d" (Squared.dll), Release "r" (Squarer.dll),
# Retail none (Square.dll). The drivers are found by their exports, whatever their names
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_RELEASE_POSTFIX r)
set(CMAKE_RETAIL_POSTFIX "")
# Flags
if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MD")
  set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDISABLE_SHARER_PREPROCESS_LINE_SET")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DDISABLE_SHARER_PREPROCESS_LINE_SET")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fPIC -pthread -DDISABLE_SHARER_PREPROCESS_LINE_SET")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fPIC -pthread -DDISABLE_SHARER_PREPROCESS_LINE_SET")
endif()
# Retail: the flags of Release (the same shaders preprocessed)
set(CMAKE_CXX_FLAGS_RETAIL "${CMAKE_CXX_FLAGS_RELEASE}")

# Square project
file(GLOB SOURCES Core/*.cpp
                  Core/Allocator/*.cpp
                  Core/Container/*.cpp
                  Audio/*.cpp
                  Data/*.cpp
                  Data/ImageUtils/*.cpp
                  Geometry/*.cpp
                  Driver/*.cpp
                  Math/*.cpp
                  Navigation/*.cpp
                  Render/*.cpp
                  Render/PostEffect/*.cpp
                  Render/Pipeline/*.cpp
                  Resource/*.cpp
                  Scene/*.cpp
                  System/*.cpp
                  UI/*.cpp)

set(OS_FLAGS "")
set(OS_SOURCES "")
set(OS_LIBS "")
set(INSTALL_DY_LIBS "")
if(WIN32)
    file(GLOB WIN_SRC Driver/Win32/*.cpp)
    list(APPEND OS_SOURCES ${WIN_SRC} Square.rc)
    list(APPEND OS_FLAGS SQUARE_LIBRARY_EXPORT
                         HLSL2ALL_LIBRARY_IMPORT
                         _CRT_SECURE_NO_WARNINGS
                         _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING) 
    list(APPEND OS_LIBS Shlwapi.lib
                        Gdi32.lib
                        Opengl32.lib)
                        
    # Add DX11 Driver
    add_subdirectory(Driver/DirectX11)
    list(APPEND INSTALL_DY_LIBS SqDX11)

    # Add Vulkan Driver
    add_subdirectory(Driver/Vulkan)
    list(APPEND INSTALL_DY_LIBS SqVulkan)

elseif(APPLE)
    file(GLOB APPLE_SRC Driver/Cocoa/*.mm)
    list(APPEND OS_SOURCES ${APPLE_SRC})
    list(APPEND OS_FLAGS GL_SILENCE_DEPRECATION)
    set_source_files_properties(${APPLE_SRC} PROPERTIES
        COMPILE_OPTIONS "-Wno-deprecated-declarations;-Wno-incompatible-pointer-types"
    )
    list(APPEND OS_LIBS
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework Metal"
        "-framework CoreVideo"
        "-framework QuartzCore"
        "-framework CoreGraphics"
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )

elseif(UNIX AND NOT APPLE)
  file(GLOB LINUX_SRC Driver/Xorg/*.cpp)
  list(APPEND OS_SOURCES ${LINUX_SRC})
  list(APPEND OS_LIBS X11
                      Xrandr
                      GLU
                      GL
                      xcb
                      dl
                      pthread
                      m)

  # Add Vulkan Driver
  add_subdirectory(Driver/Vulkan)
  list(APPEND INSTALL_DY_LIBS SqVulkan)
endif()

# Add OGL Driver
add_subdirectory(Driver/OpenGL4)
list(APPEND INSTALL_DY_LIBS SqOpenGL)

# Add Metal Driver (Apple only)
if(APPLE)
    # Enable OBJCXX here in the parent scope so the Metal subproject
    # can inherit it. Without this the subproject's enable_language(OBJCXX)
    # fails because CMAKE_OBJCXX_COMPILE_OBJECT is not yet in the cache.
    enable_language(OBJCXX)
    add_subdirectory(Driver/Metal)
    list(APPEND INSTALL_DY_LIBS SqMetal)
endif()

# Executable (its new/delete on the heap of the engine: Core/Allocator/MemoryOperators.cpp)
add_library(Square SHARED ${SOURCES} ${OS_SOURCES} Audio/AudioLibrary.c)

# miniaudio (the AudioSystem): built once in Square (Audio/AudioLibrary.c, with stb_vorbis), its
# warnings its own; no encoder (the engine only plays)
target_compile_definitions(Square PRIVATE MA_NO_ENCODING)
if(MSVC)
    set_source_files_properties(Audio/AudioLibrary.c PROPERTIES COMPILE_OPTIONS "/W0")
else()
    set_source_files_properties(Audio/AudioLibrary.c PROPERTIES COMPILE_OPTIONS "-w")
endif()

# The new/delete of the heap of the engine also in each module that links Square (they share their
# memory with it: a std::string made in one, freed in the other; see Square/Core/Allocator/MemoryOperators.h)
target_sources(Square INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/Allocator/MemoryOperators.cpp>)
target_include_directories(Square INTERFACE $<BUILD_INTERFACE:${SQUARE_ROOT_DIR}/include>)
# the configuration of RmlUi (its containers: the ones of Square) also in the modules that include
# the headers of Square::UI
target_compile_definitions(Square INTERFACE "RMLUI_CUSTOM_CONFIGURATION_FILE=\"${RMLUI_CUSTOM_CONFIGURATION_FILE}\"")

# Flags
target_compile_definitions(Square PRIVATE ${OS_FLAGS})

# The definitions of the configuration (the root of the engine: its directory) also for whoever
# links Square out of its tree (a game with the engine as its subdirectory): the same headers
target_compile_definitions(Square INTERFACE
    $<$<NOT:$<CONFIG:Retail>>:TEXTURE_INTROSPECTION>
    $<$<NOT:$<CONFIG:Retail>>:SQUARE_DEBUG_TOOLS>
    $<$<CONFIG:Retail>:SQUARE_RETAIL>
)
if(SQUARE_RENDER_PROFILER)
    target_compile_definitions(Square INTERFACE $<$<NOT:$<CONFIG:Retail>>:RENDER_PROFILER>)
endif()

# The vector instructions of the CPU code (Render/Pipeline/SoftwareOcclusion: its raster):
# SQUARE_SIMD AVX (x86/x64, the default: the games need it too), SSE4 (SSE 4.1), NONE (the scalar
# code). ARM (NEON) is found by the compiler, whatever it says
set(SQUARE_SIMD "AVX" CACHE STRING "Vector instructions of the CPU code: AVX, SSE4, NONE")
set_property(CACHE SQUARE_SIMD PROPERTY STRINGS AVX SSE4 NONE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(i.86)")
    if(SQUARE_SIMD STREQUAL "AVX")
        target_compile_definitions(Square PRIVATE SQUARE_SIMD_AVX)
        if(MSVC)
            target_compile_options(Square PRIVATE "/arch:AVX")
        else()
            target_compile_options(Square PRIVATE -mavx)
        endif()
    elseif(SQUARE_SIMD STREQUAL "SSE4")
        target_compile_definitions(Square PRIVATE SQUARE_SIMD_SSE4)
        if(NOT MSVC)
            target_compile_options(Square PRIVATE -msse4.1)
        endif()
    endif()
endif()

if(NOT MSVC)
    target_compile_options(Square PRIVATE
        -Wno-unused-value
        -Wno-unused-but-set-variable
        -Wno-braced-scalar-init
    )
endif()

# Include directories
target_include_directories(Square PRIVATE
    ${SQUARE_ROOT_DIR}/include
    ${SQUARE_ROOT_DIR}/dependencies/miniaudio
    ${SQUARE_ROOT_DIR}/dependencies/zlib
    ${SQUARE_ROOT_DIR}/dependencies/glm
    ${SQUARE_ROOT_DIR}/dependencies/HLSL2ALL/include
    ${SQUARE_ROOT_DIR}/dependencies/astc-encoder/Source
)

# Splash screen (Render/SplashScreen.cpp): the icon inside the library, a resource of Square.dll
# on Windows (Square.rc), the .incbin of its path elsewhere
set(SQUARE_SPLASH_IMAGE ${SQUARE_ROOT_DIR}/common/texture/square_icon.png)
if(WIN32)
    target_include_directories(Square PRIVATE $<$<COMPILE_LANGUAGE:RC>:${SQUARE_ROOT_DIR}/common/texture>)
    set_source_files_properties(Square.rc PROPERTIES OBJECT_DEPENDS ${SQUARE_SPLASH_IMAGE})
else()
    set_source_files_properties(Render/SplashScreen.cpp PROPERTIES
        COMPILE_DEFINITIONS "SQUARE_SPLASH_IMAGE_PATH=\"${SQUARE_SPLASH_IMAGE}\""
        OBJECT_DEPENDS ${SQUARE_SPLASH_IMAGE}
    )
endif()

# Link libraries
target_link_libraries(Square PRIVATE 
    HLSL2ALL 
    zlibstatic
    square_minizip
    ${SQUARE_ASTCENC_LIBRARY}
    ${SQUARE_RMLUI_LIBRARIES}
    ${OS_LIBS}
)

# Assets target
add_custom_target(SquareAssets ALL COMMENT "Target for assets installation")

# Install headers: the engine as the project only (its SDK); a game with the engine as its
# subdirectory installs the game, not the headers of the engine
if(SQUARE_TOP_LEVEL)
    install(DIRECTORY ${SQUARE_ROOT_DIR}/include/ DESTINATION include)
    install(DIRECTORY ${SQUARE_ROOT_DIR}/dependencies/glm/glm DESTINATION include)
    install(DIRECTORY ${SQUARE_ROOT_DIR}/dependencies/HLSL2ALL/include/ DESTINATION include)
endif()

# Install targets
install(TARGETS Square HLSL2ALL ${INSTALL_DY_LIBS}
    EXPORT SquareTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# the resources of the engine (common/): in the root of the installation, or where the game finds
# them (its engine_dir: SQUARE_INSTALL_COMMON_DIR, e.g. engine/common)
if(NOT SQUARE_INSTALL_COMMON_DIR)
    set(SQUARE_INSTALL_COMMON_DIR common)
endif()
install(DIRECTORY ../common/
        DESTINATION ${SQUARE_INSTALL_COMMON_DIR}/
        COMPONENT SquareAssets)