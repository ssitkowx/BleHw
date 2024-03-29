
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "GpioHw.h"
#include <stdint.h>
#include <string.h>
#include "UartHw.h"
#include "nvs_flash.h"
#include "BleClientHw.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    void app_main (void)
    {
        esp_err_t status = nvs_flash_init ();
        if (status == ESP_ERR_NVS_NO_FREE_PAGES ||
            status == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK (nvs_flash_erase());
            status = nvs_flash_init();
        }

        ESP_ERROR_CHECK (status);

        //static SystemTimeHw systemTimeHw;
        //SystemTime::GetInstance ()->SetInstance (&systemTimeHw);

        GpioHw      gpioHw;
        UartHw      uartHw;
        BleClientHw bleClientHw;
    }
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
