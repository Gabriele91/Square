cmake_minimum_required(VERSION 3.14...3.31)
project(ShaderPack VERSION 1.0.0 LANGUAGES CXX)

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

# Create executable
add_executable(ShaderPack main.cpp)

# Include directories
target_include_directories(ShaderPack PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/glm/
)

# Link dependencies
target_link_libraries(ShaderPack PRIVATE Square)

# Platform specific configurations
if(WIN32)
    set_target_properties(ShaderPack PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        target_compile_options(ShaderPack PRIVATE "/arch:AVX")
    endif()
endif()
