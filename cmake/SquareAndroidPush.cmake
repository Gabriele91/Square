# The resources of a game on an Android device (cmake -P, the target <game>_android_push of
# square_android_app): not in the APK, in the files of the application (its working folder,
# /sdcard/Android/data/<package>/files), as in the folder of the game on the desktop.
#
#   cmake -DADB=<adb> -DPACKAGE=<package> -DROOT=<folder of the game>
#         -DRESOURCES=<paths, relative to ROOT: files, folders; separated by ';' or '|'>
#         [-DCOMMAND_LINE="-m arena"]          the command line of the game (args.txt; "" removes it)
#         [-DENVIRONMENT="RUSH_BENCH=bench.txt;A=1"]   its variables (env.txt; "" removes it)
#         -P SquareAndroidPush.cmake
#
# Only what changed is copied (adb push --sync); the folders made by adb (of its user, shell, 2770)
# are opened to the application.
cmake_minimum_required(VERSION 3.14)

foreach(variable ADB PACKAGE ROOT)
    if(NOT ${variable})
        message(FATAL_ERROR "SquareAndroidPush: ${variable} is needed")
    endif()
endforeach()
set(device "/sdcard/Android/data/${PACKAGE}/files")

function(adb)
    execute_process(COMMAND ${ADB} ${ARGN} RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE output)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "adb ${ARGN}: ${output}")
    endif()
endfunction()

# a file or a folder of the game in the same place on the device (its parent made first)
function(push path)
    set(local "${ROOT}/${path}")
    if(NOT EXISTS "${local}")
        message(WARNING "SquareAndroidPush: no ${local}")
        return()
    endif()
    get_filename_component(parent "${path}" DIRECTORY)
    set(target "${device}")
    if(parent)
        set(target "${device}/${parent}")
        adb(shell mkdir -p "${target}")
    endif()
    message(STATUS "push ${path}")
    adb(push --sync "${local}" "${target}")
    adb(shell chmod -R a+rwX "${device}/${path}")
endfunction()

# a file of the game made here (args.txt, env.txt): its text, or removed when empty
function(push_text name text)
    if(text STREQUAL "")
        adb(shell rm -f "${device}/${name}")
        return()
    endif()
    set(file "${CMAKE_CURRENT_BINARY_DIR}/square_android_${name}")
    file(WRITE "${file}" "${text}")
    adb(push "${file}" "${device}/${name}")
    file(REMOVE "${file}")
endfunction()

adb(shell mkdir -p "${device}")
string(REPLACE "|" ";" RESOURCES "${RESOURCES}")
foreach(path ${RESOURCES})
    if(path)
        push("${path}")
    endif()
endforeach()
if(DEFINED COMMAND_LINE)
    push_text(args.txt "${COMMAND_LINE}")
endif()
if(DEFINED ENVIRONMENT)
    string(REPLACE ";" "\n" lines "${ENVIRONMENT}")
    push_text(env.txt "${lines}")
endif()
message(STATUS "done: ${device}")
