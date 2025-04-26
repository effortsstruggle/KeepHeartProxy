/* 包含的头文件 */
#include <stdio.h>        //标准输入输出,如printf、scanf以及文件操作
#include <stdlib.h>        //标准库头文件，定义了五种类型、一些宏和通用工具函数
#include <unistd.h>        //定义 read write close lseek 等Unix标准函数
#include <sys/types.h>    //定义数据类型，如 ssiz e_t off_t 等
#include <sys/stat.h>    //文件状态
#include <fcntl.h>        //文件控制定义
#include <termios.h>    //终端I/O
#include <errno.h>        //与全局变量 errno 相关的定义
#include <getopt.h>        //处理命令行参数
#include <string.h>        //字符串操作
#include <time.h>        //时间
#include <sys/select.h>    //select函数
#include "GpioLib.h"
#include <android/log.h>
#include <errno.h>

static const char *TAG = "Yps_Gpio------- ";


const char *SYS_GPIO_PATH = "/sys/class/gpio";

const short GPIO_BUF = 64;
#define NO_USE_GPIO

/*
将GPIO 导入到用户空间 Export GPIO to User Space
*/

int ExportGpioToUserSpace(int gpio) {
// #ifdef NO_USE_GPIO
//  return 0;
// #endif
   
//     int ret = -1; 
//     int fd;
//     int len;
//     char value[GPIO_BUF];
//     char export_name[GPIO_BUF];
//     char gpio_name[GPIO_BUF];
//     __android_log_print(ANDROID_LOG_DEBUG, TAG, "ExportGpioToUserSpace: gpio= %d  ", gpio);
//     sprintf(export_name, "%s/export", SYS_GPIO_PATH);
//     puts(export_name);

//     fd = open(export_name, O_WRONLY);
//     if (fd < 0) {
//         __android_log_print(ANDROID_LOG_DEBUG, TAG,
//                             "ExportGpioToUserSpace:  err!gpio= %d error= %d", gpio, errno);
//         return -1;
//     }
//     sprintf(gpio_name, "%s/gpio%d", SYS_GPIO_PATH, gpio);
//     if (!access(gpio_name, 0)) {
//         return -1;
//     }
//     len = snprintf(value, sizeof(value), "%d", gpio);
//     ret = write(fd, value, len);
//     if (ret < 0) {
//         __android_log_print(ANDROID_LOG_DEBUG, TAG,
//                             "ExportGpioToUserSpace:export write err! error= %d", gpio, errno);
//         return -1;
//     }
//     close(fd);
    return 0;
}

int UnexportFromUserSpace(int gpio) {
    #ifdef NO_USE_GPIO
 return 0;
#endif
    int ret;
    int fd;
    int len;
    char value[GPIO_BUF];
    char unexport_name[GPIO_BUF];
    sprintf(unexport_name, "%s/unexport", SYS_GPIO_PATH);
    puts(unexport_name);

    fd = open(unexport_name, O_WRONLY);
    if (fd < 0) {
        printf("open unexport gpio err!");
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "UnexportFromUserSpace:open unexport gpio err! error= %d", gpio, errno);
        return -1;
    }


    len = snprintf(value, sizeof(value), "%d", gpio);
    ret = write(fd, value, len);

    if (ret < 0) {
        // __android_log_print(ANDROID_LOG_DEBUG, TAG,
        //                     "UnexportFromUserSpace:unexport write err! error= %d", gpio, errno);
        return -1;
    }

    fsync(fd);
    close(fd);
    return 0;
}

int SetGPIODirection(int gpio, int dir) {
    #ifdef NO_USE_GPIO
 return 0;
#endif
    int ret = -1;
    int fd;
    char direction_name[GPIO_BUF];

    snprintf(direction_name, sizeof(direction_name), "%s/gpio%d/direction", SYS_GPIO_PATH, gpio);
    puts(direction_name);

    fd = open(direction_name, O_WRONLY);
    if (fd < 0) {
        printf("open gpio dir err!");
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "SetGPIODirection:open gpio dir err! gpio= %d  dir =%d  errpr =%d",
                            gpio, dir, errno);
        return -1;
    }

    if (dir) {
        ret = write(fd, "out", 4);
    } else {
        ret = write(fd, "in", 3);
    }

    if (ret < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "SetGPIODirection:write err!! gpio= %d  dir =%d  err=%d ", gpio, dir,
                            errno);
        return -1;
    }

    close(fd);
    return 0;
}

int SetGpioValue(int gpio, int value) {
    #ifdef NO_USE_GPIO
 return 0;
#endif
    int ret = -1;
    int fd;
    char value_name[GPIO_BUF];

    snprintf(value_name, sizeof(value_name), "%s/gpio%d/value", SYS_GPIO_PATH, gpio);
    puts(value_name);
    __android_log_print(ANDROID_LOG_DEBUG, TAG,
                        "SetGpioValue:  value_name=%s ", value_name);
    fd = open(value_name, O_WRONLY);

    if (fd < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "SetGpioValue:write err!! gpio= %d  value =%d  err=%d ", gpio, value,
                            errno);
        return -1;
    }
    if (value) {
        ret = write(fd, "1", 2);
    } else {
        ret = write(fd, "0", 2);
    }
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "SetGpioValue write err! gpio= %d  value =%d  err=%d ", gpio, value,
                            errno);
        return -1;
    }
    close(fd);
    return 0;
}

/*
int *value : 存储返回的值
*/
int GetGpioValue(int gpio, int *value) {
//     #ifdef NO_USE_GPIO
//  return 0;
// #endif
//     int ret = -1;
//     int fd;
//     char dat;
//     char value_name[GPIO_BUF];

//     sprintf(value_name, "%s/gpio%d/value", SYS_GPIO_PATH, gpio);
//     puts(value_name);

//     fd = open(value_name, O_RDONLY);
//     if (fd < 0) {
//         __android_log_print(ANDROID_LOG_DEBUG, TAG,
//                             "GetGpioValue write err!  gpio=%d  val=%d ", gpio, value);
//         printf("open gpio value err!");
//         return -1;
//     }

//     ret = read(fd, &dat, 1);
//     if (ret < 0) {
//         __android_log_print(ANDROID_LOG_DEBUG, TAG,
//                             "GetGpioValue write err!  gpio=%d  val=%d ", gpio, value);
//         return -1;
//     }
//     if (dat != '0') {
//         *value = 1;
//     } else {
//         *value = 0;
//     }
//     close(fd);
    return 0;
}
