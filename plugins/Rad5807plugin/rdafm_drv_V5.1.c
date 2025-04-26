
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "rdafm_drv_V5.1.h"



#define DEV_I2C_BUS "/dev/i2c-5"

static unsigned char SLAVE_ADDR  = 0x22 >>1;

static int fm_fd = -1;


#define  LOGD(fmt) printf(fmt);

/***************************************************
RDAFM interfaces
****************************************************/


unsigned short int gChipID = 0;
/***************************************************
 * 所有寄存器的操作值，只定义了相关位数的值，转换为16进制
****************************************************/

// 02 寄存器操作 没有全写，需要再添加

#define   R02_RDA_SLEEP           0x0000   //  RDA5807芯片睡眠
#define   R02_RDA_POWERON         0x0001   //  RDA5807芯片上电 / 从睡眠中进入工作状态
#define   R02_RESET               0x0002   //  复位
#define   R02_RDS_EN              0x0008   //  RDS/RBDS enable ,取反将改为0 关闭 默认关闭

#define   R02_SEEK                0x0100   //  启动搜台 
#define   R02_SEEK_DIR            0x0200   //  搜台方向，1：seek up  ，0: seek down ，默认是0   
#define   R02_CLK_POWERON         0xC001   //  晶振模块上电

// 03 寄存器操作 
#define   R03_SPACE_100           0x0000   //  搜台步长 设为 100 kHz  共两位 [1:0]
#define   R03_SPACE_200           0x0001   //  搜台步长 设为 200 kHZ
#define   R03_SPACE_50            0x0002   //  搜台步长 设为 50 kHz  
#define   R03_SPACE_25            0x0003   //  搜台步长 设为 25 kHZ

#define   R03_BAND_87_108         0x0000   //  波段 US/Europe   [3:2]
#define   R03_BAND_76_91          0x0004   //  Japan
#define   R03_BAND_76_108         0x0008   //  world wide 
#define   R03_BAND_65_76          0x000c   //  East Europe



#define   R0A_STC_OK                   0x4000


unsigned short int RDAFM_REGW[8] = {
    0x0002,
    0xC001,
    0xC001,
    0x0000,
    0x0400,
    0xC6E1, //05h
    0x6000,
    0x7212, //07H
};

// 初始化寄存器，并记录设置的值，
unsigned short int RDA580XN_init_reg[]={
    0x0002,
    0xC001,
#if defined(_SHARE_CRYSTAL_24MHz_)
    0xC051, //02H:
#elif defined(_SHARE_CRYSTAL_12MHz_)
    0xC011, //02H:
#else
    0xC001,
#endif
    0x0000,
    0x0400,
    0xC6E1, //05h
    0x6000,
    0x7212, //07H
    0x0000,
    0x0000,
    0x0000,  //0x0ah
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,  //0x10h
    0x0019,
    0x2A11,
    0xB042,  
    0x2A11,  
    0xB831,  //0x15h 
    0xC000,
    0x2A91,
    0x9400,
    0x00A8,
    0xc400,  //0x1ah
    0xF7CF,   
    0x2414,  //0x1ch
    0x806F, 
    0x4608,
    0x0086,
    0x0661, //0x20H
    0x0000,
    0x109E,
    0x23C8,
    0x0406,
    0x0E1C, //0x25H
};

void delayms(unsigned short int ms)
{
    
	usleep(ms*1000);
}
/**
 * @brief: power on RDAFM
 * @retval
 */
void RDAFM_power_on(void)
{
    unsigned char i = 0;

	RDAFM_write_data(0x02, R02_RESET);
	delayms(50);

	RDAFM_read_data(0x0E, &gChipID);

	delayms(10);

    if (gChipID == 0x5808)
    {
      
        for (i=0;i<8;i++)
            RDAFM_REGW[i] = RDA580XN_init_reg[i];

        RDAFM_write_data(0x02, R02_CLK_POWERON);

        delayms(600); 
        // 初始化每个寄存器
        for (i=0x02; i < (sizeof(RDA580XN_init_reg)/sizeof(RDA580XN_init_reg[0])); i++)
	        RDAFM_write_data(i, RDA580XN_init_reg[i]);

    }

	delayms(50);         //dealy 50 ms
}




/**
 * @brief RDAFM power off function
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param void
 * @return void
 * @retval
 */
void  RDAFM_power_0ff(void)
{
	RDAFM_REGW[2] &= (~1);
	RDAFM_write_data(0x02, RDAFM_REGW[2]);
}







void RDAFM_set_volume(unsigned char level)
{
	RDAFM_REGW[5] = (( RDAFM_REGW[5] & 0xfff0 ) | (level & 0x0f)); 

	RDAFM_write_data(0x05, RDAFM_REGW[5]);
    delayms(50);    //Dealy 50 ms
}


/**
 * @brief Cover the frequency to channel value
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param unsigned short int frequency: frequency  value
 *        frequency unit: 10KHz
 * @return unsigned short int: channel value
 * @retval
 */
unsigned short int RDAFM_freq_to_chan(unsigned short int frequency) 
{
	unsigned char channelSpacing;
	unsigned short int bottomOfBand;
	unsigned short int channel;

    // 获取波段范围 
	if ((RDAFM_REGW[3] & 0x000C) == 0x0000) 
		bottomOfBand = 8700;
	else if ((RDAFM_REGW[3] & 0x000C) == 0x0004)	
		bottomOfBand = 7600;
	else if ((RDAFM_REGW[3] & 0x000C) == 0x0008)	
		bottomOfBand = 7600;

    // 获取通道间距
	if ((RDAFM_REGW[3] & 0x0003) == 0x0000) 
		channelSpacing = 10;
	else if ((RDAFM_REGW[3] & 0x0003) == 0x0001) 
		channelSpacing = 20;
	else if ((RDAFM_REGW[3] & 0x0003) == 0x0002) 
		channelSpacing = 5;

	channel = (frequency - bottomOfBand) / channelSpacing;

	return (channel);
}


void RDAFM_set_freq(unsigned short int curFreq)
{   
	unsigned short int curChan;
    unsigned short int tReg = 0;

	if(curFreq <= 9500)
	{
		tReg = 0x8831; 
		RDAFM_write_data(0x15 , tReg);	
	}
	else
	{
		tReg = 0xF831; 
		RDAFM_write_data(0x15 , tReg);	
	}
	delayms(30);

	curChan = RDAFM_freq_to_chan(curFreq);
    RDAFM_REGW[3] = ((curChan<<6) | 0x10 | (RDAFM_REGW[3]&0x0f));
	RDAFM_write_data(0x03, RDAFM_REGW[3]);

	delayms(50);     //delay 50 ms
}


/**
 * @brief Set RDAFM to mute mode
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param  mute: if mute is true,then set mute; if mute is false,then set no mute
 * @return void
 * @retval
 */
void RDAFM_set_mute(unsigned char mute)
{
	if(mute)
	 	RDAFM_REGW[2] &= ~(1 << 14);
	else
		RDAFM_REGW[2] |= (1 << 14);

	RDAFM_write_data(0x02, RDAFM_REGW[2]);
    delayms(50);    //dealy 50 ms
}







/**
 * @brief Cover the channel to frequency value
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param unsigned short int chan: channel value
 * @return unsigned short int: frequency value
 *        frequency unit: 10KHz
 * @retval
 */
unsigned short int RDAFM_chan_to_freq(unsigned short int chan) 
{
	unsigned char channelSpacing;
	unsigned short int bottomOfBand;
	unsigned short int freq;

	if ((RDAFM_REGW[3] & 0x000C) == 0x0000) 
		bottomOfBand = 8700;
	else if ((RDAFM_REGW[3] & 0x000C) == 0x0004)	
		bottomOfBand = 7600;
	else if ((RDAFM_REGW[3] & 0x000C) == 0x0008)	
		bottomOfBand = 7600;	
	if ((RDAFM_REGW[3] & 0x0003) == 0x0000) 
		channelSpacing = 10;
	else if ((RDAFM_REGW[3] & 0x0003) == 0x0001) 
		channelSpacing = 20;
	else if ((RDAFM_REGW[3] & 0x0003) == 0x0002) 
		channelSpacing = 5;

	freq = bottomOfBand + chan * channelSpacing;

	return (freq);
}








/**
 * @brief Station judge for auto search
 * @In auto search mode,uses this function to judge the frequency if has a station
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param unsigned short int freq:frequency value
 *        frequency unit: 10KHz
 * @return bool: if return true,the frequency has a true station;otherwise doesn't have a station
 * @retval
 */
int RDAFM_valid_stop(unsigned short int freq)
{
	unsigned short int tReg = 0;	

    RDAFM_set_freq(freq);

    delayms(50);     //delay 50 ms
    RDAFM_read_data(0x0B, &tReg);	

	//check FM_TURE in 0B register，没搜到台
	if((tReg & 0x0100) == 0)
	{
	    return 0;
	}


	
    return 1;
	
}





/**
 * @brief Get the RSSI of the current frequency
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param void
 * @return unsigned char: the RSSI
 * @retval
 */
unsigned char RDAFM_get_rssi(void)
{    
    unsigned short int tReg = 0;	

    RDAFM_read_data(0x0B, &tReg);	
	delayms(50);    //dealy 50 ms

    return (tReg>>9);  /*return rssi*/
}








/**
 * @brief Initialization work before power on RDAFM and 
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param void
 * @return bool:if true,the operation is successful;otherwise is failed
 * @retval
 */
unsigned char RDAFM_init(void)
{
    //Initialization work before power on RDAFM
    //Add your code here such as Initialization work of I2C. 
	fm_fd = open(DEV_I2C_BUS, O_RDWR);
    if (fm_fd < 0) {
        LOGD("open dev filed \n");
        close(fm_fd);
        fm_fd = -1;
        return -1;
    }
    LOGD("open dev success \n");

    return 1;

}



// 每次值操作一个寄存器，故数据最长为 short
// addr 寄存器地址
// data 数据
int RDAFM_write_data(unsigned char reg_addr, unsigned short int data) {
    //TODO
  
    struct i2c_rdwr_ioctl_data rwdata;
   
	ioctl(fm_fd, I2C_TIMEOUT, 1);
    ioctl(fm_fd, I2C_RETRIES, 2);

    // 存储数据
	unsigned char arr[3];
    arr[0] = reg_addr;
 
	
	arr[1] = data>>8;
    arr[2] = data;
 //   printf("write data: %#X %#X \n" ,arr[1],arr[2]);

	struct i2c_msg msg= {

		.addr = SLAVE_ADDR,		  //设置从机额地址
		.flags = 0,				  //设置为写
		.buf = (__u8*)arr,		  //数据
		.len = 3                  //数据长度
		};


    rwdata.msgs = &msg;
    rwdata.nmsgs = 1;
    if (ioctl(fm_fd, I2C_RDWR, &rwdata) < 0) {
        printf("write reg %0X failed \n", reg_addr);
        return -1;
    }
    usleep(500);
    
    return 0;
}

//S, id, address, delay, S, id, *buf, NACK,*last, P
int RDAFM_read_data(unsigned char addr, unsigned short int *pdata) {
    struct i2c_rdwr_ioctl_data rwdata;

    ioctl(fm_fd, I2C_TIMEOUT, 1);
    ioctl(fm_fd, I2C_RETRIES, 2);

	//从i2c里面读出数据
	printf("\nbegan to read: 0x%02X\n", addr);

    struct i2c_msg msgs[2];
	//先设定一下地址
    msgs[0].addr =  SLAVE_ADDR;
	msgs[0].len = 1;
	msgs[0].flags = 0;//write
	msgs[0].buf = (unsigned char *)malloc(1);
	msgs[0].buf[0] = addr;
	//然后从刚才设定的地址处读
    unsigned char buf[2] = {0};
	msgs[1].len = 2;
	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = SLAVE_ADDR;
	msgs[1].buf  = &buf;
	
    rwdata.nmsgs  = 2;
	rwdata.msgs  = &msgs;

	int ret = ioctl(fm_fd, I2C_RDWR, (unsigned long)&rwdata);
	if(ret < 0){
		printf("error during I2C_RDWR ioctl with error code %d \n", ret);
        return 0;
 
    }
    short tmp = buf[0] << 8;
    tmp |= buf[1];
    *pdata = tmp;
    //printf(" iic read data  = %d", buf);
    printf(" iic read data  = 0x%04X  \n", *(unsigned short int*)pdata);

   

	return 0; 
}



int RDAFM_auto_seek()
{
    unsigned short int curChan;
    unsigned short int tReg = 0;
    int i = 0;
    unsigned short minfreq = 8700;
    unsigned short maxfreq = 10800;
       
    int res[210] = {0};
    int index = 0;
    for (i=minfreq;i<=maxfreq;i += 10 ){
        
        if(RDAFM_valid_stop(i))
        {
              res[index++] = i;          // 记录
        }
	 
   
    }
    printf ("seek result : \n");
    for(int j = 0;j < 210; j++)
    {
        if(res[j])
        {
            printf ("%d \n",res[j]);
        } 
    }
   return 1;
}

/**
 * @brief RDAFM test
 * @author RDA Ri'an Zeng
 * @date 2008-11-05
 * @param void
 * @return bool:if true,the operation is successful;otherwise is failed
 * @retval
 */
void  RDAFM_test(void)
{
    // 90.70 
    
    short int data;
    RDAFM_init();
    RDAFM_power_on();
    
    // RDAFM_read_data(0x02, &data);
    // data |= R02_RDS_EN;
    // RDAFM_write_data(0x02,data);

    RDAFM_set_freq(9070);
    RDAFM_read_data(0x0B,&data);

    printf ("seek start : \n");
    RDAFM_auto_seek();
}

// int main()
// {
//     printf("test fm5807 \n");
//     RDAFM_test();
    
//     return 0;
// }


