#pragma once 

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// INCLUDES /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "Logger.h"
#include "esp_log.h"
#include "SystemTime.h"

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// MACROS/DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define LOG(MODULE,MSG, ...)  Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eNone   , MODULE, MSG, ##__VA_ARGS__);
#define LOGE(MODULE,MSG, ...) Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eError  , MODULE, MSG, ##__VA_ARGS__);
#define LOGW(MODULE,MSG, ...) Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eWarning, MODULE, MSG, ##__VA_ARGS__);
#define LOGI(MODULE,MSG, ...) Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eInfo   , MODULE, MSG, ##__VA_ARGS__);
#define LOGD(MODULE,MSG, ...) Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eDebug  , MODULE, MSG, ##__VA_ARGS__);
#define LOGV(MODULE,MSG, ...) Logger::GetInstance ()->Log <LoggerHw> (Logger::ELogLevel::eVerbose, MODULE, MSG, ##__VA_ARGS__);

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// CLASSES/STRUCTURES ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class LoggerHw : public Logger
{
    private:
        constexpr std::string_view getFontColor (const ELogLevel v_eLogLevel)
        {
            switch (v_eLogLevel)
            {
                case ELogLevel::eNone:    { return LOG_COLOR_BLACK;  }
                case ELogLevel::eError:   { return LOG_COLOR_RED;    }
                case ELogLevel::eWarning: { return LOG_COLOR_CYAN;   }
                case ELogLevel::eInfo:    { return LOG_COLOR_BLUE;   }
                case ELogLevel::eDebug:   { return LOG_COLOR_PURPLE; }
                case ELogLevel::eVerbose: { return LOG_COLOR_GREEN;  }
                default:                  { break;                   }
            };

            return LOG_COLOR_BLACK;
        }

    public:
        template<typename... ARGS>
        constexpr void Log (const ELogLevel v_eLogLevel, std::string_view v_module, std::string_view v_msg, ARGS &&... v_args)
        {
            std::string line = Format ("\e[1;30m%s \e[1;%sm%s: %s\n", SystemTime::GetInstance ()->ToString ("%F %T").data (),
                                                                      getFontColor  (v_eLogLevel).data (),
                                                                      v_module.data (), v_msg.data ());

            esp_log_write (static_cast <esp_log_level_t> (v_eLogLevel), v_module.data (), line.data (), std::forward <ARGS>(v_args)...);
        }
};

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// END OF FILE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
