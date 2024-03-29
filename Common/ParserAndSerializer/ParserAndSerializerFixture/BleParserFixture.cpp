///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "LoggerHw.h"
#include "Settings.h"
#include "BleParser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "BleParserFixture.h"

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*
static constexpr char * const serializedMsg = "{\"Id\":10,\"Name\":\"Sylwester\",\"Data\":\"02.11.2021, 19.19\",\"eOperatingMode\":0}";

TEST_F (BleParserAndSerializerFixture, CheckSerializedMessage)
{
    LOG (Module, "CheckSerializedMessage");

    std::string jsonMsg;
    bleParserAndSerialzier.Serialize (jsonMsg);

    LOGI        (Module, "Json msg expected: ", jsonMsg.data ());
    LOGI        (Module, "Json msg got:      ", serializedMsg);
    EXPECT_TRUE (jsonMsg == serializedMsg);
}

TEST_F (BleParserAndSerializerFixture, CheckParsedMessage)
{
    LOG (Module, "CheckParsedMessage");

    std::string jsonMsg;
    bleParserAndSerialzier.Serialize (jsonMsg);
    bleParserAndSerialzier.Parse     (jsonMsg);

    LOG       (Module, "Json msg: ", jsonMsg.data ());
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Id  , TEN);
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Name, "Sylwester");
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Data, "02.11.2021, 19.19");
}
*/

/*
static constexpr char * const bleMsgResp = "{\"Id\":10,\"Name\":\"Sylwester\",\"Data\":\"02.11.2021, 19.19\",\"eOperatingMode\":0}";

TEST_F (BleParserAndSerializerFixture, ParseMessage)
{
    LOGI (Module, "ParseMessage");

    cJSON * root = cJSON_Parse (bleMsgResp);
    bleParserAndSerialzier.Parse (root);

    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Id            , TEN);
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Name          , "Sylwester");
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.Data          , "02.11.2021, 19.19");
    EXPECT_EQ (Settings::GetInstance ().BleMsgType.eOperatingMode, Settings::eNormal);

    LOGI (Module, "Id:             ", Settings::GetInstance ().BleMsgType.Id);
    LOGI (Module, "Name:           ", Settings::GetInstance ().BleMsgType.Name);
    LOGI (Module, "Data:           ", Settings::GetInstance ().BleMsgType.Data);
    LOGI (Module, "eOperatingMode: ", Settings::GetInstance ().BleMsgType.eOperatingMode);

    cJSON_Delete (root);
}

TEST_F (BleParserAndSerializerFixture, SerializeMessage)
{
    LOG (Module, "SerializeMessage");

    cJSON * root = cJSON_CreateObject ();
    bleParserAndSerialzier.Serialize (root);

    char * jsonMessage = cJSON_Print (root);
    LOGI (Module, "Json msg: ", jsonMessage);

    free         (jsonMessage);
    cJSON_Delete (root);
}
*/

static constexpr char * const turnOn = (char *)"{\"Turn\":0}";

TEST_F (BleParserFixture, TurnOn)
{
    LOGI (Module, "TurnOn");

    Settings::GetInstance ().Wehicle.eTurn = static_cast<Settings::ETurn>(ONE);

    cJSON * root = cJSON_Parse (turnOn);
    BleParser.Turn (root, Settings::ETurn::eOn);

    EXPECT_EQ      (Settings::GetInstance ().Wehicle.eTurn, ZERO);

    LOGI           (Module, "TurnOn:", Settings::GetInstance ().Wehicle.eTurn);
    cJSON_Delete   (root);
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
