cmake_minimum_required(VERSION 3.14...3.31)
project(SqOpenGL VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Square backend DirectX")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    InterfaceGL4.cpp
    RenderGL4.cpp
)

# Define header files
set(HEADER_FILES
    Interface.h
    OpenGL4.h
    RenderGL4.h
)

# Path to glad directory
set(GLAD_SOURCES_DIR "${CMAKE_SOURCE_DIR}/dependencies/glad/")
# install jinja2
find_package(Python REQUIRED COMPONENTS Interpreter)

# Check if jinja2 is already installed
execute_process(
    COMMAND ${Python_EXECUTABLE} -c "import jinja2"
    RESULT_VARIABLE jinja2_installed
)

# If not installed, install it
if(NOT jinja2_installed EQUAL 0)
    message(STATUS "Jinja2 not found, installing via pip...")
    execute_process(
        COMMAND ${Python_EXECUTABLE} -m pip install jinja2==3.1 --break-system-packages
        RESULT_VARIABLE install_jinja2_result
        OUTPUT_VARIABLE install_jinja2_output
        ERROR_VARIABLE install_jinja2_error
    )
    if(NOT install_jinja2_result EQUAL 0)
        message(FATAL_ERROR "Failed to install Jinja2: ${install_jinja2_error}")
    endif()
else()
    message(STATUS "Jinja2 already installed.")
endif()

# Path to glad cmake files
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)

# Specify glad settings
glad_add_library(glad_gl_core_41 REPRODUCIBLE API gl:core=4.1)

# Create library target
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

# Set preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    $<$<PLATFORM_ID:Darwin>:GL_SILENCE_DEPRECATION>
)

# Set compiler options
if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX)
endif()

# Set include directories
target_include_directories(
    ${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/glm/
    ${CMAKE_SOURCE_DIR}/dependencies/glad/include
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
        glad_gl_core_41
        GDI32
        OpenGL32
        GLU32
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_NAME "x64")
    else()
        set(PLATFORM_NAME "Win32")
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        glad_gl_core_41
        GLU
        GL
        xcb
        dl
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        glad_gl_core_41
        "-framework OpenGL"
    )
endif()

