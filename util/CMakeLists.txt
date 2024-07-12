#               *****  PICO Utility Library  *****

add_library(bgr_util INTERFACE)

target_sources(bgr_util INTERFACE
    button.cpp
    dbgflag.cpp
    led.cpp
    power.cpp
    pwm.cpp
    servo.cpp)

target_link_libraries(bgr_util INTERFACE
    hardware_adc
    hardware_clocks
    hardware_gpio
    hardware_pwm)

target_include_directories(bgr_util INTERFACE
   ${CMAKE_CURRENT_LIST_DIR})
