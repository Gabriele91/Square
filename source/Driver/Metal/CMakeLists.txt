cmake_minimum_required(VERSION 3.14...3.31)
project(SqMetal VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Square backend Metal")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Guard is also in the parent CMakeLists, but be explicit here too
if(NOT APPLE)
    message(STATUS "SqMetal is only supported on Apple platforms — skipping.")
    return()
endif()


set(SOURCE_FILES
    InterfaceMTL.mm
    RenderMTL.mm
)

set(HEADER_FILES
    Interface.h
    RenderMTL.h
)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    $<$<CONFIG:Debug>:SQ_MTL_DEBUG_LOG=1>
    $<$<CONFIG:Debug>:SQ_MTL_DEBUG_FRAMES=3>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/glm/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MACOSX_RPATH ON
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework Metal"
    "-framework QuartzCore"
    "-framework Foundation"
)
