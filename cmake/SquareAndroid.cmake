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
# Configured with the NDK (its toolchain: -DCMAKE_TOOLCHAIN_FILE=<ndk>/build/cmake/android.toolchain.cmake
# -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DANDROID_STL=c++_shared), the libraries of
# the game are built by this build, then packed in its APK by the Gradle project of the application
# (in <build>/android/<target>, from engine/cmake/android: no native build of its own):
#   <target>_apk              the APK of the configuration (in the build of everything: ALL)
#   <target>_android_install  the APK on the device (adb)
#   <target>_android_push     the resources on the device (SQUARE_ANDROID_PUSH: more of them, the
#                             maps...), in the files of the application (its working folder)
#   <target>_android_run      the game started on the device
#   <target>_android_log      the log of the engine on the device (logcat, tag Square)
# The Android SDK is the one of the NDK (SQUARE_ANDROID_SDK), Gradle needs a JDK 17
# (SQUARE_ANDROID_JAVA_HOME: JAVA_HOME by default). On the desktop: nothing to do.

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
# the tools of the Android build
# the JDK of Gradle
function(square_android_find_java out)
    if(CMAKE_HOST_WIN32)
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

# the SDK of the NDK (<sdk>/ndk/<version>)
function(square_android_find_sdk out)
    get_filename_component(ndks "${ANDROID_NDK}" DIRECTORY)
    get_filename_component(sdk "${ndks}" DIRECTORY)
    if(EXISTS "${sdk}/platform-tools")
        set(${out} "${sdk}" PARENT_SCOPE)
        return()
    endif()
    set(${out} "$ENV{ANDROID_HOME}" PARENT_SCOPE)
endfunction()

if(ANDROID)
    square_android_find_java(SQUARE_ANDROID_JAVA_FOUND)
    square_android_find_sdk(SQUARE_ANDROID_SDK_FOUND)
    set(SQUARE_ANDROID_JAVA_HOME "${SQUARE_ANDROID_JAVA_FOUND}" CACHE PATH "The JDK 17 of Gradle (the APK of the games)")
    set(SQUARE_ANDROID_SDK "${SQUARE_ANDROID_SDK_FOUND}" CACHE PATH "The Android SDK (the APK of the games)")
    set(SQUARE_ANDROID_COMPILE_SDK "35" CACHE STRING "The Android platform of the APK (compile and target SDK)")
    set(SQUARE_ANDROID_BUILD_TOOLS "35.0.0" CACHE STRING "The build tools of the APK")
    set(SQUARE_ANDROID_GRADLE "" CACHE FILEPATH "A Gradle in place of the wrapper of the project (empty: the wrapper)")
    set(SQUARE_ANDROID_PUSH "" CACHE STRING "More resources of the game pushed to the device (a list, relative to the game: assets/valley.sqz)")
endif()

#################################################################################################
# the Gradle project of a game
# the values of the templates (in the scope of square_android_app)
macro(square_android_values target)
    get_property(module GLOBAL PROPERTY SQUARE_ANDROID_DIR)
    get_property(engine GLOBAL PROPERTY SQUARE_ANDROID_ENGINE_DIR)
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
    # the build type of Gradle: the configuration of the engine (debug, release, retail)
    set(SQUARE_ANDROID_CONFIG "${CMAKE_BUILD_TYPE}")
    if(NOT SQUARE_ANDROID_CONFIG)
        set(SQUARE_ANDROID_CONFIG Release)
    endif()
    string(TOLOWER "${SQUARE_ANDROID_CONFIG}" SQUARE_ANDROID_CONFIG_NAME)
    get_filename_component(SQUARE_ANDROID_NDK_VERSION "${ANDROID_NDK}" NAME)
    set(SQUARE_ANDROID_TARGET ${target})
    set(SQUARE_ANDROID_NAME "${APP_NAME}")
    set(SQUARE_ANDROID_PACKAGE "${APP_PACKAGE}")
    set(SQUARE_ANDROID_SPLASH_COLOR "${APP_SPLASH_COLOR}")
    set(SQUARE_ANDROID_ORIENTATION "${APP_ORIENTATION}")
    set(SQUARE_ANDROID_VERSION_NAME "${APP_VERSION_NAME}")
    set(SQUARE_ANDROID_VERSION_CODE "${APP_VERSION_CODE}")
    set(SQUARE_ANDROID_MIN_SDK "${ANDROID_PLATFORM_LEVEL}")
    set(SQUARE_ANDROID_ABI "${ANDROID_ABI}")
    # the APK profileable by adb (simpleperf, the profiler of Android Studio) with the profiler of
    # the engine: SQUARE_RENDER_PROFILER, never in Retail (the build that ships); the element of
    # the manifest, else none
    set(SQUARE_PERF OFF)
    if(SQUARE_RENDER_PROFILER AND NOT SQUARE_ANDROID_CONFIG STREQUAL "Retail")
        set(SQUARE_PERF ON)
    endif()
    set(SQUARE_PERF_MANIFEST "")
    if(SQUARE_PERF)
        set(SQUARE_PERF_MANIFEST "<profileable android:shell=\"true\" />")
    endif()
endmacro()

# the project: the static files, the templates, the icon, the SDK
function(square_android_project project template icon)
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
    configure_file(${icon} ${project}/app/src/main/res/drawable-nodpi/square_icon.png COPYONLY)
    file(WRITE ${project}/local.properties "sdk.dir=${SQUARE_ANDROID_SDK}\n")
endfunction()

# the C++ runtime of the libraries (ANDROID_STL c++_shared): in the APK with them
function(square_android_stl out)
    # (the triple of the architecture: the name of the toolchain without its compiler)
    string(REGEX REPLACE "-clang$" "" triple "${ANDROID_TOOLCHAIN_NAME}")
    set(stl "${ANDROID_TOOLCHAIN_ROOT}/sysroot/usr/lib/${triple}/libc++_shared.so")
    if(NOT EXISTS "${stl}")
        message(FATAL_ERROR "square_android_app: no ${stl} (ANDROID_STL=c++_shared)")
    endif()
    set(${out} "${stl}" PARENT_SCOPE)
endfunction()

# the APK: the libraries of the game in its folder of libraries (jniLibs), then Gradle
function(square_android_apk target project module)
    if(SQUARE_ANDROID_GRADLE)
        set(gradle "${SQUARE_ANDROID_GRADLE}")
    elseif(CMAKE_HOST_WIN32)
        set(gradle ${project}/gradlew.bat)
    else()
        set(gradle ${project}/gradlew)
    endif()
    square_android_stl(stl)
    # (the list with '|': a ';' would split the argument of the command)
    set(libraries
        $<TARGET_FILE:${target}>
        $<TARGET_FILE:Square>
        $<TARGET_FILE:HLSL2ALL>
        $<TARGET_FILE:SqVulkan>
        ${stl})
    string(REPLACE ";" "|" libraries "${libraries}")
    add_custom_target(${target}_apk ALL
        COMMAND ${CMAKE_COMMAND}
            "-DLIBRARIES=${libraries}"
            -DOUTPUT=${project}/app/jniLibs/${SQUARE_ANDROID_ABI}
            -P ${module}/SquareAndroidPackage.cmake
        # (no daemon: a daemon of Gradle keeps the output of the build open, the build never ends)
        COMMAND ${CMAKE_COMMAND} -E env
            "JAVA_HOME=${SQUARE_ANDROID_JAVA_HOME}"
            "ANDROID_HOME=${SQUARE_ANDROID_SDK}"
            ${gradle} --no-daemon --console=plain -p ${project} assemble${SQUARE_ANDROID_CONFIG}
        WORKING_DIRECTORY ${project}
        COMMENT "Android: the APK of ${target} (${SQUARE_ANDROID_CONFIG})"
        USES_TERMINAL
        VERBATIM)
    add_dependencies(${target}_apk ${target} Square HLSL2ALL SqVulkan)
endfunction()

# the device: the APK, the resources, the game started, its log
function(square_android_device target project module resources)
    if(CMAKE_HOST_WIN32)
        set(adb ${SQUARE_ANDROID_SDK}/platform-tools/adb.exe)
    else()
        set(adb ${SQUARE_ANDROID_SDK}/platform-tools/adb)
    endif()
    set(apk ${project}/app/build/outputs/apk/${SQUARE_ANDROID_CONFIG_NAME}/app-${SQUARE_ANDROID_CONFIG_NAME}.apk)
    add_custom_target(${target}_android_install
        COMMAND ${adb} install -r ${apk}
        COMMENT "Android: ${target} (${SQUARE_ANDROID_CONFIG}) on the device"
        USES_TERMINAL
        VERBATIM)
    # (the list with '|': a ';' would split the argument of the command)
    string(REPLACE ";" "|" resources "${resources}")
    add_custom_target(${target}_android_push
        COMMAND ${CMAKE_COMMAND}
            -DADB=${adb}
            -DPACKAGE=${SQUARE_ANDROID_PACKAGE}
            -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
            "-DRESOURCES=${resources}"
            -P ${module}/SquareAndroidPush.cmake
        COMMENT "Android: the resources of ${target} on the device"
        USES_TERMINAL
        VERBATIM)
    add_custom_target(${target}_android_run
        COMMAND ${adb} shell am start -n ${SQUARE_ANDROID_PACKAGE}/android.app.NativeActivity
        COMMENT "Android: ${target} started on the device"
        VERBATIM)
    add_custom_target(${target}_android_log
        COMMAND ${adb} logcat -s Square
        COMMENT "Android: the log of ${target} (logcat, tag Square)"
        USES_TERMINAL
        VERBATIM)
endfunction()

function(square_android_app target)
    if(NOT ANDROID)
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
        message(STATUS "Android: no SDK or JDK 17 (SQUARE_ANDROID_SDK, SQUARE_ANDROID_JAVA_HOME): no APK of ${target}")
        return()
    endif()
    square_android_values(${target})
    set(project ${CMAKE_BINARY_DIR}/android/${target})
    square_android_project(${project} ${module}/android ${APP_ICON})
    square_android_apk(${target} ${project} ${module})
    square_android_device(${target} ${project} ${module} "${APP_RESOURCES};${SQUARE_ANDROID_PUSH}")
    message(STATUS "Android: ${target} (${APP_PACKAGE}, ${SQUARE_ANDROID_CONFIG}) in ${project}")
endfunction()
