#ifndef __SKYLOG_H
#define __SKYLOG_H

#include "bllLog.h"

#define  SKYLOGD(fmt, ...) {ecLog(BLL_LOG_MODULE_APP,BLL_LOG_LEVEL_DEBUG,"SKY_", __FUNCTION__, fmt, ##__VA_ARGS__);}
// #define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "SKY_", __VA_ARGS__);
// #define  LOGW(...) __android_log_print(ANDROID_LOG_WARN, "SKY_", __VA_ARGS__);

#endif // DEBUG

