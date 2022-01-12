SET (PackageIncludePaths "/home/sylwester/.conan/download")

SET (PackageSourcePaths ../../External/Libs/Json/cJSON.c
                        ../../Common/Json/Json.cpp
                        ../../Common/GpioHw/GpioHw.cpp
                        ../../Common/Settings/Settings.cpp
                        #../../Common/SystemTimeHw/SystemTimeHw.cpp
                        ../../Common/ParserAndSerializer/BleParser.cpp
                        ../../Common/ParserAndSerializer/BleSerializer.cpp
                        Hardware/main.cpp
                        Hardware/BleHw/BleClientHw.cpp
                        Hardware/Peripheral/UartHw/Uart.cpp
                        Hardware/Peripheral/UartHw/UartHw.cpp
                        ${PackageIncludePaths}/Gpio/Project/Gpio.cpp
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
                        ../../Common/GpioHw
                        ../../Common/Settings
                        ../../Common/LoggerHw
                        #../../Common/SystemTimeHw
                        ../../Common/ParserAndSerializer
                        Hardware/BleHw
                        Hardware/Peripheral/UartHw
                        ${Protobuf_INCLUDE_DIRS}
                        ${PackageIncludePaths}/Gpio/Project
                        ${PackageIncludePaths}/Rtos/Project
                        ${PackageIncludePaths}/Utils/Project
                        ${PackageIncludePaths}/Logger/Project
                        ${PackageIncludePaths}/SystemTime/Project
                        ${PackageIncludePaths}/Communication/Project
    )
                    
SET_PROPERTY (TARGET ${COMPONENT_TARGET} PROPERTY CXX_STANDARD 20)
SET_PROPERTY (TARGET ${COMPONENT_TARGET} PROPERTY C_STANDARD   11)
