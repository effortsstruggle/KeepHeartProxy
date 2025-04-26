/***************************************************************************************
                  Silicon Laboratories Broadcast Digital Radio SDK

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   MMI specific type definitions
   FILE: typedefs.h
   Supported IC : Si468x
   Date: June 20 2014
  (C) Copyright 2014, Silicon Laboratories, Inc. All rights reserved.
****************************************************************************************/

#ifndef _typed_h_
#define _typed_h_
#include <android/log.h>
typedef unsigned char uint8_t;
typedef unsigned char U8;
typedef unsigned char u8;

typedef unsigned short uint16_t;
typedef unsigned short int U16;
typedef unsigned short int  u16;

typedef unsigned int uint32_t;
typedef unsigned int U32;
typedef  unsigned int u32;

typedef signed char int8_t;
typedef signed char I8;
typedef signed char i8;

typedef signed short int16_t;
typedef signed short I16;
typedef signed short i16;

//typedef unsigned int bool;
typedef unsigned int Bool;

typedef signed int int32_t;
typedef signed int I32;
typedef signed int i32;

typedef signed char S8;
typedef signed short int S16;
typedef signed int S32;
typedef unsigned long long int U64;
typedef signed long long int S64;

typedef unsigned char uchar;


#define false 0
#define true  1

#define MAX_MESSAGE_LENGTH   (256)//128//64 


#define CMD_SUCCESS	0x00
#define HAL_ERROR		0x01
#define INVALID_INPUT	0x02
#define INVALID_MODE	0x04
#define TIMEOUT			0x08
#define COMMAND_ERROR	0x20
#define UNSUPPORTED_FUNCTION	0x80

typedef unsigned char  RETURN_CODE;

#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_RD", __VA_ARGS__);
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "SKY_RD", __VA_ARGS__);
#define  LOGW(...) __android_log_print(ANDROID_LOG_WARN,  "SKY_RD", __VA_ARGS__);

#endif

