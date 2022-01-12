#pragma once

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Gpio.h"
#include "Utils.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// VARIABLES ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using                    GpioIsr = void (*) (void * v_arg);
extern SemaphoreHandle_t gpioSemaphore;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void IRAM_ATTR gpioIsr (void * v_arg)
{
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR (gpioSemaphore, &xHigherPriorityTaskWoken);
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class GpioHw : public Gpio <GpioHw>
{
    static constexpr char * const MODULE = (char *)"Gpio";

    public:
        GpioHw ()
        {
            gpioSemaphore = xSemaphoreCreateBinary ();

            gpio_install_isr_service (ESP_INTR_FLAG_LEVEL1);
            setPinConfig             (GPIO_NUM_26, gpioIsr, GPIO_INTR_POSEDGE, GPIO_MODE_INPUT , GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
            setPinConfig             (GPIO_NUM_27, gpioIsr, GPIO_INTR_DISABLE, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE);
        }
        ~GpioHw () = default;

        void SetPinLevel  (const uint16_t v_num, const bool v_state) { gpio_set_level ((gpio_num_t)v_num, v_state); }
        bool ReadPinLevel (const uint16_t v_num) { return (bool)gpio_get_level ((gpio_num_t)v_num); }

    private:
        void setPinConfig (const gpio_num_t v_num, GpioIsr v_gpioIsr, const gpio_int_type_t v_isrType, const gpio_mode_t v_mode, const gpio_pulldown_t v_pullDown, const gpio_pullup_t v_pullUp)
        {
            gpio_config_t config = {};
            config.intr_type     = v_isrType;
            config.mode          = v_mode;
            config.pin_bit_mask  = (1ULL << v_num);
            config.pull_down_en  = v_pullDown;
            config.pull_up_en    = v_pullUp;
            gpio_config          (&config);
            gpio_isr_handler_add (v_num, v_gpioIsr, (void *)v_num);
        }
};

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
