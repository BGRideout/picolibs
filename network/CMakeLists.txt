#               *****  PICO Webserver Library  *****

cmake_minimum_required(VERSION 3.13)

if (DEFINED ENV{USE-HTTPS})
    set (USE-HTTPS ${USE-HTTPS})
else()
    set (USE-HTTPS "true")
endif()

add_library(bgr_webserver INTERFACE)

if (${USE-HTTPS} STREQUAL "true")
	message("Building https version")
	target_compile_definitions(bgr_webserver INTERFACE USE_HTTPS)
else()
	message("Building http version")
endif()

target_sources(bgr_webserver INTERFACE
    dhcpserver.c
    httprequest.cpp
    web.cpp
    web_set_time.c
    ws.cpp)

target_link_libraries(bgr_webserver INTERFACE
    bgr_util
    pico_mbedtls
    pico_lwip_mdns
    pico_lwip_sntp)

if (${USE-HTTPS} STREQUAL "true")
    target_link_libraries(bgr_webserver INTERFACE pico_lwip_mbedtls)
endif()
    
target_include_directories(bgr_webserver INTERFACE
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

    add_library(bgr_web_files INTERFACE)

    target_sources(bgr_web_files INTERFACE  ${WEB_FILES_OUTPUT})

    target_link_libraries(bgr_webserver INTERFACE bgr_web_files)

endfunction()

