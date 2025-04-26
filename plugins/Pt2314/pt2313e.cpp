/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-24     KBT61       the first version
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "pt2313e.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "log.h"


typedef unsigned char uint8_t;

const int PT2313_ADDR = 0x44;

const uint8_t CHANNEL_0 = 0x00;
const uint8_t CHANNEL_1 = 0x01;
const uint8_t CHANNEL_2 = 0x02;
//const uint8_t CHANNEL_3 = 0x03;

const uint8_t COMMAND_VOL_CTRL = 0x00;     // Master Volume (0..63, 63 = Mute)
const uint8_t COMMAND_L_ATTENUATOR = 0xC0; // L attenu      (0..31)
const uint8_t COMMAND_R_ATTENUATOR = 0xE0; // R attenu      (0..31)
const uint8_t  INPUT_GAIN_0 = 0x58; // Audio switch  (0..3) // 输入0增益
const uint8_t  INPUT_GAIN_1 = 0x50; // Audio switch  (0..3) // 输入 3.75 增益
const uint8_t  INPUT_GAIN_2 = 0x48; // Audio switch  (0..3)  58(  // 增益7.5
const uint8_t  INPUT_GAIN_3 = 0x40; // Audio switch  (0..3)  58(  // 增益11.75
const uint8_t COMMAND_BASS_CONTROL = 0x60; // Bass          (0..31, None = 15)
const uint8_t COMMAND_TREBLE_CONTROL = 0x70; // Treble        (0..31, None = 15)

static uint8_t INPUT_GAIN = INPUT_GAIN_2;// 输入增益
// 通道
const uint8_t PT2313_SOURCE_MIN = 1;
const uint8_t PT2313_SOURCE_MAX = 3;
//音量
const uint8_t PT2313_VOLUME_MIN = 0;
const uint8_t PT2313_VOLUME_MAX = 0x63;

const int PT2313_BASS_MIN = -7;
const int PT2313_BASS_MAX = 7;
const int PT2313_BASS_CMD_MIN = 0;
const int PT2313_BASS_CMD_MID = 7;
const int PT2313_BASS_CMD_MAX = 15;
const int PT2313_TREBLE_MIN = -7;
const int PT2313_TREBLE_MAX = 7;
const int PT2313_TREBLE_CMD_MIN = 0;
const int PT2313_TREBLE_CMD_MID = 7;
const int PT2313_TREBLE_CMD_MAX = 15;
const int PT2313_BALANCE_MIN = -31;
const int PT2313_BALANCE_MAX = 31;

static const char TAG[] = "SoundEffects- ";

struct pt2313_device {
    int fd;
    int is_ready;
};

#define PT2313_I2C_BUS     "/dev/i2c-1"      /* i2c linked */

static struct pt2313_device pt2313_dev{-1,0};


/**
 * @brief constrain
 *
 * @param x
 * @param a
 * @param b
 * @return int
 */
static int constrain(int x, int a, int b) {
    if (x < a)return a;
    else if (x > b)return b;
    else return x;
}

/**
 * @brief map
 *
 * @param x
 * @param in_min
 * @param in_max
 * @param out_min
 * @param out_max
 * @return long
 */
static long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief abs_x
 *
 * @param a
 * @return int
 */
static int abs_x(int a) {
    if (a > 0)return a;
    else return -a;
}

/**
 * @brief pt2313_reg_write
 *
 * @param reg
 * @param data
 * @param data_size
 * @return int
 */

int pt2313_reg_write(uint8_t reg, uint8_t data) {
  //  reg = 0x44;
    uint8_t buf[1];

    if (pt2313_dev.is_ready == 0) {
        LOGD("pt2313 is no ready\n");
        return -1;
    }
    struct i2c_rdwr_ioctl_data rwdata;
    struct i2c_msg msg;

    ioctl(pt2313_dev.fd, I2C_TIMEOUT, 1);
    ioctl(pt2313_dev.fd, I2C_RETRIES, 2);

    buf[0] = reg | data;
    msg.addr = PT2313_ADDR;
    msg.flags = 0; //write,, read-I2C_M_RD
    msg.len = 1;
    msg.buf = buf;
 //   __android_log_print(ANDROID_LOG_ERROR, TAG, "write ioctl buf=%x ", buf[0]);

    rwdata.msgs = &msg;
    rwdata.nmsgs = 1;
    if (ioctl(pt2313_dev.fd, I2C_RDWR, &rwdata) < 0) {
        LOGD("write ioctl err=%d ", errno);
        pt2313_dev.is_ready = 0;
        close(pt2313_dev.fd);
        return -1;
    }
 //   printf(" TAG, write ioctl Success");
    usleep(500);

    return 0;
}

/**
 * @brief pt2313_reg_write_data
 *
 * @param source
 */
int pt2313_reg_write_data(int reg, int data) {
    return pt2313_reg_write(reg, data);
}


/**
 * @brief 设置输入增益
 *
 * @param input
 */
int pt2313_set_input_gain(uint8_t input) {
    
    switch (input)
    {
    case 0:
        INPUT_GAIN = INPUT_GAIN_0 ;
        break;
    case 1:
        INPUT_GAIN = INPUT_GAIN_1 ;
        break;
    case 2:
        INPUT_GAIN = INPUT_GAIN_2 ;
        break;
    case 3:
        INPUT_GAIN = INPUT_GAIN_3 ;
        break;
    default:
        break;
    }
     
    //usleep(50000); // 10 ms
    return 0;
}


/**
 * @brief pt2313_set_source 设置通道
 *
 * @param source
 */
int pt2313_set_source(uint8_t source) {
    uint8_t value = constrain(source, PT2313_SOURCE_MIN, PT2313_SOURCE_MAX) -1;
    value =  pt2313_reg_write(INPUT_GAIN, value);
    //usleep(50000); // 10 ms
    return value;
}

/**
 * @brief pt2313_set_volume
 *
 * @param vol
 */
int pt2313_set_volume(uint8_t vol) {
    uint8_t value = 77 - constrain(vol, PT2313_VOLUME_MIN, PT2313_VOLUME_MAX);
    
    uint8_t val_B = value/10;
    uint8_t val_A = value%10;
    value = 0;
    value |= val_B << 3;
    value |= val_A;
   
 //  printf("pt2313_set_volume: %d , val_B %d , val_A ,%d \n ",value,val_B,val_A);
     value = pt2313_reg_write(COMMAND_VOL_CTRL, value);
    usleep(200);
    return value;
}

/**
 * @brief pt2313_set_bass
 *
 * @param bass
 */
int pt2313_set_bass(int bass) {
    bass = constrain(bass, PT2313_BASS_MIN, PT2313_BASS_MAX);

    uint8_t value = (bass < 0)
                    ? map(bass, PT2313_BASS_MIN, 0, PT2313_BASS_CMD_MIN, PT2313_BASS_CMD_MID)
                    : map(bass, 0, PT2313_BASS_MAX, PT2313_BASS_CMD_MAX, PT2313_BASS_CMD_MID + 1);

    return pt2313_reg_write(COMMAND_BASS_CONTROL, value);

}

/**
 * @brief pt2313_set_treble
 *
 * @param treble
 */
int pt2313_set_treble(int treble) {
    treble = constrain(treble, PT2313_TREBLE_MIN, PT2313_TREBLE_MAX);
    uint8_t value = (treble < 0)
                    ? map(treble, PT2313_TREBLE_MIN, 0, PT2313_TREBLE_CMD_MIN,
                          PT2313_TREBLE_CMD_MID)
                    : map(treble, 0, PT2313_TREBLE_MAX, PT2313_TREBLE_CMD_MAX,
                          PT2313_TREBLE_CMD_MID + 1);

    return pt2313_reg_write(COMMAND_TREBLE_CONTROL, value);
}

/**
 * @brief pt2313_set_balance
 *
 * @param balance
 */
int pt2313_set_balance(int balance) {
    balance = constrain(balance, PT2313_BALANCE_MIN, PT2313_BALANCE_MAX);

    if (balance == 0) {
        uint8_t value = 0;
        pt2313_reg_write(COMMAND_L_ATTENUATOR, value);
        pt2313_reg_write(COMMAND_R_ATTENUATOR, value);
    } else {
        if (balance < 0) {
            uint8_t value = 0;
            pt2313_reg_write(COMMAND_L_ATTENUATOR, value);
            value = (uint8_t) abs_x(balance);
            pt2313_reg_write(COMMAND_R_ATTENUATOR, value);
        } else {
            uint8_t value = 0;
            value = (uint8_t) abs_x(balance);
            pt2313_reg_write(COMMAND_L_ATTENUATOR, value);
            value = 0;
            pt2313_reg_write(COMMAND_R_ATTENUATOR, value);
        }
    }

    return 0;
}

/**
 * @brief pt2313_set_mute
 *
 * @param enable
 * @return int
 */
int pt2313_set_mute(int enable) {
    int ret = -1;
    if (enable) {
        ret = pt2313_reg_write(COMMAND_L_ATTENUATOR, 0x1F);
        ret = pt2313_reg_write(COMMAND_R_ATTENUATOR, 0x1F);
    } else {
        ret = pt2313_reg_write(COMMAND_L_ATTENUATOR, 0x00);
        ret = pt2313_reg_write(COMMAND_R_ATTENUATOR, 0x00);
    }
   // usleep(1000 * 100);
    return ret;
}

/**
 * @brief pt2313_device_init
 *
 * @return int
 */
int pt2313_device_init(void) {
    if(pt2313_dev.is_ready)
        return 0;
    pt2313_dev.is_ready = 0;
    pt2313_dev.fd = open(PT2313_I2C_BUS, O_RDWR);
    if (pt2313_dev.fd < 0) {
        LOGD("%s open fail\n",PT2313_I2C_BUS);
        return -1;
    }

   
    // 2. do ioctl
    if (ioctl(pt2313_dev.fd, I2C_SLAVE, PT2313_ADDR) < 0) {
        LOGD("pt2313 set slave address error\n");
        close(pt2313_dev.fd);
        return -1;
    }

    pt2313_dev.is_ready = 1;
    
    pt2313_reg_write(0, 0x80);
    pt2313_reg_write(0, 0xc0);
    pt2313_reg_write(0, 0xa0);
    pt2313_reg_write(0, 0xe0);
    pt2313_set_input_gain(0);

    LOGD("pt2313_device_init success ");

    return 0;
}

/**
 * @brief pt2313_device_close
 *
 * @return int
 */
int pt2313_device_close(void) {
    if (pt2313_dev.fd) {
        close(pt2313_dev.fd);
        pt2313_dev.is_ready = 0;
    }
    return 0;
}
int pt2313_opened()
{
    return pt2313_dev.is_ready;
}

 
void usage() {
    printf("Usage:\n");
    printf("\n\
        [-m  0/1]     0:set mute,1:set unmute  \n\
        [-v  0~63]    set volume  \n\
        [-c  1/2/3/]  select cahnnel \n");
}
 
int main1(int argc, char *argv[]) {


    int o;
    const char *optstring = "m:v:c:"; // 有三个选项-abc，其中c选项后有两个冒号，表示后面可选参数
    if(argc == 1){
        usage();
        return 0;
    }
        
    int ret = 0;
    pt2313_device_init();
    while ((o = getopt(argc, argv, optstring)) != -1) {
        int val = atoi(optarg);
        switch (o) {
            case 'm':
                printf("opt is m, oprarg is: %s\n", optarg);
                ret = pt2313_set_mute(val);
                if(ret != 0){
                    printf("pt2313_set_mute faild");
                }

                break;
            case 'v':
                printf("opt is v, oprarg is: %s\n", optarg);
                pt2313_set_volume(val);
                if(ret != 0){
                    printf("pt2313_set_volume faild");
                }
                break;
            case 'c':
                printf("opt is c, oprarg is: %s\n", optarg);
                pt2313_set_source(val);
                if(ret != 0){
                    printf("pt2313_set_source faild");
                }
                break;
            case '?':
                printf("unknow oprarg \n");
                usage(); // 提示使用说明
                break;
            default:
                 printf("opt is , oprarg is: %c\n", o);
        }
    }

    printf("end :\n");
    return 0;
}
