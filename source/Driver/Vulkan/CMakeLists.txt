cmake_minimum_required(VERSION 3.14...3.31)
project(SqVulkan VERSION 1.0.0 LANGUAGES C CXX DESCRIPTION "Square backend Vulkan")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan without an SDK: its headers (Vulkan-Headers), its functions loaded at run time from the
# loader of the system (volk), its memory (VMA); the validation layers: the Vulkan SDK, if any
set(VULKAN_HEADERS_DIR ${SQUARE_ROOT_DIR}/dependencies/Vulkan-Headers/include)
set(VOLK_DIR ${SQUARE_ROOT_DIR}/dependencies/volk)
set(VMA_DIR ${SQUARE_ROOT_DIR}/dependencies/VulkanMemoryAllocator/include)

# Define source files
set(SOURCE_FILES
    ${SQUARE_ROOT_DIR}/source/Core/Allocator/MemoryOperators.cpp
    ${SQUARE_ROOT_DIR}/source/Core/Container/String.cpp
    AllocationVK.cpp
    InterfaceVK.cpp
    RenderVK.cpp
    ResourcesVK.cpp
    ShaderVK.cpp
    SpirvReflection.cpp
    VmaVK.cpp
    ${VOLK_DIR}/volk.c
)
# Android: the surface of its window (ANativeWindow); Xorg: the surface of its window (Xlib)
if(ANDROID)
    list(APPEND SOURCE_FILES SurfaceAndroidVK.cpp)
elseif(UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES SurfaceVK.cpp)
endif()

# Define header files
set(HEADER_FILES
    AllocationVK.h
    Interface.h
    RenderVK.h
    SpirvReflection.h
    VulkanConfig.h
    SurfaceVK.h
)

# Create library target
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

# its new/delete on the heap of Square (the hooks Square installs when it loads the driver)
target_compile_definitions(${PROJECT_NAME} PRIVATE SQUARE_MEMORY_HOOKED)

# Set preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    VK_NO_PROTOTYPES
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# Set compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX)
endif()

# Set include directories
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    ${SQUARE_ROOT_DIR}/include
    ${SQUARE_ROOT_DIR}/dependencies/glm/
    ${VULKAN_HEADERS_DIR}
    ${VOLK_DIR}
    ${VMA_DIR}
)

# Set output directories for all build types
set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Platform-specific dependencies
if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MD")
    set(CMAKE_CXX_FLAGS_DEBUG " /Zi /Od /MDd")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
    if(ANDROID)
        target_link_libraries(${PROJECT_NAME} PRIVATE android log)
    elseif(NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE X11)
    endif()
endif()
