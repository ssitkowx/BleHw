#pragma once

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include "Utils.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// MACROS/DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define PROFILES_NUM     ONE
#define PROFILE_A_APP_ID ZERO

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void GattsProfileAEvent (esp_gatts_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gatts_cb_param_t * v_param);

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class GattsConfig
{
    public:
        struct Config
        {
            static constexpr char * ServerName = (char *)"EvolutionBoard";

            struct
            {
                uint16_t Service;
                uint16_t Characteristic;
                uint16_t Description;
            } UUID;

            uint8_t Num;
        } Configs [PROFILES_NUM];

        struct Profile
        {
            esp_gatts_cb_t  Callback;
            uint16_t        Interface;
            esp_gatt_perm_t Perm;
            uint16_t        AppId;
            uint16_t        ConnId;

            struct
            {
                esp_gatt_srvc_id_t Id;
                uint16_t           Handle;
            } Service;

            struct
            {
                esp_bt_uuid_t        Uuid;
                uint16_t             Handle;
                esp_gatt_char_prop_t Property;
            } Charactersitic;

            struct
            {
                esp_bt_uuid_t Uuid;
                uint16_t      Handle;
            } Description;
        };

        esp_attr_value_t                       DemoChar1Val;
        esp_ble_adv_data_t                     AdvData;
        esp_ble_adv_data_t                     ScanData;
        esp_ble_adv_params_t                   AdvParams;
        esp_ble_conn_update_params_t           ConnParams;
        std::array <uint8_t, THREE>            charactersistics;
        std::array <uint8_t, THIRTY_TWO_BYTES> AdvServiceUuid128 = { 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 };

        Profile                                Profiles [PROFILES_NUM];

        GattsConfig ()
        {
            Configs [PROFILE_A_APP_ID].UUID.Service                    = 0x00AA;
            Configs [PROFILE_A_APP_ID].UUID.Characteristic             = 0x00BB;
            Configs [PROFILE_A_APP_ID].UUID.Description                = 0x00CC;
            Configs [PROFILE_A_APP_ID].Num                             = FOUR;

            charactersistics.at (ZERO)                                 = 0x0A;
            charactersistics.at (ONE)                                  = 0x0B;
            charactersistics.at (TWO)                                  = 0x0C;

            DemoChar1Val.attr_max_len                                  = 0x40,
            DemoChar1Val.attr_len                                      = charactersistics.size (),
            DemoChar1Val.attr_value                                    = charactersistics.data (),

            AdvData.set_scan_rsp                                       = false;
            AdvData.include_name                                       = true;
            AdvData.include_txpower                                    = false;
            AdvData.min_interval                                       = 0x0006;    //slave connection min interval, Time = min_interval * 1.25 msec
            AdvData.max_interval                                       = 0x0010;    //slave connection max interval, Time = max_interval * 1.25 msec
            AdvData.appearance                                         = 0x00;
            AdvData.manufacturer_len                                   = 0;
            AdvData.p_manufacturer_data                                = NULL;
            AdvData.service_data_len                                   = 0;
            AdvData.p_service_data                                     = NULL;
            AdvData.service_uuid_len                                   = AdvServiceUuid128.size ();
            AdvData.p_service_uuid                                     = AdvServiceUuid128.data ();
            AdvData.flag                                               = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

            ScanData.set_scan_rsp                                      = true;
            ScanData.include_name                                      = true;
            ScanData.include_txpower                                   = true;
            // ScanData.min_interval                                     = 0x0006;
            // ScanData.max_interval                                     = 0x0010;
            ScanData.appearance                                        = 0x00;
            ScanData.manufacturer_len                                  = 0;
            ScanData.p_manufacturer_data                               = NULL;
            ScanData.service_data_len                                  = 0;
            ScanData.p_service_data                                    = NULL;
            ScanData.service_uuid_len                                  = AdvServiceUuid128.size ();
            ScanData.p_service_uuid                                    = AdvServiceUuid128.data ();
            ScanData.flag                                              = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

            AdvParams.adv_int_min                                      = 0x20;
            AdvParams.adv_int_max                                      = 0x40;
            AdvParams.adv_type                                         = ADV_TYPE_IND;
            AdvParams.own_addr_type                                    = BLE_ADDR_TYPE_PUBLIC;
            // AdvParams.peer_addr                                       =
            // AdvParams.peer_addr_type                                  =
            AdvParams.channel_map                                      = ADV_CHNL_ALL;
            AdvParams.adv_filter_policy                                = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

            ConnParams.latency                                         = 0;
            ConnParams.max_int                                         = 0x20;    // max_int = 0x20*1.25ms = 40ms
            ConnParams.min_int                                         = 0x10;    // min_int = 0x10*1.25ms = 20ms
            ConnParams.timeout                                         = 400;     // timeout = 400*10ms = 4000ms

            Profiles [PROFILE_A_APP_ID].Callback                       = GattsProfileAEvent;
            Profiles [PROFILE_A_APP_ID].Interface                      = ESP_GATT_IF_NONE;
        }
};
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
