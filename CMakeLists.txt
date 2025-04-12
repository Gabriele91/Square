cmake_minimum_required(VERSION 3.1.0)

# Force flags and paths
set(GLM_IS_MASTER_PROJECT OFF CACHE BOOL "Do not install GLM" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "Do not install GLM" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/glm)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "Do not install GLLANG" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "Do not install SPIRV CROSS" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/HLSL2ALL)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static zlib" FORCE)
set(SKIP_INSTALL_ALL TRUE CACHE BOOL "Skip install step for zlib" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/zlib EXCLUDE_FROM_ALL)

# Add square
add_subdirectory(source)
# Add examples
add_subdirectory(example)