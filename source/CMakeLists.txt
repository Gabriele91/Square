cmake_minimum_required(VERSION 3.14...3.31)
project(Square VERSION 1.0.0 DESCRIPTION "Square render engine")

# C++17
set (CMAKE_CXX_STANDARD 17)
# Add debug prefix
if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()
# Flags
if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD")
  set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDISABLE_SHARER_PREPROCESS_LINE_SET")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DDISABLE_SHARER_PREPROCESS_LINE_SET")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fPIC -pthread -DDISABLE_SHARER_PREPROCESS_LINE_SET")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC -pthread -DDISABLE_SHARER_PREPROCESS_LINE_SET")
endif()

# Square project
file(GLOB SOURCES Core/*.cpp
                  Data/*.cpp
                  Data/ImageUtils/*.cpp
                  Geometry/*.cpp
                  Driver/*.cpp
                  Math/*.cpp
                  Render/*.cpp
                  Resource/*.cpp
                  Scene/*.cpp)

set(OS_FLAGS "")
set(OS_SOURCES "")
set(OS_LIBS "")
set(INSTALL_DY_LIBS "")
if(WIN32)
    file(GLOB WIN_SRC Driver/Win32/*.cpp)
    list(APPEND OS_SOURCES ${WIN_SRC})
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
    )

elseif(UNIX AND NOT APPLE)
  file(GLOB LINUX_SRC Driver/Xorg/*.cpp)
  list(APPEND OS_SOURCES ${LINUX_SRC})
  list(APPEND OS_LIBS X11
                      Xxf86vm
                      GLU
                      GL
                      xcb
                      dl)
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

# Executable
add_library(Square SHARED ${SOURCES} ${OS_SOURCES})

# Flags
target_compile_definitions(Square PRIVATE ${OS_FLAGS})

if(NOT MSVC)
    target_compile_options(Square PRIVATE
        -Wno-unused-value
        -Wno-unused-but-set-variable
        -Wno-braced-scalar-init
    )
endif()

# Include directories
target_include_directories(Square PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dependencies/zlib
    ${CMAKE_SOURCE_DIR}/dependencies/glm
    ${CMAKE_SOURCE_DIR}/dependencies/HLSL2ALL/include
)

# Link libraries
target_link_libraries(Square PRIVATE 
    HLSL2ALL 
    zlibstatic
    ${OS_LIBS}
)

# Assets target
add_custom_target(SquareAssets ALL COMMENT "Target for assets installation")

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies/glm/glm DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies/HLSL2ALL/include/ DESTINATION include)

# Install targets
install(TARGETS Square HLSL2ALL ${INSTALL_DY_LIBS}
    EXPORT SquareTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(DIRECTORY ../common/
        DESTINATION common/
        COMPONENT SquareAssets)