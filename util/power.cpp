/*
                    *****  Power Class implementation  *****
    
    Derived from pico-examples/adc/read_vsys.c
*/

#include "power.h"
#include <hardware/gpio.h>
#include <hardware/adc.h>
#if CYW43_USES_VSYS_PIN
#include "pico/cyw43_arch.h"
#endif

#define PICO_POWER_SAMPLE_COUNT 3
#define PICO_FIRST_ADC_PIN 26

void Power::enable()
{
    //  Read voltage until stable
    float v0 = batteryVolts();
    for (int ii = 0; ii < 100; ii++)
    {
        float v1 = batteryVolts();
        if (v0 > 0.0)
        {
            float dpct = (v1 - v0) / v0;
            if (ii > 10 && dpct < 0.1)
            {
                break;
            }
        }
        v0 = v1;
    }
}

void Power::disable()
{
}

bool Power::onBattery()
{
#if CYW43_USES_VSYS_PIN
    return !cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#else
    gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
    return !gpio_get(PICO_VBUS_PIN);
#endif
}

float Power::batteryVolts()
{
#if CYW43_USES_VSYS_PIN
    cyw43_thread_enter();
    // Make sure cyw43 is awake
    cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#endif

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
 
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // read vsys
    uint32_t vsys = adc_fifo_get_blocking();
    uint32_t vlo = vsys;
    uint32_t vhi = vsys;
    for(int i = 1; i < PICO_POWER_SAMPLE_COUNT + 2; i++)
    {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
        if (val > vhi) vhi = val;
        if (val < vlo) vlo = val;
    }

    adc_run(false);
    adc_fifo_drain();
#if CYW43_USES_VSYS_PIN
    cyw43_thread_exit();
#endif

    //  Discard high and low readings and average remaining
    vsys -= vhi;
    vsys -= vlo;
    vsys /= PICO_POWER_SAMPLE_COUNT;

    // Generate voltage
    const float conversion_factor = 3.3f / (1 << 12);
    return vsys * 3 * conversion_factor;
}