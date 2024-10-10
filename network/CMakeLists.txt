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
