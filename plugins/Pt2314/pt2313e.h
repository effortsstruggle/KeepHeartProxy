/*****************************************************************************
FileName : pt2313.h
Function : PT2313音频驱动
*****************************************************************************/
#ifndef _PT2313_H_
#define _PT2313_H_
#define EFFECT_DEV "/dev/i2c-1"

#define ON                      1
#define OFF                     0
#define PT2313_Addr             0x44          // 芯片地址
#define Stereo1                 0x00          // 音源通道选择
#define Stereo2                 0x01
#define Stereo3                 0x02
#define Stereo4                 0x03
#define VolCtrlCmd              0x00          // 控制命令
#define ATTRLCtrlCmd            0xc0
#define ATTRRCtrlCmd            0xe0
#define ATTFLCtrlCmd            0x80
#define ATTFRCtrlCmd            0xa0
#define SwitchCtrlCmd           0x40
#define BassCtrlCmd             0x60
#define TreblwCtrlCmd           0x70

#define LOUDON                  0x00
#define LOUDOFF                 0x04
#define Volume_Default          45//10            // 默认值
#define Treble_Default          20//7
#define Bass_Default            20//7
#define Vol_Max_Val             60//20

#define MAIN_VOL_MAX            60 //主音量最大值
#define MAIN_VOL_DEFAULT        45 //主音量默认值
#define BAS_TRE_MAX             30 //高低音最大值
#define BAS_TRE_DEFAULT         20 //高低音默认值

#define USER                    0x00          // 音效
#define POP                     0x01
#define CLAS                    0x02
#define ROCK                    0x03
#define JAZZ                    0x04
#define NORMAL                  0x05
typedef unsigned char uint8_t;
typedef unsigned char u8;

// 极限控制，用于判断指定值是否超过一维数组大小，若超过则修改成数组大小
#define GETARRSIZE(arr) ((sizeof(arr)/sizeof(arr[0])))
#define ISVal(variable, _arr) (variable > GETARRSIZE(_arr) - 1 ? variable = GETARRSIZE(_arr) - 1 : variable)


#define IIC_DATA                2       //PB2
#define IIC_CLK                 1       //PB1
#define DATA_DE                 GPIOBDE
#define CLK_DE                  GPIOBDE
#define DATA_DIR                GPIOBDIR
#define CLK_DIR                 GPIOBDIR
#define DATA_PU                 GPIOBPU
#define CLK_PU                  GPIOBPU
#define DATA_PORT               GPIOB
#define CLK_PORT                GPIOB



typedef struct {
    u8 CrtAudioCh;  //音频通道
    u8 LoudnessSta; //响度
    u8 MasterVolVal;//主音量
    //u8 TempVolVal;  //
    u8 TrebleVal;   //高音
    u8 BassVal;     //低音
    u8 FadeVal;     //增益
    u8 BalVal;      //均衡
    u8 EQ;          //EQ
} pt2313_type;
extern pt2313_type pt2313_ctl;
/**
 *@description: 输入增益 设置 0～3 由0 到最大
 *@author: sky
 *@param input[in] 输入增益类型
 *@return 无意义
 *@date: 2024-11-22 09:29:25
*/
int pt2313_set_input_gain(uint8_t input);

int pt2313_set_source(uint8_t source);
int pt2313_set_volume(uint8_t vol);
int pt2313_set_bass(int bass);
int pt2313_set_treble(int treble);
int pt2313_set_balance(int balance);
int pt2313_reg_write_data(int reg, int data);

int pt2313_set_mute(int enable);
int pt2313_device_init(void);
int pt2313_device_close(void);
/**
 *@brief: 芯片是否打开
 *@author: sky
 *@version 
 *@return 0:打开成功
         -1 打开失败
 *@date: 2025-01-23 09:38:58
*/
int pt2313_opened();
#endif //#ifndef _PT2313_H_