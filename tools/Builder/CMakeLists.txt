cmake_minimum_required(VERSION 3.14...3.31)
project(Builder VERSION 1.0.0 LANGUAGES CXX)

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

# Create executable: the builds of a game (a tool of the command line, its console; without a
# build asked, its window: the UI of the engine)
add_executable(Builder
    main.cpp
    Build.cpp
    Process.cpp
    Settings.cpp
    Toolchain.cpp
)
# (the engine of the tool: its UI, its styles; the engine of a game without one of its own)
target_compile_definitions(Builder PRIVATE SQUARE_BUILDER_ENGINE_DIR="${SQUARE_ROOT_DIR}")

# Include directories
target_include_directories(Builder PRIVATE
    ${SQUARE_ROOT_DIR}/include
    ${SQUARE_ROOT_DIR}/dependencies/glm/
)

# Link dependencies
target_link_libraries(Builder PRIVATE Square)

if(MSVC)
    target_compile_options(Builder PRIVATE "/arch:AVX")
endif()
