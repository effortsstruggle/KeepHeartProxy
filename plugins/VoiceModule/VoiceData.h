#pragma once
#include <stdint.h>
#include <string>
#include <android/log.h>
#pragma pack(1)



typedef struct
{
    uint8_t flag ; 
    uint8_t length; 
    
} tagPacketHead ;

typedef struct 
{
    tagPacketHead head;
    uint8_t command;
    uint8_t crc;
}tagPacketData;

typedef struct 
{
    tagPacketHead head;
    uint8_t command[2];
    uint8_t crc;
}tagPacketVer;

#pragma pack()


typedef void (*OnVoiceNotifyStr)(int code,std::string&);

#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_VoiceModule", __VA_ARGS__);