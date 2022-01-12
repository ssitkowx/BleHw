
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "UartHw.h"
#include "LoggerHw.h"
#include "Settings.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// MACROS/DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define PATTERN_CHR_NUM (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/
#define DATA_LEN        100

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// VARIABLES ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static constexpr char *  module = (char *)"UartHw";
static UartHw            uart;
static QueueHandle_t     uartQueueHandle;
extern SemaphoreHandle_t gpioSemaphore;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static std::string_view getEvent (uart_event_type_t v_event)
{
    switch (v_event)
    {
        case UART_DATA:        return "UART_DATA";
        case UART_BREAK:       return "UART_BREAK";
        case UART_BUFFER_FULL: return "UART_BUFFER_FULL";
        case UART_FIFO_OVF:    return "UART_FIFO_OVF";
        case UART_FRAME_ERR:   return "UART_FRAME_ERR";
        case UART_PARITY_ERR:  return "UART_PARITY_ERR";
        case UART_DATA_BREAK:  return "UART_DATA_BREAK";
        case UART_PATTERN_DET: return "UART_PATTERN_DET";
        case UART_EVENT_MAX:   return "UART_EVENT_MAX";
        default:               return "Unknown";
    };
}

static constexpr int ConvertToUartNum (const UartHw::EUart v_eUartNum)
{
    switch (static_cast<int>(v_eUartNum))
    {
        case static_cast<int>(UartHw::EUart::e0): { return UART_NUM_0; }
        case static_cast<int>(UartHw::EUart::e1): { return UART_NUM_1; }
        case static_cast<int>(UartHw::EUart::e2): { return UART_NUM_2; }
    };

    return UART_NUM_0;
}

static void uartProcess (void * v_pvParameters)
{
    uart_event_t event;
    size_t       dataLen;

    while (ONE)
    {
        //Waiting for UART event.
        if (xQueueReceive (uartQueueHandle, (void * )&event, (portTickType)portMAX_DELAY))
        {
            LOGI (module, "Event type: %s", getEvent (event.type).data ());
            switch (event.type)
            {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                {
                    //uart_read_bytes  (ConvertToUartNum (UartHw::EUart::e0), Settings::GetInstance ().Command.data(), event.size, portMAX_DELAY);
                    uart.Receive (UartHw::EUart::e0, (uint8_t *)Settings::GetInstance ().Command.data (), event.size);
                    LOGI         (module, "Receive: data: %s, len: %d", Settings::GetInstance ().Command.data(), event.size);
                    //uart_write_bytes (ConvertToUartNum (UartHw::EUart::e0), Settings::GetInstance ().Command.data(), event.size);
                    xSemaphoreGive (gpioSemaphore);
                    break;
                //Event of HW FIFO overflow detected
                }
                case UART_FIFO_OVF:
                {
                    LOGI             (module, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input (ConvertToUartNum (UartHw::EUart::e0));
                    xQueueReset      (uartQueueHandle);
                    break;
                //Event of UART ring buffer full
                }
                case UART_BUFFER_FULL:
                {
                    LOGI         (module, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input (ConvertToUartNum (UartHw::EUart::e0));
                    xQueueReset      (uartQueueHandle);
                    break;
                //Event of UART RX break detected
                }
                case UART_BREAK:
                {
                    LOGI (module, "uart rx break");
                    break;
                //Event of UART parity check error
                }
                case UART_PARITY_ERR:
                {
                    LOGI (module, "uart parity error");
                    break;
                //Event of UART frame error
                }
                case UART_FRAME_ERR:
                {
                    LOGI (module, "uart frame error");
                    break;
                //UART_PATTERN_DET
                }
                case UART_PATTERN_DET:
                {
                    uart_get_buffered_data_len     (ConvertToUartNum (UartHw::EUart::e0), &dataLen);
                    int pos = uart_pattern_pop_pos (ConvertToUartNum (UartHw::EUart::e0));
                    LOGI                           (module, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, dataLen);
                    if (pos == -1)
                    {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input (ConvertToUartNum (UartHw::EUart::e0));
                    }
                    else
                    {
                        uart_read_bytes (ConvertToUartNum (UartHw::EUart::e0), Settings::GetInstance ().Command.data (), pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat [PATTERN_CHR_NUM + 1];

                        memset          (pat, 0, sizeof(pat));
                        uart_read_bytes (ConvertToUartNum (UartHw::EUart::e0), pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        LOGI            (module, "read data: %s", Settings::GetInstance ().Command.data ());
                        LOGI            (module, "read pat : %s", pat);
                    }
                    break;
                //Others
                }
                default:
                {
                    LOGI (module, "uart event type: %d", event.type);
                    break;
                }
            }
        }
    }

    vTaskDelete (NULL);
}

UartHw::UartHw ()
{
    uart_config_t config =
    {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    uart_driver_install               (ConvertToUartNum (UartHw::EUart::e0), DATA_LEN * TWO, DATA_LEN * TWO, TWENTY, &uartQueueHandle, ZERO);
    uart_param_config                 (ConvertToUartNum (UartHw::EUart::e0), &config);
    esp_log_level_set                 (module, ESP_LOG_INFO);
    uart_set_pin                      (ConvertToUartNum (UartHw::EUart::e0), UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_enable_pattern_det_baud_intr (ConvertToUartNum (UartHw::EUart::e0), '+', PATTERN_CHR_NUM, NINE, ZERO, ZERO);
    uart_pattern_queue_reset          (ConvertToUartNum (UartHw::EUart::e0), TWENTY);
    xTaskCreate                       (uartProcess, "uartProcess", 3000, NULL, TWELVE, NULL);
}

void UartHw::Send (const EUart v_eUartNum, uint8_t * v_data, const uint16_t v_dataLen)
{
    uart_write_bytes (ConvertToUartNum (v_eUartNum), static_cast<const uint8_t *>(v_data), v_dataLen);
}

void UartHw::Receive (const EUart v_eUartNum, uint8_t * v_data, const uint16_t v_dataLen)
{
    uart_read_bytes (ConvertToUartNum (v_eUartNum), v_data, v_dataLen, ONE_HUNDRED / portTICK_PERIOD_MS);
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
