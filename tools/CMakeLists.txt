cmake_minimum_required(VERSION 3.14...3.31)

# The tools of the engine
# Model: the importer of the models (glTF, glb) into the resources of Square
add_subdirectory(Model)
# ShaderPack: the shaders translated offline for the targets of a platform (the pack of a Retail build)
add_subdirectory(ShaderPack)
