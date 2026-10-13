# The shader pack of a Retail build (shaders.sqz: the shaders translated offline by ShaderPack for
# every target of this platform, read by the engine before its cache), made by the build.
#
# The pack in the build folder (<build>/shaders.sqz), installed in the root of the installation
# (cmake --install: the resource folder of the game, next to its resources.rs). By default the pack
# of the engine only (common/common.rs). A game makes one pack of the engine and of its own shaders
# (the engine reads one pack only) by calling, after add_subdirectory(engine):
#
#   square_shader_pack(
#       RESOURCES resources.rs     # its resource files (.rs), relative to WORKING_DIRECTORY
#       DEPENDS   effect shader    # its effects and shaders (folders, files): the pack made again when they change
#       [WORKING_DIRECTORY <dir>]  # its resource folder (default: the folder of the call)
#       [PACK <file>]              # the pack (default: <build>/shaders.sqz)
#       [GAMMA all|on|off]         # the gamma corrections of the game (default: all)
#   )
#
# Only in Retail (-DCMAKE_BUILD_TYPE=Retail; with a multi configuration generator, the Retail one).
# ShaderPack opens a window (the OpenGL backend, Metal on Apple): the build needs a display.
# SQUARE_SHADER_PACK=OFF: no pack made.

option(SQUARE_SHADER_PACK "Make the shader pack (shaders.sqz) in the Retail builds" ON)
set_property(GLOBAL PROPERTY SQUARE_SHADER_PACK_ROOT "${SQUARE_ROOT_DIR}")

function(square_shader_pack)
    cmake_parse_arguments(SP "" "WORKING_DIRECTORY;PACK;GAMMA" "RESOURCES;DEPENDS" ${ARGN})
    if(NOT SQUARE_SHADER_PACK)
        return()
    endif()
    # one pack only (the engine reads one): the one of the game in place of the default one
    get_property(made GLOBAL PROPERTY SQUARE_SHADER_PACK_MADE)
    if(made)
        message(FATAL_ERROR "square_shader_pack: one pack only (already made by ${made})")
    endif()
    set_property(GLOBAL PROPERTY SQUARE_SHADER_PACK_MADE "${CMAKE_CURRENT_LIST_FILE}")
    # Retail only: a single configuration in Retail, or a multi configuration generator
    get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(NOT multi_config AND NOT CMAKE_BUILD_TYPE STREQUAL "Retail")
        return()
    endif()
    # defaults
    if(NOT SP_WORKING_DIRECTORY)
        set(SP_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    get_filename_component(SP_WORKING_DIRECTORY "${SP_WORKING_DIRECTORY}" ABSOLUTE)
    if(NOT SP_PACK)
        set(SP_PACK ${CMAKE_BINARY_DIR}/shaders.sqz)
    endif()
    get_filename_component(SP_PACK "${SP_PACK}" ABSOLUTE BASE_DIR "${SP_WORKING_DIRECTORY}")
    if(NOT SP_GAMMA)
        set(SP_GAMMA all)
    endif()
    # the backend started by the tool (it only translates) and its driver
    if(APPLE)
        set(backend mtl)
        set(driver SqMetal)
    else()
        set(backend ogl)
        set(driver SqOpenGL)
    endif()
    # what the pack is made from: the shaders of the engine, the ones of the game, the tool
    get_property(root GLOBAL PROPERTY SQUARE_SHADER_PACK_ROOT)
    file(GLOB_RECURSE depends CONFIGURE_DEPENDS
        ${root}/common/common.rs
        ${root}/common/effect/*
        ${root}/common/shader/*
    )
    foreach(item ${SP_RESOURCES} ${SP_DEPENDS})
        get_filename_component(item "${item}" ABSOLUTE BASE_DIR "${SP_WORKING_DIRECTORY}")
        if(IS_DIRECTORY "${item}")
            file(GLOB_RECURSE files CONFIGURE_DEPENDS ${item}/*)
            list(APPEND depends ${files})
        else()
            list(APPEND depends ${item})
        endif()
    endforeach()
    # the resources of the game (common: always, by the tool)
    set(resources "")
    if(SP_RESOURCES)
        string(REPLACE ";" "," resources "${SP_RESOURCES}")
        set(resources -r ${resources})
    endif()
    # the drivers found by the tool: next to it (Windows), else by the paths of the libraries
    if(WIN32)
        set(run "")
    elseif(APPLE)
        set(run ${CMAKE_COMMAND} -E env DYLD_LIBRARY_PATH=$<TARGET_FILE_DIR:${driver}>)
    else()
        set(run ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=$<TARGET_FILE_DIR:${driver}>)
    endif()
    set(output ${CMAKE_BINARY_DIR}/shaderpack)
    add_custom_command(
        OUTPUT ${SP_PACK}
        COMMAND ${run} $<TARGET_FILE:ShaderPack> ${resources} -o ${output} -p ${SP_PACK} -g ${SP_GAMMA} -b ${backend}
        DEPENDS ShaderPack ${driver} ${depends}
        WORKING_DIRECTORY ${SP_WORKING_DIRECTORY}
        COMMENT "ShaderPack: ${SP_PACK}"
        VERBATIM
    )
    add_custom_target(SquareShaderPack ALL DEPENDS ${SP_PACK})
    # a multi configuration generator: built with Retail only
    if(multi_config)
        foreach(config ${CMAKE_CONFIGURATION_TYPES})
            if(NOT config STREQUAL "Retail")
                string(TOUPPER ${config} config)
                set_target_properties(SquareShaderPack PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_${config} TRUE)
            endif()
        endforeach()
    endif()
    # installed with the resources (the root of the installation: the resource folder of the game)
    install(FILES ${SP_PACK} DESTINATION . OPTIONAL)
endfunction()

# the default pack (the engine only): at the end of the top project, if it made none
function(square_shader_pack_default)
    get_property(made GLOBAL PROPERTY SQUARE_SHADER_PACK_MADE)
    if(NOT made)
        square_shader_pack(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif()
endfunction()
if(CMAKE_VERSION VERSION_LESS 3.19)
    message(STATUS "ShaderPack: no default shader pack (CMake 3.19+); square_shader_pack() makes one")
else()
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL square_shader_pack_default)
endif()
