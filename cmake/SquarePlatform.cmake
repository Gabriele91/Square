# The platform of the build of a game, a module of the engine: included by the game before its
# project() (the toolchain is chosen before the compilers are):
#
#   include(<engine>/cmake/SquarePlatform.cmake)
#   project(<game> ...)
#
# SQUARE_PLATFORM desktop (the one of the host, the default) or android: the NDK of the Android SDK
# (ANDROID_HOME, else the folder of Android Studio), its toolchain for arm64-v8a, Android 10
# (Vulkan 1.1), one C++ runtime shared by the libraries (c++_shared). A toolchain given
# (CMAKE_TOOLCHAIN_FILE) is kept.

set(SQUARE_PLATFORM "desktop" CACHE STRING "The platform of the build: desktop, android")
set_property(CACHE SQUARE_PLATFORM PROPERTY STRINGS desktop android)

# the Android SDK of the host (its platform tools: adb): the variables, then where it is installed
# by default (Android Studio, the packages of the system)
function(square_platform_android_sdk out)
    set(candidates "$ENV{ANDROID_HOME}" "$ENV{ANDROID_SDK_ROOT}")
    if(CMAKE_HOST_WIN32)
        list(APPEND candidates
            "$ENV{LOCALAPPDATA}/Android/Sdk"
            "$ENV{APPDATA}/Android/Sdk"
            "$ENV{USERPROFILE}/Android/Sdk"
            "C:/Android/Sdk"
            "C:/Program Files (x86)/Android/android-sdk")
    elseif(CMAKE_HOST_APPLE)
        list(APPEND candidates
            "$ENV{HOME}/Library/Android/sdk"
            "/opt/homebrew/share/android-commandlinetools"
            "/usr/local/share/android-commandlinetools")
    else()
        list(APPEND candidates
            "$ENV{HOME}/Android/Sdk"
            "$ENV{HOME}/Android/sdk"
            "$ENV{HOME}/android-sdk"
            "/opt/android-sdk"
            "/usr/lib/android-sdk")
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

# the NDK of the SDK: the version asked, else the newest one
function(square_platform_android_ndk sdk version out)
    if(EXISTS "${sdk}/ndk/${version}/build/cmake/android.toolchain.cmake")
        set(${out} "${sdk}/ndk/${version}" PARENT_SCOPE)
        return()
    endif()
    file(GLOB ndks LIST_DIRECTORIES true "${sdk}/ndk/*")
    list(SORT ndks COMPARE NATURAL ORDER DESCENDING)
    foreach(ndk ${ndks})
        if(EXISTS "${ndk}/build/cmake/android.toolchain.cmake")
            set(${out} "${ndk}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out} "" PARENT_SCOPE)
endfunction()

if(SQUARE_PLATFORM STREQUAL "android" AND NOT CMAKE_TOOLCHAIN_FILE)
    set(SQUARE_ANDROID_NDK_VERSION "28.2.13676358" CACHE STRING "The NDK of the Android builds (else the newest one)")
    square_platform_android_sdk(square_sdk)
    if(NOT square_sdk)
        message(FATAL_ERROR "SquarePlatform: no Android SDK (ANDROID_HOME)")
    endif()
    square_platform_android_ndk("${square_sdk}" "${SQUARE_ANDROID_NDK_VERSION}" square_ndk)
    if(NOT square_ndk)
        message(FATAL_ERROR "SquarePlatform: no NDK in ${square_sdk}/ndk")
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${square_ndk}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "The toolchain (the NDK)" FORCE)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "The ABI of the Android build")
    set(ANDROID_PLATFORM "android-29" CACHE STRING "The oldest Android of the games (29: Android 10, Vulkan 1.1)")
    set(ANDROID_STL "c++_shared" CACHE STRING "One C++ runtime shared by the game, the engine, its drivers")
    message(STATUS "SquarePlatform: Android, the NDK ${square_ndk}")
endif()
