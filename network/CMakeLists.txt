#               *****  PICO Utility Library  *****

add_library(bgr_network INTERFACE)

target_sources(bgr_network INTERFACE
    dhcpserver.c
    httprequest.cpp
    web.cpp
    web_set_time.c
    ws.cpp)

target_include_directories(bgr_network INTERFACE
   ${CMAKE_CURRENT_LIST_DIR})

#[[         *****  web_files  *****

\brief      Generate compiled filesystem

\details    Create the data portion for the WEB_FILES class

\param      WEBSOCKET   If present, the websocket.js fie is included
\param      DIR         Directory to receive generated file (default build/generated)
\param      OUTPUT      Name of generated file (default web_files.cpp)
\param      FILES       List of source files to be included in filesystem

]]
function(web_files)
    cmake_parse_arguments(PARSE_ARGV 0 "WF" "WEBSOCKET" "DIR;OUTPUT" "FILES")

    if("${WF_DIR}" STREQUAL "")
        set(WF_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
    endif()
    if ("${WF_OUTPUT}" STREQUAL "")
        set(WF_OUTPUT "web_files.cpp")
    endif()
    set(WEB_FILES_OUTPUT ${WF_DIR}/${WF_OUTPUT})
    if(${WF_WEBSOCKET})
        set(WS_JS ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/websocket.js)
    endif()

    set(WEB_FILES_TARGET "${PROJECT_NAME}_web_files")
    add_custom_target(${WEB_FILES_TARGET} DEPENDS ${WF_FILES} ${WS_JS})

    find_package (Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_command(
        OUTPUT ${WEB_FILES_OUTPUT}
        DEPENDS ${WF_FILES} ${WF_JS} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/create_web_files.py
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WF_DIR} &&
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/create_web_files.py -o ${WEB_FILES_OUTPUT} ${WF_FILES} ${WS_JS}
	    VERBATIM)

    add_dependencies(${PROJECT_NAME} ${WEB_FILES_TARGET})
    
endfunction()
