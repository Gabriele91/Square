# The libraries of a game in the folder of the libraries of its APK (cmake -P, the target
# <game>_apk of square_android_app): the ones of the build (the game, the engine, its driver) and
# the C++ runtime, as they are now (the folder made again: no library of another configuration).
#
#   cmake -DLIBRARIES=<files; separated by ';' or '|'> -DOUTPUT=<app/jniLibs/<abi>>
#         -P SquareAndroidPackage.cmake
cmake_minimum_required(VERSION 3.14)

foreach(variable LIBRARIES OUTPUT)
    if(NOT ${variable})
        message(FATAL_ERROR "SquareAndroidPackage: ${variable} is needed")
    endif()
endforeach()

string(REPLACE "|" ";" LIBRARIES "${LIBRARIES}")
file(REMOVE_RECURSE "${OUTPUT}")
file(MAKE_DIRECTORY "${OUTPUT}")
foreach(library ${LIBRARIES})
    if(NOT EXISTS "${library}")
        message(FATAL_ERROR "SquareAndroidPackage: no ${library}")
    endif()
    file(COPY "${library}" DESTINATION "${OUTPUT}")
endforeach()
