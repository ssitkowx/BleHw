SET (PackageIncludePaths "/home/sylwester/.conan/download")

SET (PackageSourcePaths ../../External/Libs/Json/cJSON.c
                        ../../Common/Json/Json.cpp
                        ../../Common/Settings/Settings.cpp
                        #../../Common/SystemTimeHw.cpp
                        ../../Common/ParserAndSerializer/BleParserAndSerializer.cpp
                        "BleServerHw.cpp"
                        ${PackageIncludePaths}/Rtos/Project/Rtos.cpp
                        ${PackageIncludePaths}/Utils/Project/Utils.cpp
                        ${PackageIncludePaths}/Logger/Project/Logger.cpp
                        ${PackageIncludePaths}/SystemTime/Project/SystemTime.cpp
                        ${PackageIncludePaths}/Communication/Project/Communication.cpp
    )

IDF_COMPONENT_REGISTER (SRCS ${PackageSourcePaths}
                        INCLUDE_DIRS "."
                        ../../External/Libs/Json
                        ../../Common
                        ../../Common/Json
                        ../../Common/LoggerHw
                        ../../Common/Settings
                        ../../Common/ParserAndSerializer
                        ${PackageIncludePaths}/Rtos/Project
                        ${PackageIncludePaths}/Utils/Project
                        ${PackageIncludePaths}/Logger/Project
                        ${PackageIncludePaths}/SystemTime/Project
                        ${PackageIncludePaths}/Communication/Project
    )
                    
SET_PROPERTY (TARGET ${COMPONENT_TARGET} PROPERTY CXX_STANDARD 17)
SET_PROPERTY (TARGET ${COMPONENT_TARGET} PROPERTY C_STANDARD   11)
