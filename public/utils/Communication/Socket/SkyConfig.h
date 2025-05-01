#ifndef _CONFIG_H
#define _CONFIG_H
#include "functor.h"
// #include <android/log.h>
//用于控制输出的

#define _DEBUG
#ifdef _DEBUG
// #define TRACE(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_SOCKET_IPC", __VA_ARGS__);
#else
#define TRACE(...)
#endif   

#define DEFAULTPORT 4445    //音乐默认端口 
#define BACKLOG 2
#define BUF_SIZE 200
#define Reconnection_Interval 1000 // 重联间隔

typedef int skinfo; //socket 描述符 ，使用别名方便以后扩展更改类型
typedef Functor2<skinfo, char*> FunCallback;
typedef Functor1<char*> Fun1char;

#endif
