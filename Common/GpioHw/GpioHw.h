#pragma once

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Gpio.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static xQueueHandle queueHandle;

static void IRAM_ATTR isr (void * v_arg)
{
    uint32_t num = (uint32_t) v_arg;
    xQueueSendFromISR (queueHandle, &num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t pinNum;
    while (1)
    {
        if (xQueueReceive(queueHandle, &pinNum, portMAX_DELAY)) { printf ("GPIO [%d] intr, val: %d\n", pinNum, gpio_get_level ((gpio_num_t)pinNum)); }
    }
}

class GpioHw : public Gpio <GpioHw>
{
    static constexpr char * const MODULE = (char *)"Gpio";

    public:
        GpioHw ()
        {
            gpio_install_isr_service (ESP_INTR_FLAG_LEVEL1);
            setPinConfig             (GPIO_NUM_26, GPIO_INTR_POSEDGE, GPIO_MODE_INPUT , GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
            setPinConfig             (GPIO_NUM_27, GPIO_INTR_DISABLE, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE);

            queueHandle = xQueueCreate (10, sizeof (uint32_t));
            xTaskCreate (gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

        }
        ~GpioHw () = default;

        void SetPinLevel  (const uint16_t v_num, const bool v_state) { gpio_set_level ((gpio_num_t)v_num, v_state); }
        bool ReadPinLevel (const uint16_t v_num) { return (bool)gpio_get_level ((gpio_num_t)v_num); }

    private:
        void setPinConfig (const gpio_num_t v_num, const gpio_int_type_t v_isrType, const gpio_mode_t v_mode, const gpio_pulldown_t v_pullDown, const gpio_pullup_t v_pullUp)
        {
            gpio_config_t config = {};
            config.intr_type     = v_isrType;
            config.mode          = v_mode;
            config.pin_bit_mask  = (1ULL << v_num);
            config.pull_down_en  = v_pullDown;
            config.pull_up_en    = v_pullUp;
            gpio_config          (&config);
            //gpio_install_isr_service (ESP_INTR_FLAG_LEVEL1);    // try remove this from here
            //gpio_set_intr_type   (v_num, v_isrType);
            gpio_isr_handler_add (v_num, isr, (void *)v_num);
        }
};

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
