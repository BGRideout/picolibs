#                   *****  PICO IR Library  *****

add_library(bgr_ir INTERFACE)

target_sources(bgr_ir INTERFACE
    ir_led.cpp
    ir_receiver.cpp)

target_link_libraries(bgr_ir INTERFACE
    bgr_util
    hardware_gpio
    hardware_irq
    hardware_timer
    pico_time)

target_include_directories(bgr_ir INTERFACE
   ${CMAKE_CURRENT_LIST_DIR})


#  IR Protocols

add_library(bgr_ir_protocols INTERFACE)

target_sources(bgr_ir_protocols INTERFACE
    nec_receiver.cpp
    nec_transmitter.cpp)

target_link_libraries(bgr_ir_protocols INTERFACE
    bgr_ir)

target_include_directories(bgr_ir_protocols INTERFACE
   ${CMAKE_CURRENT_LIST_DIR})


