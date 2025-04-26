
//------------------------4754C
#include "Drv_Si4754C.h"
#include "si475x6x_common.h"
#include "si475x6x_commanddefs.h"
#include "si475x6x_propertydefs.h"
#include "si47xx_tuner_api_core.h"
#include "si47xx_tuner_api_extend.h"
#include "delay.h"
#include "si475x6x_seek.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <android/log.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "GpioLib.h"
#include "typedefs.h"


uint8_t  xdata g_tuner_type = TUNER_SI475X_SINGLE;

uint8_t xdata gTunerFWRevision;
uint8_t xdata gFmSpace, gAmSpace;

uint16_t gFmFreq;
uint16_t gAmFreq;
int8_t gFreqOffset;
uint16_t xdata gTempFreq;
uint8_t xdata gBandMode;
uint8_t xdata gFmSnr, gAmSnr;
uint8_t xdata gStatn;
uint8_t xdata gFmRssi, gAmRssi, gFmBlend, gFmValid, gAmValid;
uint8_t xdata gAmValidStatn, gFmValidStatn; 
uint8_t gStopSeeking = 0 ;  // 全局变量 ，在搜台中停止搜台 1 停止
uint8_t gSeekUp = 1;        // 全局变量 ，0， 逆向搜台 1 ,搜台为向上搜台 
uint8_t gPowerStarus = 0;   // 全局变量 ，0 未启动 1 已启动 2 状态切换中,避免开启中关闭
uint8_t i2cError = 0;
//uint16_t xdata gFmStatn[FM_STATION_MEMORY_SIZE],gAmStatn[AM_STATION_MEMORY_SIZE];

#define EFFECT_DEV "/dev/i2c-1"
uint8_t xdata si475x_chipAddress = 0x60; ///<i2c address, please ask you hardware engineer the acctual i2c address.

uint8_t xdata si475x_xcLoad; ///<xcLoad value usally is set to 0x27, user can modify it according AN543Rev0.8 8.1.12

int FmGpio = 144;//PE16=32*4+16
static int fm_fd = -1;

static const char *TAG = "- ";
char FM_ADDRESS = 0x60;


RETURN_CODE I2C1_Write(uint8_t id, uint8_t *buff, uint16_t count);
RETURN_CODE I2C1_Read(uint8_t id, uint8_t *buff, uint16_t count);

static void init_si475x6x_tuner() {
    fm_rssi_threshold = VALID_FM_RSSI_THRESHOLD;

    fm_snr_threshold = VALID_FM_SNR_THRESHOLD;

    fm_freqoff_threshold = VALID_FM_ABS_FREQOFF_THRESHOLD;

    am_rssi_threshold = VALID_AM_RSSI_THRESHOLD;

    am_snr_threshold = VALID_AM_SNR_THRESHOLD;
}

unsigned char Init_Si4754C(const char* iic_addr,uint8_t slave_addr){
    if(gPowerStarus == 1 || gPowerStarus == 2)
        return -1;

    gPowerStarus = 2;  
    i2cError = 0;
  
#ifdef LGNJ01
    system ("echo 71 > /sys/class/gpio/export");
    system ("echo out > /sys/class/gpio/gpio71/direction");
    system ("echo 1 > /sys/class/gpio/gpio71/value");
    delay_ms(100);
    system  ("echo 0 > /sys/class/gpio/gpio71/value");
    LOGD("LGNJ01 Init_Si4754C  "); 
#else
    system( "echo PE16   > /sys/kernel/debug/sunxi_pinctrl/sunxi_pin");
    system( "echo PE16 1 > /sys/kernel/debug/sunxi_pinctrl/data");
    delay_ms(100);
    system( "echo PE16 0 > /sys/kernel/debug/sunxi_pinctrl/data");
    LOGD("GLBM01 Init_Si4754C  "); 
#endif
    delay_ms(100);
    int rtc = SetGpioValue(FmGpio, 0);   // 暂时没有用
   // usleep(100);
    if (rtc != CMD_SUCCESS) {
        LOGE("SetGpioValue err");
        gPowerStarus = 0;
        return -1;
    }


    // LOGD("Open EFFECT_DEV : %s", EFFECT_DEV);
    fm_fd = open(EFFECT_DEV, O_RDWR);
    if (fm_fd < 0) {
        LOGE("EFFECT_DEV open fail ret= %d err= %d dev= %s ",  fm_fd, errno, EFFECT_DEV);
        gPowerStarus = 0;
        return -1;
    }
    unsigned char ret = 0;

    si475x_xcLoad = 0x27;
//    crystal = 0;
    if (slave_addr != 0) {
        si475x_chipAddress = slave_addr;
    } else {
        si475x_chipAddress = 0x60 ;//fmRadio default address C0
        FM_ADDRESS = 0x60;
    }

    init_si475x6x_tuner();
    if (CMD_SUCCESS != Tuner_47xx_PowerUp(BAND_FM)) {
        // printf("Tuner_47xx_PowerUp fail \n");
        LOGD( "Tuner_47xx_PowerUp  FAILED ");
        close(fm_fd);
        gPowerStarus = 0;
        return -1;
    }

    LOGD("Tuner_47xx_PowerUp  SUCCESS ");
    SetMute(1);
    gFmFreq = 8750;

    gBandMode = BAND_FM;
    g_band = BAND_FM;
    gPowerStarus = 1;
    return ret;

}
uint8_t GetStatus()
{
    return gPowerStarus;
}


uint16_t GetVolume()
{
    int res =  si475x6x_get_property(AUDIO_ANALOG_VOLUME);

  //  LOGD("GetVolume : %d",res );
    return res;
}

uint8_t SetBandMode(uint8_t mode)
{
    if(mode == BAND_FM)
        SwitchBand(BAND_FM);
    else 
        SwitchBand(BAND_AM);
    return 0;
}
uint8_t GetBandMode()
{
   return g_band;
}

void SetVolume(uint8_t value) {
    Tuner_47xx_SetVolume(value);
}

void SetMute(uint8_t value) {
    LOGD ("D_SetMute %d",value);
    si475x6x_mute(value);
}


uint16_t GetMute(){
    return si475x6x_get_property(AUDIO_MUTE) == 3?1:0;
}

int  PowerUp(){
   return  Init_Si4754C("",0);
   
}

void Powerdown(void) {
    i2cError = 0;
    if(fm_fd  <0 || gPowerStarus == 2 || gPowerStarus ==0)
        return;
        
    StopSeeking();
    gPowerStarus = 2 ;
    si475x_powerdown();
    
    close(fm_fd);
    fm_fd = -1;

    //LOGD("Powerdown SetGpioValue err  rtc=%d", rtc);
    gPowerStarus = 0;
}

void SetStereoMono(uint8_t mode) {
    SwitchStereoMono(mode);
}

uint16_t GetFreq() {
    return GetFreqence(gBandMode);
}

uint8_t FmValidate(uint16_t freq) {
    return gBandMode == BAND_FM ? fm_validate(freq) : am_validate(freq);
}


uint8_t Tuner_Tune(uint16_t freq) {
    uint8_t ret = HAL_ERROR;
    if ((freq >= AM_BOT_FREQ) && (freq <= AM_TOP_FREQ)) {
        if (g_band != BAND_AM)
            SwitchBand(BAND_AM);

        gAmFreq = freq;
        ret = Tuner_47xx_Tune(freq);
    } else if ((freq >= FM_BOT_FREQ) && (freq <= FM_TOP_FREQ)) {
        if (g_band != BAND_FM)
            SwitchBand(BAND_FM);

        gFmFreq = freq;
        ret = Tuner_47xx_Tune(freq);
    }

    gBandMode = g_band;

    return ret;
}

void SetSeekUp(uint8_t up)
{
    gSeekUp = up;
}

void SetSeekSpace(uint8_t val)
{
    SetFreqSeekSpace(val);
}


void StopSeeking()
{
    gStopSeeking = 1;
}

int GetI2cError()
{
    return i2cError;
}
void AutoSeek(seek_process_callback process_freq, seek_found_callback found_freq)
{
    gStopSeeking = 0;

     switch (g_band) {
        case BAND_FM:
            si475x_fm_autoseek(process_freq, found_freq);
            break;

        case BAND_AM:
            si475x_am_autoseek(process_freq, found_freq);
            break;

        }
}
RETURN_CODE I2C1_Write(uint8_t addr, uint8_t *data, uint16_t count) {
    
    if(fm_fd < 0)
    {
        LOGD("open %s  faild fm_fd = -1", EFFECT_DEV);
        return -1;
    }
    addr = FM_ADDRESS;

    struct i2c_rdwr_ioctl_data rwdata;
    struct i2c_msg msg;

    ioctl(fm_fd, I2C_TIMEOUT, 1);
    ioctl(fm_fd, I2C_RETRIES, 2);

    msg.addr = addr;
    msg.flags = 0; //0=write,, read-I2C_M_RD
    msg.len = count;
    msg.buf = data;

    rwdata.msgs = &msg;
    rwdata.nmsgs = 1;
    if (ioctl(fm_fd, I2C_RDWR, &rwdata) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "write ioctl err=%d ,%s", errno,strerror(errno));
        i2cError = 0;
        gStopSeeking = 1;
        return -1;
    }
    usleep(500);
 //   __android_log_print(ANDROID_LOG_DEBUG, TAG, "I2C1_Write  CMD_SUCCESS");
    return CMD_SUCCESS;
}

//S, id, address, delay, S, id, *buf, NACK,*last, P
RETURN_CODE I2C1_Read(uint8_t addr, uint8_t *buff, uint16_t count) {
    if(fm_fd < 0)
    {
        LOGD("open %s  faild fm_fd = -1", EFFECT_DEV);
        return -1;
    }
    addr = FM_ADDRESS;

    struct i2c_rdwr_ioctl_data rwdata;
    struct i2c_msg msg;

    ioctl(fm_fd, I2C_TIMEOUT, 1);
    ioctl(fm_fd, I2C_RETRIES, 2);

    msg.addr = addr;
    msg.flags = I2C_M_RD;
    msg.len = count;
    msg.buf = buff;

    rwdata.msgs = &msg;
    rwdata.nmsgs = 1;
    if (ioctl(fm_fd, I2C_RDWR, &rwdata) < 0) {
       
        LOGD("Read ioctl err=%d,%s ", errno,strerror(errno));
        i2cError = 1;
        gStopSeeking = 1;
        return -1;
    }
    usleep(500);

//    __android_log_print(ANDROID_LOG_ERROR, TAG, "I2C1_Read  CMD_SUCCESS");

//     for (int i = 0; i < count; ++i) {
//         __android_log_print(ANDROID_LOG_DEBUG, TAG, "I2C1_Read addr= %x count= %d  data[%d]=%x ",
//                             addr, count, i, buff[i]);
//     }

    return CMD_SUCCESS;
}


