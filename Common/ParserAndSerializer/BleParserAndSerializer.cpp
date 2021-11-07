
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include "Utils.h"
#include "Settings.h"
#include "BleParserAndSerializer.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*
BleParserAndSerializer::BleParserAndSerializer ()
{
    jsonOptions.always_print_enums_as_ints    = true;
    jsonOptions.preserve_proto_field_names    = true;
    jsonOptions.always_print_primitive_fields = true;
}

void BleParserAndSerializer::Parse (std::string & v_jsonMsg)
{
    BleMsgType msg;
    Status status = JsonStringToMessage (v_jsonMsg, &msg);    // this returns error

    //use msg for settings
    Settings::GetInstance ().BleMsgType.Id             = msg.id             ();
    Settings::GetInstance ().BleMsgType.Name           = msg.name           ();
    Settings::GetInstance ().BleMsgType.Data           = msg.data           ();
    Settings::GetInstance ().BleMsgType.eOperatingMode = (Settings::EOperatingMode)msg.eoperatingmode ();
}

void BleParserAndSerializer::Serialize (std::string & v_jsonMsg)
{
    BleMsgType msg;
    msg.set_id             (Settings::GetInstance ().BleMsgType.Id); 
    msg.set_name           (Settings::GetInstance ().BleMsgType.Name);
    msg.set_data           (Settings::GetInstance ().BleMsgType.Data);
    msg.set_eoperatingmode ((::BleMsgType_EOperatingMode)Settings::GetInstance ().BleMsgType.eOperatingMode);
    Status status = MessageToJsonString (msg, &v_jsonMsg, jsonOptions);
}
*/

void BleParserAndSerializer::Parse (cJSON * v_root)
{
    cJSON * id = cJSON_GetObjectItem (v_root, "Id");
    isNodeJsonEmpty ("root", "Id", id);
    Settings::GetInstance ().BleMsgType.Id = id->valueint;

    cJSON * name = cJSON_GetObjectItem (v_root, "Name");
    isNodeJsonEmpty ("root", "Name", name);
    memcpy (Settings::GetInstance ().BleMsgType.Name.data (), name->valuestring, strlen (name->valuestring));

    cJSON * data = cJSON_GetObjectItem (v_root, "Data");
    isNodeJsonEmpty ("root", "Data", data);
    memcpy (Settings::GetInstance ().BleMsgType.Data.data (), data->valuestring, strlen (data->valuestring));

    cJSON * operatingMode = cJSON_GetObjectItem (v_root, "eOperatingMode");
    isNodeJsonEmpty ("root", "eOperatingMode", operatingMode);
    Settings::GetInstance ().BleMsgType.eOperatingMode = (Settings::EOperatingMode)operatingMode->valueint;
}

void BleParserAndSerializer::Serialize (cJSON * v_root)
{
    add ("Id"            , Settings::GetInstance ().BleMsgType.Id            , v_root);
    add ("Name"          , Settings::GetInstance ().BleMsgType.Name          , v_root);
    add ("Data"          , Settings::GetInstance ().BleMsgType.Data          , v_root);
    add ("eOperatingMode", Settings::GetInstance ().BleMsgType.eOperatingMode, v_root);
}
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
