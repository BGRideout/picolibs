#               *****  PICO Utility Library  *****

add_library(bgr_network INTERFACE)

target_sources(bgr_network INTERFACE
    dhcpserver.c
    ws.cpp)

target_include_directories(bgr_network INTERFACE
   ${CMAKE_CURRENT_LIST_DIR})
