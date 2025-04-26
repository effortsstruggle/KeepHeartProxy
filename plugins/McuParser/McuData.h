#pragma once
#include <stdint.h>

typedef enum
{
    KEY_VALUE_NONE      = 0x00,         // 无按键
    KEY_VALUE_RETURN    = 0x01,         // 返回键
    KEY_VALUE_PREV      = 0x02,         // 上一曲
    KEY_VALUE_PLAY      = 0x04,         // 播放键
    KEY_VALUE_NEXT      = 0x08,         // 下一曲
    KEY_VALUE_VOLUP     = 0x10,         // 音量加
    KEY_VALUE_VOLDOWN   = 0x20,         // 音量减
    KEY_VALUE_UNLOCK    = 0x40,         // 解锁键
} tpp_key_value_type_t;


#pragma pack(1)


// 包头的定义
typedef struct
{
    uint16_t flag ;        // 协议头 0xCA53 固定的协议包头
    uint16_t length;      // 根据具体的协议，长度会有所不同  /总长度 减 4 ，不包含flag 和 length
    uint16_t command;     // 请求指令   0xB001  // 发送给mcu 0xC00*  接受mcu 0XD00*
} tagPacketHead ;

// 接收MCU的升级请求
typedef struct {
    tagPacketHead header;
    uint8_t  result;      // MCU校验结果:0 无升级，1 有升级，2 进入升级状态，3 升级完成，4 未知错误
    uint16_t   crc;         // crc16值 除校验位之外的crc校验
}updataePacket;

typedef struct tagMcu2Soc
{
    tagPacketHead     header;                     // 0状态信息头：command 0xD001
    
    struct {
        uint8_t     RNDSgear : 5;                        // RNDS 当前档位
        uint8_t     Pgear : 1;                           // P档当前状态
        uint8_t     PFault : 1;                      //  P档 故障
        uint8_t     DNRFault : 1;                    //  是否RNDS P档是否故障
    } gearInfo;

    struct {
        uint8_t     battery_soc;                // 2 电量百分比
        uint8_t     battery_charge_state : 1,   // 3 电池充电状态 0未冲，1正在冲
                    battery_heat_state : 1,     // 3 电池加热状态 0未加热，1正在加热
                    reserve : 6;                // 3 电池充电状态
        uint16_t    battery_voltage;            // 4-5 充电电压
        uint16_t    battery_current;            // 6-7 充电电流
        uint32_t    charge_time;                // 8-11 充电时间
        uint32_t    charge_time_left;           // 12-15 剩余充电时间
       
    } batteyInfo;
   
    struct {
        uint8_t     trip1_avg_speed;            // 18 平均车速 km/h
        uint16_t    trip1_elec_consume;         // 19-20 电耗       0.1度

        uint16_t    trip1_time_info : 15,       // 21-22 行驶时间 单位分钟
                    trip1_time_flag : 1;        // 行驶时间，数据有效 0/1=无效/有效
    
        uint16_t    trip1_info : 15,                 // 23-24 小计里程信息 单位0.1km
                    trip1_flag : 1;                  // 小计里程信息，数据有效 0/1=无效/有效
    } trip1;

   
    struct {
        uint8_t     trip2_avg_speed;            // 25 平均车速 km/h
        uint16_t    trip2_elec_consume;         // 26-27 电耗       0.1度

        uint32_t    trip2_time_info : 31,            // 28-31 行驶时间 单位分钟
                    trip2_time_flag : 1;        // 行驶时间，数据有效 0/1=无效/有效

        uint16_t    trip2_info : 15,                 // 32-33 小计里程信息 单位0.1km
                    trip2_flag : 1;                  // 小计里程信息，数据有效 0/1=无效/有效
    } trip2;

   
    struct {
        uint32_t    odo_info : 31,            // 34-37 总里程信息
                    odo_flag : 1;             // 总里程信息，数据有效 0/1=无效/有效

        uint16_t    range_left;               // 38-39 续航里程   里程数值，精度：0.1km(mile)/bit

        uint32_t    odo_time_info : 31,              // 40-43 行驶时间 单位分钟
                    odo_time_flag : 1;        // 行驶时间，数据有效 0/1=无效/有效
    } odo_info;


    uint16_t    error_code;                   // 44-45 故障码
    uint8_t     ac_type;                      // 46    空调类型
    uint8_t     unlock;                       // 开机解锁方式，0 未解锁，3需要秘密输入
    uint8_t     request_poweroff;             // 请求关机   1 
    uint8_t     inst_bright;                  // 仪表亮度

    uint8_t     key_state ;                   // 16 共7个按键，0表示未按下，1表示按下
                   

    uint8_t     speed;                        // 17 车速 km/h

    uint32_t    realtime;                     // 时间自1970-01-01 00:00:00 到现在的秒数
   
    uint8_t     maintenance;                  // 0 不需要维护，1 需要维护
    uint8_t     tbox4G;                       // 0 无 1，有
    uint8_t     alternatingCurrent;           // 0 不是，1 是交流电
    uint8_t     lightstatus;                  // 灯光状态 0～6 

    uint32_t     mcuVersion;                   // 46    系统版本号0bit    阶段标识,0 ：试制阶段，1 ：正式版本 1-7bit  MCU Build 版本 MCU 每次发布+1 的版本号
    uint16_t   crc;                           // crc16值
}tag_mcu_to_soc;

#define MCU2SOCCRC  sizeof(tag_mcu_to_soc) - sizeof(uint16_t)

// 接收蓝牙消息
typedef struct packetMcu2SocD002          // 状态信息头：0xD002
{
    tagPacketHead     header;             // 状态信息头：0xD002
    uint8_t     vMsgType;                       // 消息类型，0：扫描设备列表，1：绑定设备列表
    char     vMacAddr[12];                 // 蓝牙钥匙/App MAC地址
    uint8_t     vAddrType;                  //地址类型
    uint8_t     vName[34];                 //蓝牙名称
    uint8_t     vDeviceType;                //设备类型，0：蓝牙钥匙，1：手机APP
    uint8_t     vLinkIndex;                 //连接索引,当消息类型为绑定设备列表时有效
    uint16_t   crc;                       // 校验值
}packet_buletooth_msg;
#define PACKET_BTKEY_CRCLEN sizeof(packet_buletooth_msg) -sizeof(uint16_t)


//========================== SOC 发送给 MCU 的消息 =======================================================
// 发送给mcu 密码解锁开机
typedef struct packetSoc2McuC001
{
    tagPacketHead     header;                     // 状态信息头：0xC001
    uint8_t     lock_state;                          // 保留
    uint16_t   crc;                                // 校验值
}s_packet_unlock;



// 发送给MCU 档位
typedef struct packetSoc2McuC002
{
    tagPacketHead     header;                     // 状态信息头：0xC002

    struct McuData{
            uint8_t rnds_state:7;                 // RNDS 档位
            uint8_t p_state:1;                    // P档状态
 
    }gear_state ;                        // 
    uint16_t   crc;                               // 校验值
}s_packet_gear;


// soc 发送给mcu trip2 清空
typedef struct packetSoc2McuC003
{
    tagPacketHead     header;                     // 状态信息头：0xC003
    uint8_t     clean_flag;                 // 保留
    uint16_t   crc;                       // 校验值
}s_packet_trip2;



//  soc 发送给mcu 空调设置
typedef struct packetSoc2McuC004
{
    tagPacketHead     header;                     // 状态信息头：0xC004
    uint8_t     ac_mode;                          // 空调模式
    uint8_t     ac_temp;                          // 温度调节
    uint8_t     wind_speed;                       // 风速调节
    uint8_t     wind_direction;                   // 风向
    uint16_t    crc;                              // 校验值
}s_packet_climate;


// 关机
typedef struct packetSoc2McuC005          // 状态信息头：0xC005
{
    tagPacketHead     header;             // 状态信息头：0x
    uint8_t     cmd;                       
    uint16_t   crc;                       // 校验值
}s_packet_close;

// 仪表设置
typedef struct packetSoc2McuC006         // 状态信息头：0xC006
{
    tagPacketHead header;                // 状态信息头：0x
    uint8_t       units_mileage:2,       // 里程单位格式
                  units_temp:2,          // 温度单位格式
                  units_time:2,          // 时间单位
                  units_tirePressure:2;  // 胎压单位
    uint8_t       brightness_mode:1,     // 亮度模式 00 自动，01 手动
                  brightness_level:3,    // 亮度等级
                  language:4;            // 语言 00 中文 01 英文 02 俄文
    uint16_t      crc;                       // 校验值
}s_packet_instument;


// 仪表升级 
typedef struct packetSoc2McuC007          // 状态信息头：0xC007
{
    tagPacketHead     header;             // 状态信息头：0x
    uint8_t     cmd;                       // 1 确认
    uint16_t   crc;                       // 校验值
}s_packet_ins_update;

//vcu车身动力设置
typedef struct packetSoc2McuC008          // 状态信息头：0xC008
{
    tagPacketHead     header;             // 状态信息头：0xC008
    uint8_t     vForwardMaximumSpeed;     // 前向最高车速,档位km/h
    uint8_t     vBackwardMaximumSpeed;     // 后向最高车速,档位km/h

    uint8_t     vACSocketSwitch : 1;            //交流电插座开关, 0 : off, 1 : on
    uint8_t     vRegenLevel : 2;                //再生制动等级, 0 : 舒适comfortable, 1 : normal普通, 3 : stronge强
    uint8_t     vCreepOrHillHold : 1;           //爬行模式, 0:爬行,无坡道保持, 1:无爬行,坡道保持
    uint8_t     vEPBAutoHold : 1;               //EPB自动保持，0：off, 1:on
    uint8_t     vEPBRepair : 1;                 //EPB维修释放命令, 0：EPB正常状态，1：EPB维修释放命令
    uint8_t     : 2;                           //reserve
    uint8_t     vAutoPoweroffTime;          //自动关机时间，单位0.1h
    uint16_t   crc;                       // 校验值
}s_packet_vcu_setting;
#define PACKET_VCU_SETTING_LENGTH  sizeof(s_packet_vcu_setting)


//蓝牙钥匙扫描绑定/解绑指令
typedef struct packetSoc2McuC009          // 状态信息头：0xC009
{
    tagPacketHead     header;             // 状态信息头：0xC009
    uint8_t     vCmd;                       // 指令，0：扫描附近蓝牙钥匙，1：绑定指定蓝牙钥匙，2:蓝牙钥匙解绑 3获取绑定列表
    char      vMacAddr[12];                 // 蓝牙钥匙/App MAC地址
    uint8_t     vAddrType;                  //地址类型，绑定需指定，扫描时MCU上传
    uint8_t     vLinkIndex;                 //连接索引，解绑需指定，获取绑定列表时MCU上传
    uint16_t   crc;                       // 校验值
}s_packet_bt_key_cmd;
#define PACKET_BULETOOTH_CMD_LENGTH  sizeof(s_packet_bt_key_cmd)


#pragma pack()


