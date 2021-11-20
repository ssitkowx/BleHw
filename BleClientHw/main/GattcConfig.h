#pragma once

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "esp_gattc_api.h"
#include "esp_gap_ble_api.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// MACROS/DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define PROFILES_NUM     ONE
#define PROFILE_A_APP_ID ZERO

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void GattcEvent (esp_gattc_cb_event_t v_event, esp_gatt_if_t v_interface, esp_ble_gattc_cb_param_t * v_params);

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class GattcConfig
{
    public:
        struct
        {
            const char * ServerName = "EvolutionBoard";

            uint16_t ProfileNum;
            uint16_t AppId;

            struct Uuid
            {
                uint16_t Service;
                uint16_t Characteristic;
            } Uuid;
        } Config;

    public:
        typedef struct
        {
            uint16_t Interface;
            uint16_t AppId;
            uint16_t ConnId;

            struct
            {
                struct
                {
                    uint16_t Start;
                    uint16_t End;
                } Service;

                uint16_t Characteristic;
            } Handle;

            esp_bd_addr_t  ServerMac;
            esp_gattc_cb_t Callback;
        } Profile;

        struct
        {
            esp_bt_uuid_t Service;
            esp_bt_uuid_t Characteristic;
            esp_bt_uuid_t Description;
        } FiltersUuid;

        esp_ble_scan_params_t ScanParams;
        Profile               Profiles [PROFILES_NUM];

        GattcConfig ()
        {
            Config.Uuid.Service                    = 0x00AA;
            Config.Uuid.Characteristic             = 0x00BB;
            Config.ProfileNum                      = PROFILES_NUM;
            Config.AppId                           = PROFILE_A_APP_ID;

            FiltersUuid.Service.len                = ESP_UUID_LEN_16;
            FiltersUuid.Service.uuid.uuid16        = Config.Uuid.Service;

            FiltersUuid.Characteristic.len         = ESP_UUID_LEN_16;
            FiltersUuid.Characteristic.uuid.uuid16 = Config.Uuid.Characteristic;

            FiltersUuid.Description.len            = ESP_UUID_LEN_16;
            FiltersUuid.Description.uuid.uuid16    = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

            ScanParams.scan_type                   = BLE_SCAN_TYPE_ACTIVE;
            ScanParams.own_addr_type               = BLE_ADDR_TYPE_PUBLIC;
            ScanParams.scan_filter_policy          = BLE_SCAN_FILTER_ALLOW_ALL;
            ScanParams.scan_interval               = 0x50;
            ScanParams.scan_window                 = 0x30;
            ScanParams.scan_duplicate              = BLE_SCAN_DUPLICATE_DISABLE;

            Profiles [Config.AppId].Callback       = GattcEvent;
            Profiles [Config.AppId].Interface      = ESP_GATT_IF_NONE;
	    }
};

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
