cmake_minimum_required(VERSION 3.14...3.31)
project(SqDX11 VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Square backend DirectX")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    InterfaceDX11.cpp
    RenderDX11.cpp
)

# Define header files
set(HEADER_FILES
    Interface.h
    DirectX11.h
    RenderDX11.h
)

# Create library target
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

# Set preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)

# Set compiler options
target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX)

# Set include directories
target_include_directories(
    ${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/glm/
)

# Set output directories for all build types
set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Platform-specific dependencies for Windows
if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE " /Ox /MD")
    set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        GDI32
        d3d11
        dxgi
        d3dcompiler
    )
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_NAME "x64")
    else()
        set(PLATFORM_NAME "Win32")
    endif()
else()
    message(FATAL_ERROR "Unsupported platform. This project is intended for Windows only.")
endif()

