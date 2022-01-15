
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "esp_bt.h"
#include "Settings.h"
#include "LoggerHw.h"
#include "nvs_flash.h"
#include "GattsConfig.h"
#include "esp_bt_main.h"
#include "BleServerHw.h"
#include "BleSerializer.h"
//#include "SystemTimeHw.h"
#include "esp_gatt_common_api.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// MACROS/DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define ADV_CONFIG_FLAG  (1 << 0)
#define SCAN_CONFIG_FLAG (1 << 1)

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// VARIABLES ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static constexpr char *     GAP_MODULE    = (char *)"GAP";
static constexpr char *     GATTS_MODULE  = (char *)"GATTS";

static std::string          message;
static esp_gatt_char_prop_t aProperty;
static GattsConfig          gattsConfig;
static uint8_t              advConfigDone;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static std::string_view ConvertGapEventToString (esp_gap_ble_cb_event_t v_event)
{
    switch (v_event)
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT     : { return "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT";      }
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT: { return "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT"; }
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT        : { return "ESP_GAP_BLE_ADV_START_COMPLETE_EVT";         }
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT         : { return "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT";          }
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT        : { return "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT";         }
        default                                        : { return "Event not serviced";                         }
    };
}

static std::string_view ConvertGattcEventToString (const esp_gatts_cb_event_t v_event)
{
    switch (v_event)
    {
        case ESP_GATTS_REG_EVT           : { return "ESP_GATTS_REG_EVT";            }
        case ESP_GATTS_READ_EVT          : { return "ESP_GATTS_READ_EVT";           }
        case ESP_GATTS_WRITE_EVT         : { return "ESP_GATTS_WRITE_EVT";          }
        case ESP_GATTS_EXEC_WRITE_EVT    : { return "ESP_GATTS_EXEC_WRITE_EVT";     }
        case ESP_GATTS_MTU_EVT           : { return "ESP_GATTS_MTU_EVT";            }
        case ESP_GATTS_UNREG_EVT         : { return "ESP_GATTS_UNREG_EVT";          }
        case ESP_GATTS_CREATE_EVT        : { return "ESP_GATTS_CREATE_EVT";         }
        case ESP_GATTS_ADD_CHAR_EVT      : { return "ESP_GATTS_ADD_CHAR_EVT";       }
        case ESP_GATTS_ADD_CHAR_DESCR_EVT: { return "ESP_GATTS_ADD_CHAR_DESCR_EVT"; }
        case ESP_GATTS_START_EVT         : { return "ESP_GATTS_START_EVT";          }
        case ESP_GATTS_CONNECT_EVT       : { return "ESP_GATTS_CONNECT_EVT";        }
        case ESP_GATTS_DISCONNECT_EVT    : { return "ESP_GATTS_DISCONNECT_EVT";     }
        case ESP_GATTS_CONF_EVT          : { return "ESP_GATTS_CONF_EVT";           }
        case ESP_GATTS_RESPONSE_EVT      : { return "ESP_GATTS_RESPONSE_EVT";       }
        default                          : { return "Event not serviced";           }
    };
}

static void GapEvent (esp_gap_ble_cb_event_t v_event, esp_ble_gap_cb_param_t * v_param)
{
    LOG (GAP_MODULE, "Event id: 0x%X, name: %s", v_event, ConvertGapEventToString (v_event).data ());

    switch (v_event)
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        {
            advConfigDone &= (~ADV_CONFIG_FLAG);
            if (advConfigDone == ZERO) { esp_ble_gap_start_advertising (&gattsConfig.AdvParams); }
            break;
        }
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        {
            advConfigDone &= (~SCAN_CONFIG_FLAG);
            if (advConfigDone == ZERO) { esp_ble_gap_start_advertising(&gattsConfig.AdvParams); }
            break;
        }
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        {
            if (v_param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) { LOGE (GAP_MODULE, "Advertising start status: 0x%X", v_param->adv_start_cmpl.status); }
            break;
        }
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        {
            if (v_param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) { LOGE (GAP_MODULE, "Advertising stop status: 0x%X", v_param->adv_stop_cmpl.status); }
            break;
        }
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        {
            LOGI (GATTS_MODULE, "Update connection params. Status: 0x%X, min_int: %d, max_int: %d, conn_int: %d, latency: %d, timeout: %d", v_param->update_conn_params.status,
                                                                                                                                            v_param->update_conn_params.min_int,
                                                                                                                                            v_param->update_conn_params.max_int,
                                                                                                                                            v_param->update_conn_params.conn_int,
                                                                                                                                            v_param->update_conn_params.latency,
                                                                                                                                            v_param->update_conn_params.timeout);
            break;
        }
        default:{ break; }
    }
}

static void send (esp_gatt_if_t v_interface, std::string_view v_message, esp_ble_gatts_cb_param_t * v_param)
{
    if (v_param->write.need_rsp)
    {
        esp_gatt_status_t status = ESP_GATT_OK;
        if (v_param->write.is_prep)
        {
            LOGI (GATTS_MODULE, "Send: %s", (const char *)v_param->write.value);

            std::unique_ptr <esp_gatt_rsp_t> response = std::make_unique <esp_gatt_rsp_t> ();
            response->attr_value.len                  = v_param->write.len;
            response->attr_value.handle               = v_param->write.handle;
            response->attr_value.offset               = v_param->write.offset;
            response->attr_value.auth_req             = ESP_GATT_AUTH_REQ_NONE;

            memcpy                                             (response->attr_value.value, v_param->write.value, v_param->write.len);
            esp_err_t statusResp = esp_ble_gatts_send_response (v_interface, v_param->write.conn_id, v_param->write.trans_id, status, response.get());

            if (statusResp != ESP_OK) { LOGE (GATTS_MODULE, "Send response. Status response: %d", statusResp); }
        }
        else
        {
            esp_ble_gatts_send_response (v_interface, v_param->write.conn_id, v_param->write.trans_id, status, NULL);
        }
    }
}

void GattsProfileAEvent (esp_gatts_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gatts_cb_param_t * v_param)
{
    LOG (GATTS_MODULE, "Event id: 0x%X, name: %s", v_event, ConvertGattcEventToString (v_event).data ());

    switch (v_event)
    {
        case ESP_GATTS_REG_EVT:
        {
            gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Id.is_primary          = true;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Id.id.inst_id          = ZERO;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Id.id.uuid.len         = ESP_UUID_LEN_16;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Id.id.uuid.uuid.uuid16 = gattsConfig.Configs [PROFILE_A_APP_ID].UUID.Service;

            esp_err_t status = esp_ble_gap_set_device_name (GattsConfig::Config::ServerName);
            if (status) { LOGE (GATTS_MODULE, "Set device name. Status 0x%X", status); }

            status = esp_ble_gap_config_adv_data (&gattsConfig.AdvData);
            if (status){ LOGE (GATTS_MODULE, "Adv data status: 0x%X", status); }

            advConfigDone |= ADV_CONFIG_FLAG;

            status = esp_ble_gap_config_adv_data (&gattsConfig.ScanData);
            if (status) { LOGE (GATTS_MODULE, "Scan data status: 0x%X", status); }

            advConfigDone |= SCAN_CONFIG_FLAG;
            esp_ble_gatts_create_service (v_interface, &gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Id, gattsConfig.Configs [PROFILE_A_APP_ID].Num);
            break;
        }
        case ESP_GATTS_READ_EVT:
        {
            LOGI (GATTS_MODULE, "Conn_id: %d, trans_id: %d, handle: %d", v_param->read.conn_id,
                                                                         v_param->read.trans_id,
                                                                         v_param->read.handle);
            esp_gatt_rsp_t response = {};
            response.attr_value.handle        = v_param->read.handle;
            response.attr_value.len           = FOUR;
            response.attr_value.value [ZERO]  = 0x12;
            response.attr_value.value [ONE]   = 0x34;
            response.attr_value.value [TWO]   = 0x56;
            response.attr_value.value [THREE] = 0x78;
            //response.attr_value.value [ZERO]  = 0xde;
            //response.attr_value.value [ONE]   = 0xed;
            //response.attr_value.value [TWO]   = 0xbe;
            //response.attr_value.value [THREE] = 0xef;
            LOGI (GATTS_MODULE, "Receive len: %d, data: %s", v_param->conf.len, (char *)v_param->conf.value);
            esp_ble_gatts_send_response (v_interface, v_param->read.conn_id, v_param->read.trans_id, ESP_GATT_OK, &response);
            break;
        }
        case ESP_GATTS_WRITE_EVT:
        {
            if (!v_param->write.is_prep)
            {
                if (gattsConfig.Profiles [PROFILE_A_APP_ID].Description.Handle == v_param->write.handle && v_param->write.len == TWO)
                {
                    uint16_t descriptionValue = v_param->write.value [ONE] << EIGHT | v_param->write.value [ZERO];
                    LOGI (GATTS_MODULE, "Description value: 0x%X", descriptionValue);
                    if (descriptionValue == 0x0001)
                    {
                        if (aProperty & ESP_GATT_CHAR_PROP_BIT_NOTIFY)
                        {
                            LOGI (GATTS_MODULE, "Notify enable");

                            Settings::GetInstance ().Wehicle.eDrive = Settings::EDrive::eBackward;
                            Settings::GetInstance ().Wehicle.Speed  = TWO_HUNDRED;

                            cJSON * root = cJSON_CreateObject ();
                            BleSerializer bleSerialzier;
                            bleSerialzier.Drive (root);

                            char * jsonMessage = cJSON_Print (root);
                            LOGI (GATTS_MODULE, "Send: %s", jsonMessage);

                            esp_ble_gatts_send_indicate (v_interface,
                                                         v_param->write.conn_id,
                                                         gattsConfig.Profiles [PROFILE_A_APP_ID].Charactersitic.Handle,
                                                         strlen (jsonMessage),
                                                         (uint8_t *)jsonMessage,
                                                         false);
                            free         (jsonMessage);
                            cJSON_Delete (root);
                        }
                    }
                    else
                    {
                        LOGE (GATTS_MODULE, "Unserviced description value");
                    }
                }
            }

            send (v_interface, message.data (), v_param);
            break;
        }
        case ESP_GATTS_MTU_EVT:
        {
            LOGI (GATTS_MODULE, "MTU: %d", v_param->mtu.mtu);
            break;
        }
        case ESP_GATTS_CREATE_EVT:
        {
            gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Handle                  = v_param->create.service_handle;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Charactersitic.Uuid.len         = ESP_UUID_LEN_16;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Charactersitic.Uuid.uuid.uuid16 = gattsConfig.Configs [PROFILE_A_APP_ID].UUID.Characteristic;

            esp_ble_gatts_start_service (gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Handle);

            aProperty = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
            esp_err_t status = esp_ble_gatts_add_char (gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Handle,
                                                       &gattsConfig.Profiles [PROFILE_A_APP_ID].Charactersitic.Uuid,
                                                       ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                       aProperty,
                                                       (esp_attr_value_t *)&gattsConfig.DemoChar1Val,
                                                       NULL);

            if (status) { LOGE (GATTS_MODULE, "Add characteristic. Status: 0x%X", status); }
            break;
        }
        case ESP_GATTS_ADD_CHAR_EVT:
        {
            gattsConfig.Profiles [PROFILE_A_APP_ID].Charactersitic.Handle        = v_param->add_char.attr_handle;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Description.Uuid.len         = ESP_UUID_LEN_16;
            gattsConfig.Profiles [PROFILE_A_APP_ID].Description.Uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

            const uint8_t * characteristic;
            uint16_t        characteristicLen = ZERO;
            esp_err_t       status            = esp_ble_gatts_get_attr_value (v_param->add_char.attr_handle,  &characteristicLen, &characteristic);

            if (status != ESP_OK) { LOGE (GATTS_MODULE, "Get attribute value. Status: 0x%X, len: %d", status, characteristicLen); }

            for (auto characteristicNum = ZERO; characteristicNum < characteristicLen; characteristicNum++)
            {
                LOGI (GATTS_MODULE, "Characteristic [%x]: 0x%X", characteristicNum, characteristic [characteristicNum]);
            }

            status = esp_ble_gatts_add_char_descr (gattsConfig.Profiles [PROFILE_A_APP_ID].Service.Handle,
                                                   &gattsConfig.Profiles [PROFILE_A_APP_ID].Description.Uuid,
                                                   ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                   NULL,
                                                   NULL);

            if (status) { LOGE (GATTS_MODULE, "Add characteristic description. Status: 0x%X", status); }
            break;
        }
        case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        {
            gattsConfig.Profiles [PROFILE_A_APP_ID].Description.Handle = v_param->add_char_descr.attr_handle;
            LOGI (GATTS_MODULE, "Status: 0x%X, attr_handle: %d, service_handle: %d", v_param->add_char_descr.status,
                                                                                     v_param->add_char_descr.attr_handle,
                                                                                     v_param->add_char_descr.service_handle);
            break;
        }
        case ESP_GATTS_START_EVT:
        {
            LOGI (GATTS_MODULE, "App_id: 0x%X, status: 0x%X, service_handle: %d", v_param->reg.app_id,
                                                                                  v_param->start.status,
                                                                                  v_param->start.service_handle);
            break;
        }
        case ESP_GATTS_CONNECT_EVT:
        {

            memcpy (gattsConfig.ConnParams.bda, v_param->connect.remote_bda, sizeof (esp_bd_addr_t));

            LOGI (GATTS_MODULE, "Remote client mac: %02X:%02X:%02X:%02X:%02X:%02X:", v_param->connect.remote_bda [ZERO],
                                                                                     v_param->connect.remote_bda [ONE],
                                                                                     v_param->connect.remote_bda [TWO],
                                                                                     v_param->connect.remote_bda [THREE],
                                                                                     v_param->connect.remote_bda [FOUR],
                                                                                     v_param->connect.remote_bda [FIVE]);

            gattsConfig.Profiles [PROFILE_A_APP_ID].ConnId = v_param->connect.conn_id;

            esp_ble_gap_update_conn_params (&gattsConfig.ConnParams);
            break;
        }
        case ESP_GATTS_DISCONNECT_EVT:
        {
            LOGI                          (GATTS_MODULE, "Disconnect reason: 0x%X", v_param->disconnect.reason);
            esp_ble_gap_start_advertising (&gattsConfig.AdvParams);
            break;
        }
        case ESP_GATTS_CONF_EVT:
        {
            if (v_param->conf.status != ESP_GATT_OK) { LOGE (GATTS_MODULE, "Value: %s", v_param->conf.value); }
            break;
        }
        default: { break; }
    }
}

static void GattsEvent (esp_gatts_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gatts_cb_param_t * v_param)
{
    if (v_event == ESP_GATTS_REG_EVT)
    {
        if (v_param->reg.status == ESP_GATT_OK) { gattsConfig.Profiles [v_param->reg.app_id].Interface = v_interface; }
        else
        {
            LOGI (GATTS_MODULE, "App_id: %04X, status: %d", v_param->reg.app_id,
                                                            v_param->reg.status);
            return;
        }
    }

    do
    {
        for (uint8_t profileNum = ZERO; profileNum < PROFILES_NUM; profileNum++)
        {
            if (v_interface == ESP_GATT_IF_NONE || v_interface == gattsConfig.Profiles [profileNum].Interface)
            {
                if (gattsConfig.Profiles [profileNum].Callback) { gattsConfig.Profiles [profileNum].Callback (v_event, v_interface, v_param); }
            }
        }
    } while (ZERO);
}

BleServerHw::BleServerHw ()
{
    ESP_ERROR_CHECK (esp_bt_controller_mem_release (ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t btConfig = BT_CONTROLLER_INIT_CONFIG_DEFAULT ();
    esp_err_t status = esp_bt_controller_init (&btConfig);
    if (status)
    {
        LOGE (GATTS_MODULE, "Initialize bluetooth controller. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bt_controller_enable (ESP_BT_MODE_BLE);
    if (status)
    {
        LOGE (GATTS_MODULE, "Enable bluetooth controller. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bluedroid_init ();
    if (status)
    {
        LOGE (GATTS_MODULE, "Init bluetooth stack. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bluedroid_enable ();
    if (status)
    {
        LOGE (GATTS_MODULE, "Enable bluetooth. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_ble_gatts_register_callback (GattsEvent);
    if (status)
    {
        LOGE (GATTS_MODULE, "Register Gatts callback. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gap_register_callback (GapEvent);
    if (status)
    {
        LOGE (GATTS_MODULE, "Register GAP callback. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gatts_app_register (PROFILE_A_APP_ID);
    if (status)
    {
        LOGE (GATTS_MODULE, "Gatts app a register. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gatt_set_local_mtu (FIVE_HUNDRED);
    if (status) { LOGE (GATTS_MODULE, "Set local  MTU. Status: 0x%X", status); }
}

extern "C"
{
    void app_main(void)
    {
        esp_err_t status = nvs_flash_init ();
        if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK (nvs_flash_erase ());
            status = nvs_flash_init ();
        }

        ESP_ERROR_CHECK (status);

        //static SystemTimeHw systemTimeHw;
        //SystemTime::GetInstance ()->SetInstance (&systemTimeHw);

        BleServerHw bleServerHw;

        return;
    }
}
