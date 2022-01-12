#pragma once

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include <stdint.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class Settings final
{
    public:
        enum ETurn
        {
            eOn,
            eOff,
            eLeft,
            eRight
        };

        enum EDrive
        {
            eForward,
            eBackward
        };

        enum EOperatingMode
        {
            eNormal = ZERO,
            eStable = ONE,
            eEmpty  = TWO,
        };

        struct
        {
            uint32_t    Id;
            std::string Name;
            std::string Data;

            EOperatingMode eOperatingMode = eNormal;
        } BleMsgType;

        struct
        {
            ETurn    eTurn;
            EDrive   eDrive;
            uint16_t Speed;
        } Wehicle;
        
        std::string Command;

        Settings () = default;

        static Settings & GetInstance (void)
        {
            static Settings instance;
            return instance;
        }
};

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
