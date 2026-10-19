cmake_minimum_required(VERSION 3.14...3.31)
project(Model VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Preprocessor definitions
add_definitions(-DSQUARE_LIBRARY_IMPORT)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

# Source files
set(SOURCE_FILES
    main.cpp
    ${SQUARE_ROOT_DIR}/tools/Common/ImageConverter.cpp
    SquareExtras.cpp
    TextureManager.cpp
    MaterialManager.cpp
    MeshManager.cpp
    LodGroups.cpp
    InstanceGroups.cpp
    Occluders.cpp
    Sprites.cpp
    Particles.cpp
    Animations.cpp
    Skins.cpp
)

# Create executable
add_executable(Model ${SOURCE_FILES})
# The engine it is built from (its common/: the effects, the shaders, the textures), whatever
# folder it runs from (the root of the engine, a game with the engine in a folder of its own)
target_compile_definitions(Model PRIVATE SQUARE_ENGINE_DIR="${SQUARE_ROOT_DIR}")

# Include directories
target_include_directories(Model PRIVATE 
    ${SQUARE_ROOT_DIR}/include
    ${SQUARE_ROOT_DIR}/tools/Common
    ${SQUARE_ROOT_DIR}/dependencies/glm/
    ${SQUARE_ROOT_DIR}/dependencies/glad/include
)

# Link dependencies
target_link_libraries(Model PRIVATE Square)

# Platform specific configurations
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MD")
    set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")
    # Set Windows-specific properties
    set_target_properties(Model PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Advanced Vector Extensions for x86/x64
    if(MSVC)
        target_compile_options(Model PRIVATE "/arch:AVX")
    endif()

    # Platform-specific include directories
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit platform
        set(PLATFORM_NAME "x64")
    else()
        # 32-bit platform
        set(PLATFORM_NAME "Win32")
    endif()
endif()

# Assets target
add_custom_target(ModelAssets ALL COMMENT "Target for assets installation")

# Install targets
install(TARGETS Model
    EXPORT ModelTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)