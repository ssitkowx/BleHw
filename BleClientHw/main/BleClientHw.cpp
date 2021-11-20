
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "Utils.h"
#include "GpioHw.h"
#include "esp_bt.h"
#include <string.h>
#include "Settings.h"
#include "LoggerHw.h"
#include "nvs_flash.h"
#include "esp_bt_main.h"
#include "GattcConfig.h"
#include "BleClientHw.h"
//#include "SystemTimeHw.h"
#include "esp_gatt_common_api.h"
#include "BleParserAndSerializer.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// VARIABLES ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static constexpr char * Module                      = (char *)"Main";
static constexpr char * GAP_MODULE                  = (char *)"BleClientGap";
static constexpr char * GATTC_MODULE                = (char *)"BleClientGatt";
static bool             connectGattc                = false;
static bool             isServiceAvaliable          = false;
static GattcConfig      gGattcConfig;
static xQueueHandle     queueHandle;
bool                    BleClientHw::IsNewDataReady = false;

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GapCallback   (esp_gap_ble_cb_event_t v_event, esp_ble_gap_cb_param_t * v_param);
static void GattcCallback (esp_gattc_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gattc_cb_param_t * v_param);

static std::array <char, FORTY> ConvertGapEventToString (esp_gap_ble_cb_event_t v_event)
{
    std::array <char, FORTY> event = {};

    switch (v_event)
    {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: { memcpy (event.data(), "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT", strlen ("ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT")); break; }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:     { memcpy (event.data(), "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT"    , strlen ("ESP_GAP_BLE_SCAN_START_COMPLETE_EVT"));     break; }
        case ESP_GAP_BLE_SCAN_RESULT_EVT:             { memcpy (event.data(), "ESP_GAP_BLE_SCAN_RESULT_EVT"            , strlen ("ESP_GAP_BLE_SCAN_RESULT_EVT"));             break; }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:      { memcpy (event.data(), "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT"     , strlen ("ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT"));      break; }
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:       { memcpy (event.data(), "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT"      , strlen ("ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT"));       break; }
        default:                                      { memcpy (event.data(), "Event not serviced"                     , strlen ("Event not serviced"));                      break; }
    };

    return event;
}

static std::array <char, THIRTY> ConvertGattcEventToString (esp_gattc_cb_event_t v_event)
{
    std::array <char, THIRTY> event = {};

    switch (v_event)
    {
        case ESP_GATTC_REG_EVT:              { memcpy (event.data(), "ESP_GATTC_REG_EVT"             , strlen ("ESP_GATTC_REG_EVT"));              break; }
        case ESP_GATTC_UNREG_EVT:            { memcpy (event.data(), "ESP_GATTC_UNREG_EVT"           , strlen ("ESP_GATTC_UNREG_EVT"));            break; }
        case ESP_GATTC_OPEN_EVT:             { memcpy (event.data(), "ESP_GATTC_OPEN_EVT"            , strlen ("ESP_GATTC_OPEN_EVT"));             break; }
        case ESP_GATTC_READ_CHAR_EVT:        { memcpy (event.data(), "ESP_GATTC_READ_CHAR_EVT"       , strlen ("ESP_GATTC_READ_CHAR_EVT"));        break; }
        case ESP_GATTC_WRITE_CHAR_EVT:       { memcpy (event.data(), "ESP_GATTC_WRITE_CHAR_EVT"      , strlen ("ESP_GATTC_WRITE_CHAR_EVT"));       break; }
        case ESP_GATTC_CLOSE_EVT:            { memcpy (event.data(), "ESP_GATTC_CLOSE_EVT"           , strlen ("ESP_GATTC_CLOSE_EVT"));            break; }
        case ESP_GATTC_SEARCH_CMPL_EVT:      { memcpy (event.data(), "ESP_GATTC_SEARCH_CMPL_EVT"     , strlen ("ESP_GATTC_SEARCH_CMPL_EVT"));      break; }
        case ESP_GATTC_SEARCH_RES_EVT:       { memcpy (event.data(), "ESP_GATTC_SEARCH_RES_EVT"      , strlen ("ESP_GATTC_SEARCH_RES_EVT"));       break; }
        case ESP_GATTC_READ_DESCR_EVT:       { memcpy (event.data(), "ESP_GATTC_READ_DESCR_EVT"      , strlen ("ESP_GATTC_READ_DESCR_EVT"));       break; }
        case ESP_GATTC_WRITE_DESCR_EVT:      { memcpy (event.data(), "ESP_GATTC_WRITE_DESCR_EVT"     , strlen ("ESP_GATTC_WRITE_DESCR_EVT"));      break; }
        case ESP_GATTC_NOTIFY_EVT:           { memcpy (event.data(), "ESP_GATTC_NOTIFY_EVT"          , strlen ("ESP_GATTC_NOTIFY_EVT"));           break; }
        case ESP_GATTC_PREP_WRITE_EVT:       { memcpy (event.data(), "ESP_GATTC_PREP_WRITE_EVT"      , strlen ("ESP_GATTC_PREP_WRITE_EVT"));       break; }
        case ESP_GATTC_EXEC_EVT:             { memcpy (event.data(), "ESP_GATTC_EXEC_EVT"            , strlen ("ESP_GATTC_EXEC_EVT"));             break; }
        case ESP_GATTC_ACL_EVT:              { memcpy (event.data(), "ESP_GATTC_ACL_EVT"             , strlen ("ESP_GATTC_ACL_EVT"));              break; }
        case ESP_GATTC_CANCEL_OPEN_EVT:      { memcpy (event.data(), "ESP_GATTC_CANCEL_OPEN_EVT"     , strlen ("ESP_GATTC_CANCEL_OPEN_EVT"));      break; }
        case ESP_GATTC_SRVC_CHG_EVT:         { memcpy (event.data(), "ESP_GATTC_SRVC_CHG_EVT"        , strlen ("ESP_GATTC_SRVC_CHG_EVT"));         break; }
        case ESP_GATTC_ENC_CMPL_CB_EVT:      { memcpy (event.data(), "ESP_GATTC_ENC_CMPL_CB_EVT"     , strlen ("ESP_GATTC_ENC_CMPL_CB_EVT"));      break; }
        case ESP_GATTC_CFG_MTU_EVT:          { memcpy (event.data(), "ESP_GATTC_CFG_MTU_EVT"         , strlen ("ESP_GATTC_CFG_MTU_EVT"));          break; }
        case ESP_GATTC_ADV_DATA_EVT:         { memcpy (event.data(), "ESP_GATTC_ADV_DATA_EVT"        , strlen ("ESP_GATTC_ADV_DATA_EVT"));         break; }
        case ESP_GATTC_MULT_ADV_ENB_EVT:     { memcpy (event.data(), "ESP_GATTC_MULT_ADV_ENB_EVT"    , strlen ("ESP_GATTC_MULT_ADV_ENB_EVT"));     break; }
        case ESP_GATTC_MULT_ADV_UPD_EVT:     { memcpy (event.data(), "ESP_GATTC_MULT_ADV_UPD_EVT"    , strlen ("ESP_GATTC_MULT_ADV_UPD_EVT"));     break; }
        case ESP_GATTC_MULT_ADV_DATA_EVT:    { memcpy (event.data(), "ESP_GATTC_MULT_ADV_DATA_EVT"   , strlen ("ESP_GATTC_MULT_ADV_DATA_EVT"));    break; }
        case ESP_GATTC_MULT_ADV_DIS_EVT:     { memcpy (event.data(), "ESP_GATTC_MULT_ADV_DIS_EVT"    , strlen ("ESP_GATTC_MULT_ADV_DIS_EVT"));     break; }
        case ESP_GATTC_CONGEST_EVT:          { memcpy (event.data(), "ESP_GATTC_CONGEST_EVT"         , strlen ("ESP_GATTC_CONGEST_EVT"));          break; }
        case ESP_GATTC_BTH_SCAN_ENB_EVT:     { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_ENB_EVT"    , strlen ("ESP_GATTC_BTH_SCAN_ENB_EVT"));     break; }
        case ESP_GATTC_BTH_SCAN_CFG_EVT:     { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_CFG_EVT"    , strlen ("ESP_GATTC_BTH_SCAN_CFG_EVT"));     break; }
        case ESP_GATTC_BTH_SCAN_RD_EVT:      { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_RD_EVT"     , strlen ("ESP_GATTC_BTH_SCAN_RD_EVT"));      break; }
        case ESP_GATTC_BTH_SCAN_THR_EVT:     { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_THR_EVT"    , strlen ("ESP_GATTC_BTH_SCAN_THR_EVT"));     break; }
        case ESP_GATTC_BTH_SCAN_PARAM_EVT:   { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_PARAM_EVT"  , strlen ("ESP_GATTC_BTH_SCAN_PARAM_EVT"));   break; }
        case ESP_GATTC_BTH_SCAN_DIS_EVT:     { memcpy (event.data(), "ESP_GATTC_BTH_SCAN_DIS_EVT"    , strlen ("ESP_GATTC_BTH_SCAN_DIS_EVT"));     break; }
        case ESP_GATTC_SCAN_FLT_CFG_EVT:     { memcpy (event.data(), "ESP_GATTC_SCAN_FLT_CFG_EVT"    , strlen ("ESP_GATTC_SCAN_FLT_CFG_EVT"));     break; }
        case ESP_GATTC_SCAN_FLT_STATUS_EVT:  { memcpy (event.data(), "ESP_GATTC_SCAN_FLT_STATUS_EVT" , strlen ("ESP_GATTC_SCAN_FLT_STATUS_EVT"));  break; }
        case ESP_GATTC_ADV_VSC_EVT:          { memcpy (event.data(), "ESP_GATTC_ADV_VSC_EVT"         , strlen ("ESP_GATTC_ADV_VSC_EVT"));          break; }
        case ESP_GATTC_REG_FOR_NOTIFY_EVT:   { memcpy (event.data(), "ESP_GATTC_REG_FOR_NOTIFY_EVT"  , strlen ("ESP_GATTC_REG_FOR_NOTIFY_EVT"));   break; }
        case ESP_GATTC_UNREG_FOR_NOTIFY_EVT: { memcpy (event.data(), "ESP_GATTC_UNREG_FOR_NOTIFY_EVT", strlen ("ESP_GATTC_UNREG_FOR_NOTIFY_EVT")); break; }
        case ESP_GATTC_CONNECT_EVT:          { memcpy (event.data(), "ESP_GATTC_CONNECT_EVT"         , strlen ("ESP_GATTC_CONNECT_EVT"));          break; }
        case ESP_GATTC_DISCONNECT_EVT:       { memcpy (event.data(), "ESP_GATTC_DISCONNECT_EVT"      , strlen ("ESP_GATTC_DISCONNECT_EVT"));       break; }
        case ESP_GATTC_READ_MULTIPLE_EVT:    { memcpy (event.data(), "ESP_GATTC_READ_MULTIPLE_EVT"   , strlen ("ESP_GATTC_READ_MULTIPLE_EVT"));    break; }
        case ESP_GATTC_QUEUE_FULL_EVT:       { memcpy (event.data(), "ESP_GATTC_QUEUE_FULL_EVT"      , strlen ("ESP_GATTC_QUEUE_FULL_EVT"));       break; }
        case ESP_GATTC_SET_ASSOC_EVT:        { memcpy (event.data(), "ESP_GATTC_SET_ASSOC_EVT"       , strlen ("ESP_GATTC_SET_ASSOC_EVT"));        break; }
        case ESP_GATTC_GET_ADDR_LIST_EVT:    { memcpy (event.data(), "ESP_GATTC_GET_ADDR_LIST_EVT"   , strlen ("ESP_GATTC_GET_ADDR_LIST_EVT"));    break; }
        case ESP_GATTC_DIS_SRVC_CMPL_EVT:    { memcpy (event.data(), "ESP_GATTC_DIS_SRVC_CMPL_EVT"   , strlen ("ESP_GATTC_DIS_SRVC_CMPL_EVT"));    break; }
        default:                             { memcpy (event.data(), "Event not serviced"            , strlen ("Event not serviced"));             break; }
    };

    return event;
}

static bool searchServerName (esp_ble_gap_cb_param_t * v_params)
{
    uint8_t   serverNameLen = ZERO;
    uint8_t * serverName    = esp_ble_resolve_adv_data (v_params->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &serverNameLen);

    if (serverName != NULL)
    {
        if (strlen (gGattcConfig.Config.ServerName)                                     == serverNameLen &&
            strncmp ((char *)serverName, gGattcConfig.Config.ServerName, serverNameLen) == ZERO)
        {
            return true;
        }
    }

    return false;
}

static void GapCallback (esp_gap_ble_cb_event_t v_event, esp_ble_gap_cb_param_t * v_params)
{
    LOG (GAP_MODULE, "Event id: 0x%X, name: %s", v_event, ConvertGapEventToString (v_event).data ());

    switch (v_event)
    {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        {
            esp_ble_gap_start_scanning (THIRTY);
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        {
            if (v_params->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) { LOGE (GAP_MODULE, "Status: 0x%X", v_params->scan_start_cmpl.status); }
            break;
        }
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
        {
            esp_ble_gap_cb_param_t * params = (esp_ble_gap_cb_param_t *)v_params;
            switch (params->scan_rst.search_evt)
            {
                case ESP_GAP_SEARCH_INQ_RES_EVT:
                {
                    LOGI (GAP_MODULE, "Search: adv data len: %d, scan data len: %d", params->scan_rst.adv_data_len,
                                                                                     params->scan_rst.scan_rsp_len);

                    if (searchServerName (params) == true)
                    {
                        LOGI (GAP_MODULE, "Server name found: %s", gGattcConfig.Config.ServerName);
                        if (connectGattc == false)
                        {
                            connectGattc = true;
                            LOGI                      (GAP_MODULE, "Connect to the remote device");
                            esp_ble_gap_stop_scanning ();
                            esp_ble_gattc_open        (gGattcConfig.Profiles [ZERO].Interface,
                                                       params->scan_rst.bda,
                                                       params->scan_rst.ble_addr_type,
                                                       true);
                        }
                    }
                    break;
                }
                default: { break; }
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        {
            if (v_params->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) { LOGE (GAP_MODULE, "Status: 0x%X", v_params->scan_stop_cmpl.status); }
            break;
        }
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        {
            if (v_params->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) { LOGE (GAP_MODULE, "Status: 0x%X", v_params->adv_stop_cmpl.status); }
            break;
        }
        default:
            break;
    }
}

void GattcEvent (esp_gattc_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gattc_cb_param_t * v_params)
{
    LOG (GATTC_MODULE, "Event id: 0x%X, name: %s", v_event, ConvertGattcEventToString (v_event).data ());

    esp_ble_gattc_cb_param_t * params = (esp_ble_gattc_cb_param_t *)v_params;

    switch (v_event)
    {
        case ESP_GATTC_REG_EVT:
        {
            esp_err_t status = esp_ble_gap_set_scan_params (&gGattcConfig.ScanParams);
            if (status){ LOGE (GATTC_MODULE, "Set scan params: 0x%X", status); }
            break;
        }
        case ESP_GATTC_CONNECT_EVT:
        {
            gGattcConfig.Profiles [ZERO].ConnId = params->connect.conn_id;
            memcpy (gGattcConfig.Profiles [ZERO].ServerMac, params->connect.remote_bda, sizeof (esp_bd_addr_t));

            LOGI (GATTC_MODULE, "Remote server mac: %02X:%02X:%02X:%02X:%02X:%02X", gGattcConfig.Profiles [ZERO].ServerMac [ZERO],
                                                                                    gGattcConfig.Profiles [ZERO].ServerMac [ONE],
                                                                                    gGattcConfig.Profiles [ZERO].ServerMac [TWO],
                                                                                    gGattcConfig.Profiles [ZERO].ServerMac [THREE],
                                                                                    gGattcConfig.Profiles [ZERO].ServerMac [FOUR],
                                                                                    gGattcConfig.Profiles [ZERO].ServerMac [FIVE]);

            esp_err_t status = esp_ble_gattc_send_mtu_req (v_interface, params->connect.conn_id);
            if (status) { LOGE (GATTC_MODULE, "Send mtu request: 0x%X", status); }
            break;
        }
        case ESP_GATTC_OPEN_EVT:
        {
            if (v_params->open.status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Open status: 0x%X", params->open.status); }
            break;
        }
        case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        {
            if (v_params->dis_srvc_cmpl.status != ESP_GATT_OK)
            {
                LOGE (GATTC_MODULE, "Discover service status: 0x%X", v_params->dis_srvc_cmpl.status);
                break;
            }

            esp_ble_gattc_search_service (v_interface, v_params->cfg_mtu.conn_id, &gGattcConfig.FiltersUuid.Service);
            break;
        }
        case ESP_GATTC_CFG_MTU_EVT:
        {
            if (v_params->cfg_mtu.status != ESP_GATT_OK) { LOGE (GATTC_MODULE,"Mtu status: 0x%X", v_params->cfg_mtu.status); }

            LOGI (GATTC_MODULE, "Status: 0x%X, mtu: %d, conn_id: 0x%X", v_params->cfg_mtu.status,
                                                                        v_params->cfg_mtu.mtu,
                                                                        v_params->cfg_mtu.conn_id);
            break;
        }
        case ESP_GATTC_SEARCH_RES_EVT:
        {
            if (params->search_res.srvc_id.uuid.len         == gGattcConfig.FiltersUuid.Service.len &&
                params->search_res.srvc_id.uuid.uuid.uuid16 == gGattcConfig.FiltersUuid.Service.uuid.uuid16)
            {
                LOGI (GATTC_MODULE, "Service with uuid: 0x%X found", (uint16_t)params->search_res.srvc_id.uuid.uuid.uuid16);

                isServiceAvaliable                                = true;
                gGattcConfig.Profiles [ZERO].Handle.Service.Start = params->search_res.start_handle;
                gGattcConfig.Profiles [ZERO].Handle.Service.End   = params->search_res.end_handle;
            }
            else
            {
                LOGI (GATTC_MODULE, "Service with uuid: 0x%X does not found", (uint16_t)params->search_res.srvc_id.uuid.uuid.uuid16);
            }
            break;
        }
        case ESP_GATTC_SEARCH_CMPL_EVT:
        {
            if (params->search_cmpl.status != ESP_GATT_OK)
            {
                LOGE (GATTC_MODULE, "Search service. Status: 0x%X", params->search_cmpl.status);
                break;
            }

            if      (params->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) { LOGI (GATTC_MODULE, "Get service information from remote device"); }
            else if (params->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH)     { LOGI (GATTC_MODULE, "Get service information from flash");         }
            else                                                                                         { LOGI (GATTC_MODULE, "Unknown service source");                     }

            if (isServiceAvaliable)
            {
                uint16_t characterisiticNum = ZERO;
                esp_gatt_status_t status    = esp_ble_gattc_get_attr_count (v_interface,
                                                                            params->search_cmpl.conn_id,
                                                                            ESP_GATT_DB_CHARACTERISTIC,
                                                                            gGattcConfig.Profiles [ZERO].Handle.Service.Start,
                                                                            gGattcConfig.Profiles [ZERO].Handle.Service.End,
                                                                            ZERO,
                                                                            &characterisiticNum);

                if (status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Can't get attribute number: Status: 0x%X", status); }

                LOGI (GATTC_MODULE, "Characteristic numbers: %d", characterisiticNum);

                if (characterisiticNum > ZERO)
                {
                    auto characteristics = std::make_unique <esp_gattc_char_elem_t []> (characterisiticNum);
                    status = esp_ble_gattc_get_char_by_uuid (v_interface,
                                                             params->search_cmpl.conn_id,
                                                             gGattcConfig.Profiles [ZERO].Handle.Service.Start,
                                                             gGattcConfig.Profiles [ZERO].Handle.Service.End,
                                                             gGattcConfig.FiltersUuid.Characteristic,
                                                             characteristics.get (),
                                                             &characterisiticNum);

                    if (status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Search charactersitics in remote. Status: 0x%X, characterisitic uuid: 0x%X, properties: 0x%X", status,
                                                                                                                                                                     characteristics.get()->uuid,
                                                                                                                                                                     characteristics.get()->properties);
                                               }

                    if (characterisiticNum > ZERO && (characteristics [ZERO].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY))
                    {
                        gGattcConfig.Profiles [ZERO].Handle.Characteristic = characteristics [ZERO].char_handle;
                        esp_ble_gattc_register_for_notify (v_interface, gGattcConfig.Profiles [ZERO].ServerMac,
                                                                        characteristics [ZERO].char_handle);
                    }
                }
                else
                {
                    LOGE (GATTC_MODULE, "No charactersistics found");
                }
            }
            break;
        }
        case ESP_GATTC_REG_FOR_NOTIFY_EVT:
        {
            if (params->reg_for_notify.status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Status: %d", params->reg_for_notify.status); }
            else
            {
                uint16_t attributeNum    = ZERO;
                esp_gatt_status_t status = esp_ble_gattc_get_attr_count (v_interface,
                                                                         gGattcConfig.Profiles [ZERO].ConnId,
                                                                         ESP_GATT_DB_DESCRIPTOR,
                                                                         gGattcConfig.Profiles [ZERO].Handle.Service.Start,
                                                                         gGattcConfig.Profiles [ZERO].Handle.Service.End,
                                                                         gGattcConfig.Profiles [ZERO].Handle.Characteristic,
                                                                         &attributeNum);

                if (status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Get attribute number: Status: 0x%X", status); }

                if (attributeNum > ZERO)
                {
                    auto description = std::make_unique <esp_gattc_descr_elem_t []> (attributeNum);

                    status = esp_ble_gattc_get_descr_by_char_handle (v_interface,
                                                                     gGattcConfig.Profiles [ZERO].ConnId,
                                                                     params->reg_for_notify.handle,
                                                                     gGattcConfig.FiltersUuid.Description,
                                                                     description.get (),
                                                                     &attributeNum);

                    if (status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Get description by characteristic"); }

                    if (attributeNum                         > ZERO            &&
                        description [ZERO].uuid.len         == ESP_UUID_LEN_16 &&
                        description [ZERO].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG)
                    {
                        uint16_t notifyEn = ONE;
                        status = (esp_gatt_status_t)esp_ble_gattc_write_char_descr (v_interface,
                                                                                    gGattcConfig.Profiles [ZERO].ConnId,
                                                                                    description [ZERO].handle,
                                                                                    sizeof (notifyEn),
                                                                                    (uint8_t *)&notifyEn,
                                                                                    ESP_GATT_WRITE_TYPE_RSP,
                                                                                    ESP_GATT_AUTH_REQ_NONE);
                    }

                    if (status != ESP_GATT_OK) { LOGE (GATTC_MODULE, "Write characteristics description"); }
                }
                else { LOGE (GATTC_MODULE, "Attribute decsription not found"); }
            }
            break;
        }
        case ESP_GATTC_NOTIFY_EVT:
        {
            if (params->notify.is_notify) { LOGI (GATTC_MODULE, "Notify value");   }
            else                          { LOGI (GATTC_MODULE, "Indicate value"); }

            LOGI (GATTC_MODULE, "Receive: %s", params->notify.value);
            break;
        }
        case ESP_GATTC_WRITE_DESCR_EVT:
        {
            if (params->write.status != ESP_GATT_OK)
            {
                LOGE (GATTC_MODULE, "Status: 0x%X", params->write.status);
                break;
            }

            Settings::GetInstance ().BleMsgType.Id             = TWENTY;
            Settings::GetInstance ().BleMsgType.Name           = "Client";
            Settings::GetInstance ().BleMsgType.Data           = "This is my not very long payload from client";
            Settings::GetInstance ().BleMsgType.eOperatingMode = Settings::eNormal;

            cJSON * root = cJSON_CreateObject ();
            BleParserAndSerializer bleParserAndSerialzier;
            bleParserAndSerialzier.Serialize (root);

            char * jsonMessage = cJSON_Print (root);
            LOGI (GATTC_MODULE, "Send: %s", jsonMessage);

            esp_ble_gattc_write_char (v_interface,
                                      gGattcConfig.Profiles [ZERO].ConnId,
                                      gGattcConfig.Profiles [ZERO].Handle.Characteristic,
                                      strlen (jsonMessage),
                                      (uint8_t *)jsonMessage,
                                      ESP_GATT_WRITE_TYPE_RSP,
                                      ESP_GATT_AUTH_REQ_NONE);

            free         (jsonMessage);
            cJSON_Delete (root);
            break;
        }
        case ESP_GATTC_SRVC_CHG_EVT:
        {
            LOGI (GATTC_MODULE, "Remote server mac: %02X:%02X:%02X:%02X:%02X:%02X:", params->srvc_chg.remote_bda [ZERO],
                                                                                     params->srvc_chg.remote_bda [ONE],
                                                                                     params->srvc_chg.remote_bda [TWO],
                                                                                     params->srvc_chg.remote_bda [THREE],
                                                                                     params->srvc_chg.remote_bda [FOUR],
                                                                                     params->srvc_chg.remote_bda [FIVE]);
            break;
        }
        case ESP_GATTC_WRITE_CHAR_EVT:
        {
            if (params->write.status != ESP_GATT_OK)
            {
                LOGE (GATTC_MODULE, "Status = 0x%X", params->write.status);
                break;
            }
            break;
        }
        case ESP_GATTC_DISCONNECT_EVT:
        {
            connectGattc       = false;
            isServiceAvaliable = false;

            LOGI (GATTC_MODULE, "Reason: 0x%X", params->disconnect.reason);
            break;
        }
        default: { break; }
    }
}

static void GattcCallback (esp_gattc_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gattc_cb_param_t * v_param)
{
    if (v_event == ESP_GATTC_REG_EVT)
    {
        if (v_param->reg.status == ESP_GATT_OK) { gGattcConfig.Profiles [v_param->reg.app_id].Interface = v_interface; }
        else
        {
            LOGE (GAP_MODULE, "AppId: %04x. Status: 0x%X", v_param->reg.app_id,
                                                           v_param->reg.status);
            return;
        }
    }

    for (uint8_t appNum = ZERO; appNum < (sizeof (gGattcConfig.Profiles) / sizeof (gGattcConfig.Profiles [ZERO])); appNum++)
    {
        if (v_interface == ESP_GATT_IF_NONE ||
            v_interface == gGattcConfig.Profiles [appNum].Interface)
        {
            if (gGattcConfig.Profiles [appNum].Callback) { gGattcConfig.Profiles [appNum].Callback (v_event, v_interface, v_param); }
        }
    }
}

BleClientHw::BleClientHw ()
{
    ESP_ERROR_CHECK (esp_bt_controller_mem_release (ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t btConfig = BT_CONTROLLER_INIT_CONFIG_DEFAULT ();
    esp_err_t status                    = esp_bt_controller_init            (&btConfig);
    if (status)
    {
        LOGE (Module, "Initialize bluetooth controller. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bt_controller_enable (ESP_BT_MODE_BLE);
    if (status)
    {
        LOGE (Module, "Enable bluetooth controller. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bluedroid_init ();
    if (status)
    {
        LOGE (Module, "Init bluetooth stack. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_bluedroid_enable ();
    if (status)
    {
        LOGE (Module, "Enable bluetooth. Status: %s", esp_err_to_name (status));
        return;
    }

    status = esp_ble_gap_register_callback (GapCallback);
    if (status)
    {
        LOGE (Module, "Register GAP callback. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gattc_register_callback (GattcCallback);
    if(status){
        LOGE (Module, "Register GATTC callback. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gattc_app_register (0);
    if (status)
    {
        LOGE (Module, "Gattc app register. Status: 0x%X", status);
        return;
    }

    status = esp_ble_gatt_set_local_mtu (FIVE_HUNDRED);
    if (status)
    {
        LOGE (Module, "Set local  MTU. Status: 0x%X", status);
        return;
    }

    SetState (EState::eReceive);
}

static void IRAM_ATTR gpioIsr (void * v_arg)
{
    uint32_t pinNum = (uint32_t) v_arg;
    xQueueSendFromISR (queueHandle, &pinNum, NULL);
}

static void restartConnection (void * v_arg)
{
    uint32_t pinNum;
    while (1)
    {
        if (xQueueReceive (queueHandle, &pinNum, portMAX_DELAY))
        {
            LOG (Module, "Restart connection");
            esp_ble_gattc_cache_refresh (gGattcConfig.Profiles [ZERO].ServerMac);
        }
    }
}

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

        GpioHw      gpioHw (gpioIsr);
        BleClientHw bleClientHw;

        queueHandle = xQueueCreate (TEN, sizeof (uint32_t));
        xTaskCreate (restartConnection, "restartConnection", 2048, NULL, TEN, NULL);
    }
}
