if (CMAKE_HOST_WIN32)
    find_program(CMakeJS "cmake-js.cmd")
    find_program(NPM "npm.cmd")
else ()
    find_program(CMakeJS "cmake-js")
    find_program(NPM "npm")
endif()

if (NPM)
    message(STATUS "Found NPM")
else ()
    message(FATAL_ERROR "NPM not found. This project requires Node.js.")
endif ()

if (CMakeJS)
    message(STATUS "Found CMake.js")
else ()
    message(FATAL_ERROR "CMake.js not found. This project requires cmake-js installed globally.")
endif ()

function(GET_VARIABLE INPUT_STRING VARIABLE_TO_SELECT OUTPUT_VARIABLE)
    set(SEARCH_STRING "-D${VARIABLE_TO_SELECT}=")
    string(LENGTH ${SEARCH_STRING} SEARCH_STRING_LENGTH)
    string(LENGTH ${INPUT_STRING} INPUT_STRING_LENGTH)
    string(FIND ${INPUT_STRING} ${SEARCH_STRING} SEARCH_STRING_INDEX)
    if (${SEARCH_STRING_INDEX} LESS 0)
        set(${OUTPUT_VARIABLE} "" PARENT_SCOPE)
    else ()
        math(EXPR SEARCH_STRING_INDEX "${SEARCH_STRING_INDEX}+${SEARCH_STRING_LENGTH}")
        string(SUBSTRING ${INPUT_STRING} ${SEARCH_STRING_INDEX} ${INPUT_STRING_LENGTH} AFTER_SEARCH_STRING)
        string(FIND ${AFTER_SEARCH_STRING} "'" QUOTE_INDEX)
        string(SUBSTRING ${AFTER_SEARCH_STRING} 0 ${QUOTE_INDEX} RESULT_STRING)
        set(${OUTPUT_VARIABLE} ${RESULT_STRING} PARENT_SCOPE)
    endif ()
endfunction(GET_VARIABLE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Select build type" FORCE)
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)

if (CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
    execute_process(
        COMMAND ${CMakeJS} print-configure --debug
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE CMAKE_JS_OUTPUT
        RESULT_VARIABLE CMAKE_JS_RESULT
    )
else ()
    execute_process(
        COMMAND ${CMakeJS} print-configure
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE CMAKE_JS_OUTPUT
        RESULT_VARIABLE CMAKE_JS_RESULT
    )
endif ()

# check
if(NOT CMAKE_JS_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to execute cmake-js: ${CMAKE_JS_RESULT}")
endif()

get_variable(${CMAKE_JS_OUTPUT} "CMAKE_JS_INC" CMAKE_JS_INC)
get_variable(${CMAKE_JS_OUTPUT} "CMAKE_JS_LIB" CMAKE_JS_LIB)
get_variable(${CMAKE_JS_OUTPUT} "CMAKE_JS_SRC" CMAKE_JS_SRC)