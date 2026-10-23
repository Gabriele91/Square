cmake_minimum_required(VERSION 3.14...3.31)

# The tools of the engine
# Android: the tools run on the host (its build makes the assets and the shader pack), only the
# function of the pack (square_shader_pack, called by the games) without a pack made
if(ANDROID)
    set(SQUARE_SHADER_PACK OFF CACHE BOOL "No shader pack made by the Android build (the host makes it)" FORCE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/ShaderPack/ShaderPack.cmake)
    return()
endif()
# Model: the importer of the models (glTF, glb) into the resources of Square
add_subdirectory(Model)
# ShaderPack: the shaders translated offline for the targets of a platform (the pack of a Retail build)
add_subdirectory(ShaderPack)
# AssetPack: the archives of the resources of a game for the release of a platform (the compression
# of its textures, the same images in one file, a report)
add_subdirectory(AssetPack)
# Builder: the builds of a game (cmake, ninja, a compiler or an SDK; Debug, Release, Retail; its
# release, its deploy on a mobile), a command line and a window
add_subdirectory(Builder)
