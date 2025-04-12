cmake_minimum_required(VERSION 3.14)
project(HelloWorldCSM VERSION 1.0.0 LANGUAGES CXX)

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
)

# Create executable
add_executable(HelloWorldCSM ${SOURCE_FILES})

# Include directories
target_include_directories(HelloWorldCSM PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/glm/
    ${CMAKE_SOURCE_DIR}/dependencies/glad/include
)

# Link dependencies
target_link_libraries(HelloWorldCSM PRIVATE Square)

# Platform specific configurations
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library")
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD")
    set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")
    # Set Windows-specific properties
    set_target_properties(HelloWorldCSM PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Advanced Vector Extensions for x86/x64
    if(MSVC)
        target_compile_options(HelloWorldCSM PRIVATE "/arch:AVX")
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
add_custom_target(HelloWorldCSMAssets ALL COMMENT "Target for assets installation")

# Install targets
install(TARGETS HelloWorldCSM
    EXPORT HelloWorldCSMTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(DIRECTORY assets/
        DESTINATION example/HelloWorldCsm/assets/
        COMPONENT HelloWorldCSMAssets)

install(FILES resources.rs
        DESTINATION example/HelloWorldCsm/
        COMPONENT HelloWorldCSMAssets)