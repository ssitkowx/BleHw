PROJECT                (BleParserAndSerializer)
CMAKE_MINIMUM_REQUIRED (VERSION 3.16)

SET (ProjectName Project)
SET (downloadsPath "/home/sylwester/.conan/download")

SET (PROTOBUF_INCLUDE_PATH   ${PROJECT_SOURCE_DIR}/../External/Libs/protobuf)
SET (GOOGLEMOCK_INCLUDE_PATH ${PROTOBUF_INCLUDE_PATH}/third_party/googletest/googlemock)
SET (GOOGLETEST_INCLUDE_PATH ${PROTOBUF_INCLUDE_PATH}/third_party/googletest/googletest)

FIND_PACKAGE            (Protobuf REQUIRED)
PROTOBUF_GENERATE_CPP   (PROTO_FILES PROTO_INCLUDE_PATH MsgType/BleMsgType.proto)

SET (ProjectPathsLists  ../External/Libs/Json/
                        Json
                        MsgType
                        Settings
                        ParserAndSerializer
                        ${Protobuf_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${GOOGLEMOCK_INCLUDE_PATH}
                        ${GOOGLETEST_INCLUDE_PATH}
                        ${GOOGLEMOCK_INCLUDE_PATH}/include
                        ${GOOGLETEST_INCLUDE_PATH}/include
                        ${downloadsPath}/Utils/Project
                        ${downloadsPath}/Logger/Project
                        ${downloadsPath}/LoggerHw/Project
                        ${downloadsPath}/SystemTime/Project
    )

SET (ProjectFilesAndFolders main.cpp
                            ../External/Libs/Json/cJSON.h
                            ../External/Libs/Json/cJSON.c
                            ${PROTO_INCLUDE_PATH}
                            ${PROTO_FILES}
                            Json/Json.h
                            Json/Json.cpp
                            Settings/Settings.h
                            Settings/Settings.cpp
                            ParserAndSerializer/BleParser.h
                            ParserAndSerializer/BleParser.cpp
                            ParserAndSerializer/BleSerializer.h
                            ParserAndSerializer/BleSerializer.cpp
                            ParserAndSerializer/ParserAndSerializerFixture/BleParserFixture.h
                            ParserAndSerializer/ParserAndSerializerFixture/BleParserFixture.cpp
                            ParserAndSerializer/ParserAndSerializerFixture/BleSerializerFixture.h
                            ParserAndSerializer/ParserAndSerializerFixture/BleSerializerFixture.cpp
                            ${GOOGLEMOCK_INCLUDE_PATH}/src/gmock-all.cc
                            ${GOOGLETEST_INCLUDE_PATH}/src/gtest-all.cc
                            ${GOOGLEMOCK_INCLUDE_PATH}/src/gmock_main.cc
                            ${downloadsPath}/Utils/Project/Utils.h
                            ${downloadsPath}/Utils/Project/Utils.cpp
                            ${downloadsPath}/Logger/Project/Logger.h
                            ${downloadsPath}/Logger/Project/Logger.cpp
                            ${downloadsPath}/LoggerHw/Project/LoggerHw.h
                            ${downloadsPath}/LoggerHw/Project/LoggerHw.cpp
                            ${downloadsPath}/SystemTime/Project/SystemTime.h
                            ${downloadsPath}/SystemTime/Project/SystemTime.cpp
    )

#SOURCE_GROUP               (TREE                              ${downloadsPath}/Logger PREFIX "" FILES ${downloadsPath}/Logger/Project/Logger.h)                          
#SOURCE_GROUP               (TREE                              ${ProjectIncludePath} PREFIX "" FILES ${ProjectFilesAndFolders})
#LINK_DIRECTORIES           (${ProjectName}                    ${PackageLibsPaths})
LINK_DIRECTORIES            (${ProjectName}                    ${PackageLibsPaths})
ADD_EXECUTABLE              (${ProjectName}                    ${ProjectFilesAndFolders})
TARGET_INCLUDE_DIRECTORIES  (${ProjectName} PUBLIC             ${ProjectPathsLists} ${PackageIncludesPaths})
#TARGET_LINK_LIBRARIES      (${ProjectName}                    ${CONAN_LIBS_GTEST} ${PackageLibsNames})
TARGET_LINK_LIBRARIES       (${ProjectName}                    ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
TARGET_COMPILE_FEATURES     (${ProjectName} PUBLIC cxx_std_17)
