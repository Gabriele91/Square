cmake_minimum_required(VERSION 3.14...3.31)
project(AssetPack VERSION 1.0.0 LANGUAGES CXX)

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

# Create executable: the archives of a release (a tool of the command line: its console)
add_executable(AssetPack
    main.cpp
    ArchiveFiles.cpp
    AssetReport.cpp
    TextureDescriptor.cpp
    TexturePacker.cpp
    ${SQUARE_ROOT_DIR}/tools/Common/ImageConverter.cpp
)

# Include directories
target_include_directories(AssetPack PRIVATE
    ${SQUARE_ROOT_DIR}/include
    ${SQUARE_ROOT_DIR}/dependencies/glm/
    ${SQUARE_ROOT_DIR}/tools/Common
)

# Link dependencies
target_link_libraries(AssetPack PRIVATE Square)

if(MSVC)
    target_compile_options(AssetPack PRIVATE "/arch:AVX")
endif()
