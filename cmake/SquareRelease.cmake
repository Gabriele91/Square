# The release of a game for its platforms (cmake -P, from the folder of the game or with GAME):
# each platform built in its configurations (Release, Retail), installed, its archives made for
# its GPUs (tools/AssetPack: desktop BC in DDS, mobile ASTC in KTX, the same images in one file, a
# report), its shaders packed for its targets (tools/ShaderPack: shaders.sqz, nothing translated
# when the game runs):
#
#   cmake -P engine/cmake/SquareRelease.cmake
#         [-DGAME=<folder of the game: .>] [-DOUTPUT=<GAME>/release] [-DBUILD=<GAME>/build-release]
#         [-DPLATFORMS=desktop;android] [-DCONFIGS=Release;Retail]
#         [-DARCHIVES=<the archives made: arena.sqz;title.sqz...; default: all of assets/>]
#         [-DEXCLUDE=<the files of assets/ out of the release: its tests; default: the ones of
#                     <GAME>/release.cmake, set(EXCLUDE wildwater.sqz config/maps/test.json)>]
#         [-DGENERATOR=<desktop: Visual Studio 17 2022 on Windows, else Ninja Multi-Config>]
#
# The release (OUTPUT):
#   desktop/<config>/          the game installed (cmake --install), its archives and its shaders
#   android/<config>/<game>.apk  the APK; files/: the resources of the game (the archives of the
#                              mobiles, shaders.sqz) to push in the files of the application
#                              (SquareAndroidPush.cmake, ROOT=files)
#   assetpack_<platform>.txt   the report of the archives of a platform
# The tools (AssetPack, ShaderPack) are the ones of the desktop build (built first, Release).
cmake_minimum_required(VERSION 3.21)

#################################################################################################
# the settings
if(NOT GAME)
    set(GAME "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
get_filename_component(GAME "${GAME}" ABSOLUTE)
if(NOT OUTPUT)
    set(OUTPUT "${GAME}/release")
endif()
if(NOT BUILD)
    set(BUILD "${GAME}/build-release")
endif()
if(NOT PLATFORMS)
    set(PLATFORMS desktop android)
endif()
if(NOT CONFIGS)
    set(CONFIGS Release Retail)
endif()
# the files of the game out of its release (its tests), by the game (release.cmake), if not asked
if(NOT DEFINED EXCLUDE AND EXISTS "${GAME}/release.cmake")
    include("${GAME}/release.cmake")
endif()
if(NOT GENERATOR)
    if(CMAKE_HOST_WIN32)
        set(GENERATOR "Visual Studio 17 2022")
    else()
        set(GENERATOR "Ninja Multi-Config")
    endif()
endif()

# the platform of the host (its archives, its shaders: the names of AssetPack and ShaderPack)
if(CMAKE_HOST_WIN32)
    set(HOST_PLATFORM windows)
elseif(CMAKE_HOST_APPLE)
    set(HOST_PLATFORM macos)
else()
    set(HOST_PLATFORM linux)
endif()

# the platform of the files of a platform of the build (desktop: the host)
function(release_files_platform platform out)
    if(platform STREQUAL "desktop")
        set(${out} ${HOST_PLATFORM} PARENT_SCOPE)
    else()
        set(${out} ${platform} PARENT_SCOPE)
    endif()
endfunction()

#################################################################################################
# the steps
# a command, stopped at its first error
function(release_run)
    string(REPLACE ";" " " command "${ARGN}")
    message(STATUS "release: ${command}")
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${GAME}" RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "release: ${command} (${result})")
    endif()
endfunction()

# Ninja for the builds of one configuration (in the path, else the one of the Android SDK)
function(release_ninja out)
    find_program(ninja NAMES ninja)
    if(NOT ninja)
        file(GLOB ninjas "$ENV{LOCALAPPDATA}/Android/Sdk/cmake/*/bin/ninja*" "$ENV{ANDROID_HOME}/cmake/*/bin/ninja*")
        list(GET ninjas 0 ninja)
    endif()
    set(${out} "${ninja}" PARENT_SCOPE)
endfunction()

# the desktop: one build of every configuration (a multi configuration generator)
function(release_build_desktop)
    set(build "${BUILD}/desktop")
    release_run(${CMAKE_COMMAND} -S "${GAME}" -B "${build}" -G "${GENERATOR}" -DSQUARE_PLATFORM=desktop)
    # (Release: the tools, AssetPack and ShaderPack, whatever the configurations of the release;
    # the other ones only for a release of the desktop)
    set(configs Release)
    if("desktop" IN_LIST PLATFORMS)
        list(APPEND configs ${CONFIGS})
    endif()
    list(REMOVE_DUPLICATES configs)
    foreach(config ${configs})
        release_run(${CMAKE_COMMAND} --build "${build}" --config ${config} --parallel)
    endforeach()
endfunction()

# Android: a build each configuration (Ninja), its APK built with it
function(release_build_android config)
    string(TOLOWER ${config} name)
    set(build "${BUILD}/android-${name}")
    release_ninja(ninja)
    release_run(${CMAKE_COMMAND} -S "${GAME}" -B "${build}" -G Ninja "-DCMAKE_MAKE_PROGRAM=${ninja}"
                -DSQUARE_PLATFORM=android -DCMAKE_BUILD_TYPE=${config})
    release_run(${CMAKE_COMMAND} --build "${build}" --parallel)
endfunction()

# a tool of the desktop build (Release)
function(release_tool name out)
    file(GLOB tools "${BUILD}/desktop/bin/Release/${name}*" "${BUILD}/desktop/bin/${name}*")
    list(FILTER tools INCLUDE REGEX "/${name}(\\.exe)?$")
    if(NOT tools)
        message(FATAL_ERROR "release: no ${name} in ${BUILD}/desktop/bin")
    endif()
    list(GET tools 0 tool)
    set(${out} "${tool}" PARENT_SCOPE)
endfunction()

# the archives of a platform (once for all its configurations): <BUILD>/assets-<platform>
function(release_archives platform)
    release_files_platform(${platform} files_platform)
    release_tool(AssetPack asset_pack)
    # the archives asked, else the ones of assets/ but the excluded ones
    set(archives_asked ${ARCHIVES})
    if(NOT archives_asked)
        file(GLOB archives_asked RELATIVE "${GAME}/assets" "${GAME}/assets/*.sqz")
    endif()
    set(input "")
    foreach(archive ${archives_asked})
        if(NOT archive IN_LIST EXCLUDE)
            list(APPEND input "${GAME}/assets/${archive}")
        endif()
    endforeach()
    string(REPLACE ";" "," input "${input}")
    set(archives "${BUILD}/assets-${platform}")
    file(REMOVE_RECURSE "${archives}")
    # (its cache: the images of the releases before, not compressed again)
    release_run("${asset_pack}" -t ${files_platform} -i "${input}" -o "${archives}"
                -c "${BUILD}/assetcache-${platform}"
                -r "${OUTPUT}/assetpack_${platform}.txt")
endfunction()

# the shaders of a platform, for all its targets: <BUILD>/shaders-<platform>.sqz
function(release_shaders platform)
    release_files_platform(${platform} files_platform)
    release_tool(ShaderPack shader_pack)
    release_run("${shader_pack}" -t ${files_platform} -r resources.rs -o "${BUILD}/shaderpack-${platform}"
                -p "${BUILD}/shaders-${platform}.sqz")
endfunction()

# the archives and the shaders of a platform in a folder of resources (the installed game), the
# excluded files out of it
function(release_resources platform folder)
    file(GLOB archives "${BUILD}/assets-${platform}/*.sqz")
    file(COPY ${archives} DESTINATION "${folder}/assets")
    file(COPY_FILE "${BUILD}/shaders-${platform}.sqz" "${folder}/shaders.sqz")
    foreach(excluded ${EXCLUDE})
        file(REMOVE_RECURSE "${folder}/assets/${excluded}")
    endforeach()
endfunction()

# the components of the installation of a build but the one of no component (its libraries, its
# executables: the files of the game only, for a device)
function(release_components build out)
    file(GLOB_RECURSE scripts "${build}/cmake_install.cmake")
    set(components "")
    foreach(script ${scripts})
        file(STRINGS "${script}" lines REGEX "CMAKE_INSTALL_COMPONENT STREQUAL \"[^\"]+\"")
        foreach(line ${lines})
            string(REGEX MATCH "STREQUAL \"([^\"]+)\"" match "${line}")
            list(APPEND components "${CMAKE_MATCH_1}")
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES components)
    list(REMOVE_ITEM components "Unspecified")
    set(${out} "${components}" PARENT_SCOPE)
endfunction()

# the desktop of a configuration: installed, its archives, its shaders
function(release_desktop config)
    string(TOLOWER ${config} name)
    set(folder "${OUTPUT}/desktop/${name}")
    file(REMOVE_RECURSE "${folder}")
    release_run(${CMAKE_COMMAND} --install "${BUILD}/desktop" --config ${config} --prefix "${folder}")
    release_resources(desktop "${folder}")
endfunction()

# Android of a configuration: its APK, the resources of the game (its components) with the
# archives and the shaders of the mobiles
function(release_android config)
    string(TOLOWER ${config} name)
    set(build "${BUILD}/android-${name}")
    set(folder "${OUTPUT}/android/${name}")
    file(REMOVE_RECURSE "${folder}")
    file(MAKE_DIRECTORY "${folder}")
    file(GLOB apks "${build}/android/*/app/build/outputs/apk/${name}/*.apk")
    if(NOT apks)
        message(FATAL_ERROR "release: no APK in ${build}/android")
    endif()
    list(GET apks 0 apk)
    file(GLOB projects LIST_DIRECTORIES true "${build}/android/*")
    list(GET projects 0 project)
    get_filename_component(game "${project}" NAME)
    file(COPY_FILE "${apk}" "${folder}/${game}.apk")
    release_components("${build}" components)
    foreach(component ${components})
        release_run(${CMAKE_COMMAND} --install "${build}" --component ${component} --prefix "${folder}/files")
    endforeach()
    release_resources(android "${folder}/files")
endfunction()

#################################################################################################
# the release
file(MAKE_DIRECTORY "${OUTPUT}")
# the desktop first: its tools (AssetPack, ShaderPack), its game
release_build_desktop()
foreach(platform ${PLATFORMS})
    release_archives(${platform})
    release_shaders(${platform})
    foreach(config ${CONFIGS})
        if(platform STREQUAL "desktop")
            release_desktop(${config})
        elseif(platform STREQUAL "android")
            release_build_android(${config})
            release_android(${config})
        else()
            message(FATAL_ERROR "release: unknown platform ${platform} (desktop, android)")
        endif()
    endforeach()
endforeach()
message(STATUS "release: ${OUTPUT}")
