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
#include "bd37033.h"
#include "dap.h"
#include <errno.h>
#include <android/log.h>
#include "log.h"
const int   BD37033_ADDR               = 0x40;

const uint8_t  CHANNEL_0               = 0x00;  // BT
const uint8_t  CHANNEL_1               = 0x01;  // RADIO
const uint8_t  CHANNEL_2               = 0x02;  // 
const uint8_t  CHANNEL_3               = 0x03;  // SOC
const uint8_t  CHANNEL_4               = 0x04;  // CSK4002


const uint8_t  COMMAND_INPUT_GAIN      = 0x06; // 输入增益和静音
const uint8_t  COMMAND_VOL_CTRL        = 0x20; // Master Volume (0..63, 63 = Mute)
const uint8_t  COMMAND_LF_ATTENUATOR   = 0x28; // LF attenu      (0..31)
const uint8_t  COMMAND_RF_ATTENUATOR   = 0x29; // RF attenu      (0..31)
const uint8_t  COMMAND_LR_ATTENUATOR   = 0x2A; // LR attenu      (0..31)
const uint8_t  COMMAND_RR_ATTENUATOR   = 0x2B; // RR attenu      (0..31)
const uint8_t  COMMAND_AUDIO_SWITCH    = 0x05; // Audio switch  (0..3)
const uint8_t  COMMAND_BASS_CONTROL    = 0x51; // Bass          (0..15, None = 15)
const uint8_t  COMMAND_TREBLE_CONTROL  = 0x57; // Treble        (0..15, None = 15)
const uint8_t  COMMAND_MID_CONTROL     = 0x54; // mid

const uint8_t  COMMAND_LODNESS_GAIN    = 0x75;
 

struct bd37033_device
{
    int fd;
    int is_ready;
};

#define BD37033_I2C_BUS     "/dev/i2c-1"      /* i2c linked */

static struct bd37033_device  bd37033_dev{-1,0};

static uint8_t s_source = 0;
static uint8_t g_volume = 0;   // 记录的是人类可识别的音量

// 0x84 蓝牙的input gain
// 0x86 soc的input gain
static uint8_t s_lf_att = 0x71;// 0;
static uint8_t s_rf_att = 0x71;// 0;
static uint8_t s_lr_att = 0x71;// 0;
static uint8_t s_rr_att = 0x71;// 0;

static uint8_t s_bass   = 0;
static uint8_t s_treble = 0;
static uint8_t s_loudness = 0;
static uint8_t s_gain     = 0;


uint8_t i2c_read_data(unsigned char addr) ;
int bd37033_reg_write(uint8_t reg, uint8_t data);
/**
 * @brief bd37033_device_init
 * 
 * @return int 
 */
int bd37033_device_init(void)
{
	if(bd37033_dev.is_ready == 1)
		return 0;
	bd37033_dev.is_ready = 0;
	bd37033_dev.fd = open(BD37033_I2C_BUS, O_RDWR);
	if (bd37033_dev.fd < 0) {
		LOGD("bd37033 i2c-1 [%d] open fail\n",BD37033_I2C_BUS);
		return -1;
	}

	  // 2. do ioctl
    if (ioctl(bd37033_dev.fd, I2C_SLAVE, BD37033_ADDR) < 0) {
        LOGD("pt2313 set slave address error\n");
        close(bd37033_dev.fd);
        return -1;
    }

    bd37033_dev.is_ready = 1;
    
	bd37033_reg_write(0x01, 0xB1);
	bd37033_reg_write(0x02, 0x07);
	bd37033_reg_write(0x03, 0xC6); 
	bd37033_set_gain(0);
	bd37033_set_volume(80);  


	bd37033_set_source(0);
    bd37033_set_loudness(0xf);
    
    bd37033_set_bass(0);
    bd37033_set_treble(0);

	bd37033_set_mute(1);
	LOGD("bd37033_device_init  success \n");
    return 0;
}

/**
 * @brief bd37033_device_close
 * 
 * @return int 
 */
int bd37033_device_close(void)
{
	if(bd37033_dev.fd ){
		close(bd37033_dev.fd );
		bd37033_dev.is_ready = 0;
	}
	return 1;
}
int bd37033_opened()
{
	return bd37033_dev.is_ready ;
}


/**
 * @brief bd37033_reg_write
 * 
 * @param reg 
 * @param data 
 * @param data_size 
 * @return int 
 */
int bd37033_reg_write(uint8_t reg, uint8_t data)
{
	uint8_t buf[2];
		
	if(bd37033_dev.is_ready == 0){
        LOGD("bd37033 is no ready\n");
		return -1;
	}

#if 0	
    if (ioctl(bd37033_dev.fd, I2C_SLAVE, BD37033_ADDR) < 0) {
		printf("bd37033 set slave address error\n");
		return -1;
	}
	
	buf[0] = reg | data;
	//buf[1] = data[0];	
	if (write(bd37033_dev.fd, buf, 1) != 1) {
    	printf("i2c bus write failed!\r\n");
    	return -1;
  	}
#endif

    struct i2c_rdwr_ioctl_data rwdata;
    struct i2c_msg msg;

    ioctl(bd37033_dev.fd, I2C_TIMEOUT, 1);
    ioctl(bd37033_dev.fd, I2C_RETRIES, 2);
    
    buf[0] = reg;
	
	buf[1] = data;

    msg.addr = BD37033_ADDR;
    msg.flags = 0; //write,, read-I2C_M_RD
    msg.len = 2;
    msg.buf = buf,

    rwdata.msgs = &msg;
    rwdata.nmsgs = 1;
    if(ioctl(bd37033_dev.fd, I2C_RDWR, &rwdata) < 0){
		bd37033_dev.is_ready = 0;
        printf("bd37033 write ioctl err\n");
        return -1;
    }
    usleep(500);

    return 0;
}
/**
 *@brief: 读不出来数据，用i2cget 也读不出来，应该是不支持
 *@version 
*/
uint8_t i2c_read_data(unsigned char addr) {
     if(bd37033_dev.fd < 0)
        return 0;
    struct i2c_rdwr_ioctl_data rwdata;

    ioctl(bd37033_dev.fd, I2C_TIMEOUT, 1);
    ioctl(bd37033_dev.fd, I2C_RETRIES, 2);

    struct i2c_msg msgs[2];
    unsigned char buf1[1] = {0};
	//先设定一下地址
    msgs[0].addr =  BD37033_ADDR;
	msgs[0].len = 1;
	msgs[0].flags = 0;//write
	msgs[0].buf = buf1;
	msgs[0].buf[0] = addr;
	//然后从刚才设定的地址处读


    unsigned char buf[1] = {0};
	msgs[1].addr = BD37033_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 1;
	msgs[1].buf  = buf;
	
    rwdata.nmsgs  = 2;
	rwdata.msgs  = msgs;

	int ret = ioctl(bd37033_dev.fd, I2C_RDWR, (unsigned long)&rwdata);
    usleep(500);
	if(ret < 0){
		printf("error during I2C_RDWR ioctl with error code %0X \n", ret);
        return -1;
    }
   
  
	printf("i2c_read_data %0X \n",buf[0]);
    return buf[0];
 
}

/**
 * @brief bd37033_set_source
 * 
 * @param source 
 */
int bd37033_set_source(uint8_t source)
{

    uint8_t value = source & 0x1f;

    //printf("bd37033 set source %d\n", source);

	uint8_t fadergain  = 0x80;
	uint8_t inputgain = 0;
	uint8_t volume = g_volume;
	LOGD("HEX select channel  = %d ,inputgain =  %#x ,fadergain =  %#x volume =  %#x",value,inputgain,fadergain, volume);
	LOGD("select channel  = %d ,inputgain =  %d ,g_volume =  %d volume =  %d",value,inputgain,g_volume, volume);
	switch ( source)
	{
	case 0:  // 蓝牙
		fadergain = 0x85;
		break;
	case 1:  // 收音机
		inputgain = 0x9;
		fadergain = 0x71;
		break;
	case 3: // soc
		fadergain = 0x85;
		break;
	case 8: // 语音模块
		fadergain = 128 + 80 - g_volume; //fadergain = 0x76;
		volume = 70;
		break;
	default:
		fadergain = 0x88;
		break;
	}

	LOGD("HEX select channel  = %d ,inputgain =  %#x ,fadergain =  %#x volume =  %#x",value,inputgain,fadergain, volume);
	LOGD("select channel  = %d ,inputgain =  %d ,g_volume =  %d volume =  %d",value,inputgain,g_volume, volume);
	
	bd37033_set_fader(fadergain);
	bd37033_set_gain(inputgain);

    uint8_t tmpVol = 128 + 80 - volume;   //128+80 128-15 : 0-95
    
    bd37033_reg_write(COMMAND_VOL_CTRL, tmpVol);  // 设置音量

    bd37033_reg_write(COMMAND_AUDIO_SWITCH, value); // 切换声道

	return 0;
}

int bd37033_set_loudness(uint8_t val)
{
	//printf("bd37033 set loudness %d\n", enable);
	val = val & 0x1f;
	bd37033_reg_write(COMMAND_LODNESS_GAIN, val);
	return 0;
}

int bd37033_set_gain(uint8_t gain)
{
	s_gain = gain & 0x1f;
	
	bd37033_reg_write(COMMAND_INPUT_GAIN, s_gain);
	
	return 0;
}

/**
 * @brief bd37033_set_volume
 * 
 * @param vol 
 */
int bd37033_set_volume(uint8_t vol)
{

	if(vol > 94)
		vol = 94;
	g_volume = vol;
    uint8_t value = 128 + 80 - vol;   //128+80 128-15 : 0-95
    

    //printf("bd37033 set volume %d\n", vol);
    return bd37033_reg_write(COMMAND_VOL_CTRL, value);
}

int bd37033_get_volume(void)
{
	//printf("bd37033 get volume %d\n", s_volume);
	return g_volume;
}

/**
 * @brief bd37033_set_bass
 * 
 * @param bass 
 */
int bd37033_set_bass(int bass)
{
    uint8_t value  = bass & 0x0f;
	s_bass = value;
	//printf("bd37033 set bass %d\n", bass);
    return bd37033_reg_write(COMMAND_BASS_CONTROL, value);

}

/**
 * @brief bd37033_set_treble
 * 
 * @param treble 
 */
int bd37033_set_treble(int treble)
{
    uint8_t value  = treble & 0x0f;
	s_treble = value;
	//printf("bd37033 set treble %d\n",  treble);
    return bd37033_reg_write(COMMAND_TREBLE_CONTROL, value);
}

/**
 * @brief bd37033_set_middle
 * 
 * @param treble 
 */
int bd37033_set_middle(int middle)
{
    uint8_t value  = middle & 0x0f;
	s_treble = value;
	//printf("bd37033 set treble %d\n",  treble);
    return bd37033_reg_write(COMMAND_MID_CONTROL, value);
}

/**
 * @brief bd37033_set_balance
 * 
 * @param balance 
 */
int bd37033_set_speaker(int channel, int volume)
{
	if(channel == SPEAKER_CH_LF)
	{
		uint8_t value_lf = 128 + 80 - volume;
		s_lf_att = value_lf;
		bd37033_reg_write(COMMAND_LF_ATTENUATOR, value_lf);
	}
	else if(channel == SPEAKER_CH_RF)
	{
		uint8_t value_rf = 128 + 80 - volume;
		s_rf_att = value_rf;
		bd37033_reg_write(COMMAND_RF_ATTENUATOR, value_rf);
	}
	else if(channel == SPEAKER_CH_LR)
	{
		uint8_t value_lr = 128 + 80 - volume;
		s_lr_att = value_lr;
		bd37033_reg_write(COMMAND_LR_ATTENUATOR, value_lr);
	}
	else if(channel == SPEAKER_CH_RR)
	{
		uint8_t value_rr = 128 + 80 - volume;
		s_rr_att = value_rr;
		bd37033_reg_write(COMMAND_RR_ATTENUATOR, value_rr);
	}

    //printf("bd37033 set speaker %d %d %d %d\n", lf, rf, lr, rr);
    return 0;
}


int bd37033_set_mute(int enable)
{
	//printf("bd37033 set mute %d\n", enable);
	if(enable)
	{
		bd37033_reg_write(COMMAND_LF_ATTENUATOR, 0xff);
    	bd37033_reg_write(COMMAND_RF_ATTENUATOR, 0xff);
    	bd37033_reg_write(COMMAND_LR_ATTENUATOR, 0xff);
    	bd37033_reg_write(COMMAND_RR_ATTENUATOR, 0xff);

		 
		bd37033_reg_write(COMMAND_INPUT_GAIN, s_gain| 0x80  );
    }
    else
    {
		bd37033_reg_write(COMMAND_LF_ATTENUATOR, s_lf_att);
    	bd37033_reg_write(COMMAND_RF_ATTENUATOR, s_rf_att);
    	bd37033_reg_write(COMMAND_LR_ATTENUATOR, s_lr_att);
    	bd37033_reg_write(COMMAND_RR_ATTENUATOR, s_rr_att);  	

		bd37033_reg_write(COMMAND_INPUT_GAIN,  s_gain& 0x1f);
    }

	return 0;
}
int bd37033_set_fader(int val)
{

	s_lf_att = val;// 0;
	s_rf_att = val;// 0;
	s_lr_att = val;// 0;
	s_rr_att = val;// 0;
	bd37033_reg_write(COMMAND_LF_ATTENUATOR, val);
	bd37033_reg_write(COMMAND_RF_ATTENUATOR, val);
	bd37033_reg_write(COMMAND_LR_ATTENUATOR, val);
	bd37033_reg_write(COMMAND_RR_ATTENUATOR, val);  	
    bd37033_reg_write(0x2C, val); 
	bd37033_reg_write(0x30, val); 
 
	return 0;
}

dap_t *register_dap(void)
{
	dap_t *pdap = (dap_t*)malloc(sizeof(struct _dap_t));
	if(pdap == NULL)
	{
		printf("error malloc dap\n");
		return NULL;
	}
	else
	{
		pdap->init         = bd37033_device_init;
		pdap->deinit       = bd37033_device_close;
		pdap->set_source   = bd37033_set_source;
		pdap->set_loudness = bd37033_set_loudness;
		pdap->set_volume   = bd37033_set_volume;
		pdap->get_volume   = bd37033_get_volume;
		pdap->set_bass     = bd37033_set_bass;
		pdap->set_treble   = bd37033_set_treble;
		pdap->set_speaker  = bd37033_set_speaker;
		pdap->set_mute     = bd37033_set_mute;
	}
	
	return pdap;
}






