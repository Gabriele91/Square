# The games of Square on Android, a module of the engine (engine/cmake, in CMAKE_MODULE_PATH:
# include(SquareAndroid), done by the engine for every game).
#
#   square_add_game(<target> <sources>...)
#
# The executable of a game: on Android the library of its NativeActivity (lib<target>.so, its
# android_main: SquareMain.h), with the link time optimization of the optimized configurations.
#
#   square_android_app(<target>
#       NAME         <name>               # the name of the application ("Rush")
#       PACKAGE      <package>            # its package (com.square.engine: no '-')
#       [ICON        <png>]               # its icon and the one of its splash (default: Square's)
#       [SPLASH_COLOR <#rrggbb>]          # the background of its splash (default: black)
#       [ORIENTATION <orientation>]       # of its activity (default: sensorLandscape)
#       [VERSION_NAME <name>] [VERSION_CODE <code>]
#       [RESOURCES   <path>...]           # its resources (files, folders, relative to its folder)
#   )
#
# Configured on the desktop (not with the NDK), it makes the Gradle project of the application in
# <build>/android/<target> (from engine/cmake/android) and its targets:
#   <target>_android          the APK of the configuration (Gradle: CMake with the NDK on the game)
#   <target>_android_install  the APK on the device (adb)
#   <target>_android_push     the resources on the device (SQUARE_ANDROID_PUSH: more of them, the
#                             maps...), in the files of the application (its working folder)
#   <target>_android_run      the game started on the device
# It needs the Android SDK (SQUARE_ANDROID_SDK: ANDROID_HOME by default) with its NDK and CMake
# (SQUARE_ANDROID_NDK_VERSION, SQUARE_ANDROID_CMAKE_VERSION), a JDK 17 (SQUARE_ANDROID_JAVA_HOME:
# JAVA_HOME by default); without them, no target. Built with the NDK: nothing to do.

# (its folder and the one of the engine: for the functions called by the game, out of this scope)
set_property(GLOBAL PROPERTY SQUARE_ANDROID_DIR "${CMAKE_CURRENT_LIST_DIR}")
set_property(GLOBAL PROPERTY SQUARE_ANDROID_ENGINE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

#################################################################################################
# the game
function(square_add_game target)
    if(ANDROID)
        add_library(${target} SHARED ${ARGN})
        set_target_properties(${target} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
            INTERPROCEDURAL_OPTIMIZATION_RETAIL ON
        )
    else()
        add_executable(${target} ${ARGN})
    endif()
endfunction()

#################################################################################################
# the tools of the host
function(square_android_find_sdk out)
    set(candidates "$ENV{ANDROID_HOME}" "$ENV{ANDROID_SDK_ROOT}")
    if(WIN32)
        list(APPEND candidates "$ENV{LOCALAPPDATA}/Android/Sdk")
    elseif(APPLE)
        list(APPEND candidates "$ENV{HOME}/Library/Android/sdk")
    else()
        list(APPEND candidates "$ENV{HOME}/Android/Sdk")
    endif()
    foreach(candidate ${candidates})
        if(candidate AND EXISTS "${candidate}/platform-tools")
            file(TO_CMAKE_PATH "${candidate}" candidate)
            set(${out} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out} "" PARENT_SCOPE)
endfunction()

function(square_android_find_java out)
    if(WIN32)
        set(java bin/java.exe)
        file(GLOB candidates
            "C:/Program Files/Java/jdk-17*"
            "C:/Program Files/Eclipse Adoptium/jdk-17*"
            "C:/Program Files/Android/Android Studio/jbr"
        )
    else()
        set(java bin/java)
        file(GLOB candidates
            "/usr/lib/jvm/java-17*"
            "/Library/Java/JavaVirtualMachines/*17*/Contents/Home"
            "/Applications/Android Studio.app/Contents/jbr/Contents/Home"
        )
    endif()
    foreach(candidate "$ENV{JAVA_HOME}" ${candidates})
        if(candidate AND EXISTS "${candidate}/${java}")
            file(TO_CMAKE_PATH "${candidate}" candidate)
            set(${out} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out} "" PARENT_SCOPE)
endfunction()

if(NOT ANDROID)
    square_android_find_sdk(SQUARE_ANDROID_SDK_FOUND)
    square_android_find_java(SQUARE_ANDROID_JAVA_FOUND)
    set(SQUARE_ANDROID_SDK "${SQUARE_ANDROID_SDK_FOUND}" CACHE PATH "The Android SDK (the Android builds of the games)")
    set(SQUARE_ANDROID_JAVA_HOME "${SQUARE_ANDROID_JAVA_FOUND}" CACHE PATH "The JDK 17 of Gradle (the Android builds of the games)")
    set(SQUARE_ANDROID_NDK_VERSION "28.2.13676358" CACHE STRING "The NDK of the Android builds")
    set(SQUARE_ANDROID_CMAKE_VERSION "3.31.6" CACHE STRING "The CMake of the SDK of the Android builds")
    set(SQUARE_ANDROID_COMPILE_SDK "35" CACHE STRING "The Android platform of the builds (compile and target SDK)")
    set(SQUARE_ANDROID_BUILD_TOOLS "35.0.0" CACHE STRING "The build tools of the Android builds")
    set(SQUARE_ANDROID_MIN_SDK "29" CACHE STRING "The oldest Android of the games (29: Android 10, Vulkan 1.1)")
    set(SQUARE_ANDROID_ABIS "arm64-v8a" CACHE STRING "The ABIs of the Android builds (a list)")
    set(SQUARE_ANDROID_GRADLE "" CACHE FILEPATH "A Gradle in place of the wrapper of the project (empty: the wrapper)")
    set(SQUARE_ANDROID_PUSH "" CACHE STRING "More resources of the game pushed to the device (a list, relative to the game: assets/valley.sqz)")
endif()

#################################################################################################
# the Gradle project of a game
function(square_android_app target)
    if(ANDROID)
        return()
    endif()
    cmake_parse_arguments(APP "" "NAME;PACKAGE;ICON;SPLASH_COLOR;ORIENTATION;VERSION_NAME;VERSION_CODE" "RESOURCES" ${ARGN})
    if(NOT APP_NAME OR NOT APP_PACKAGE)
        message(FATAL_ERROR "square_android_app(${target}): NAME and PACKAGE are needed")
    endif()
    if(APP_PACKAGE MATCHES "-")
        message(FATAL_ERROR "square_android_app(${target}): an Android package has no '-' (${APP_PACKAGE})")
    endif()
    if(NOT SQUARE_ANDROID_SDK OR NOT SQUARE_ANDROID_JAVA_HOME)
        message(STATUS "Android: no SDK or JDK 17 (SQUARE_ANDROID_SDK, SQUARE_ANDROID_JAVA_HOME): no Android targets for ${target}")
        return()
    endif()
    get_property(module GLOBAL PROPERTY SQUARE_ANDROID_DIR)
    get_property(engine GLOBAL PROPERTY SQUARE_ANDROID_ENGINE_DIR)
    # defaults
    if(NOT APP_ICON)
        set(APP_ICON ${engine}/common/texture/square_icon.png)
    endif()
    get_filename_component(APP_ICON "${APP_ICON}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    if(NOT APP_SPLASH_COLOR)
        set(APP_SPLASH_COLOR "#000000")
    endif()
    if(NOT APP_ORIENTATION)
        set(APP_ORIENTATION sensorLandscape)
    endif()
    if(NOT APP_VERSION_NAME)
        set(APP_VERSION_NAME "1.0")
    endif()
    if(NOT APP_VERSION_CODE)
        set(APP_VERSION_CODE 1)
    endif()
    # the values of the templates
    set(SQUARE_ANDROID_TARGET ${target})
    set(SQUARE_ANDROID_NAME "${APP_NAME}")
    set(SQUARE_ANDROID_PACKAGE "${APP_PACKAGE}")
    set(SQUARE_ANDROID_SPLASH_COLOR "${APP_SPLASH_COLOR}")
    set(SQUARE_ANDROID_ORIENTATION "${APP_ORIENTATION}")
    set(SQUARE_ANDROID_VERSION_NAME "${APP_VERSION_NAME}")
    set(SQUARE_ANDROID_VERSION_CODE "${APP_VERSION_CODE}")
    set(SQUARE_ANDROID_CMAKE_LISTS "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
    set(SQUARE_ANDROID_TARGETS "\"${target}\", \"Square\", \"HLSL2ALL\", \"SqVulkan\"")
    string(REPLACE ";" "\", \"" SQUARE_ANDROID_ABIS "\"${SQUARE_ANDROID_ABIS}\"")
    # the project: the static files, the templates, the icon, the SDK
    set(template ${module}/android)
    set(project ${CMAKE_BINARY_DIR}/android/${target})
    file(COPY
        ${template}/gradlew
        ${template}/gradlew.bat
        ${template}/gradle.properties
        ${template}/build.gradle.kts
        ${template}/gradle
        DESTINATION ${project})
    foreach(file
            settings.gradle.kts
            app/build.gradle.kts
            app/src/main/AndroidManifest.xml
            app/src/main/res/values/strings.xml
            app/src/main/res/values/styles.xml
            app/src/main/res/values-v31/styles.xml)
        configure_file(${template}/${file}.in ${project}/${file} @ONLY)
    endforeach()
    configure_file(${APP_ICON} ${project}/app/src/main/res/drawable-nodpi/square_icon.png COPYONLY)
    file(WRITE ${project}/local.properties "sdk.dir=${SQUARE_ANDROID_SDK}\n")
    # the tools: Gradle (its wrapper, or the one given) with its JDK, adb
    if(SQUARE_ANDROID_GRADLE)
        set(gradle "${SQUARE_ANDROID_GRADLE}")
    elseif(WIN32)
        set(gradle ${project}/gradlew.bat)
    else()
        set(gradle ${project}/gradlew)
    endif()
    set(run_gradle ${CMAKE_COMMAND} -E env "JAVA_HOME=${SQUARE_ANDROID_JAVA_HOME}" "ANDROID_HOME=${SQUARE_ANDROID_SDK}" ${gradle} --console=plain -p ${project})
    if(WIN32)
        set(adb ${SQUARE_ANDROID_SDK}/platform-tools/adb.exe)
    else()
        set(adb ${SQUARE_ANDROID_SDK}/platform-tools/adb)
    endif()
    # the targets: the APK of the configuration (assembleRelease...), on the device, its resources,
    # the game started
    add_custom_target(${target}_android
        COMMAND ${run_gradle} assemble$<CONFIG>
        WORKING_DIRECTORY ${project}
        COMMENT "Android: the APK of ${target} ($<CONFIG>)"
        USES_TERMINAL
        VERBATIM)
    add_custom_target(${target}_android_install
        COMMAND ${run_gradle} install$<CONFIG>
        WORKING_DIRECTORY ${project}
        COMMENT "Android: ${target} ($<CONFIG>) on the device"
        USES_TERMINAL
        VERBATIM)
    # (the list with '|': a ';' would split the argument of the command)
    set(resources ${APP_RESOURCES} ${SQUARE_ANDROID_PUSH})
    string(REPLACE ";" "|" resources "${resources}")
    add_custom_target(${target}_android_push
        COMMAND ${CMAKE_COMMAND}
            -DADB=${adb}
            -DPACKAGE=${APP_PACKAGE}
            -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
            "-DRESOURCES=${resources}"
            -P ${module}/SquareAndroidPush.cmake
        COMMENT "Android: the resources of ${target} on the device"
        USES_TERMINAL
        VERBATIM)
    add_custom_target(${target}_android_run
        COMMAND ${adb} shell am start -n ${APP_PACKAGE}/android.app.NativeActivity
        COMMENT "Android: ${target} started on the device"
        VERBATIM)
    message(STATUS "Android: ${target} (${APP_PACKAGE}) in ${project}: targets ${target}_android, ${target}_android_install, ${target}_android_push, ${target}_android_run")
endfunction()
