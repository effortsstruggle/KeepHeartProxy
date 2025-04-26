#pragma once
#include <string>
//定义枚举值
//used for ieai_mcu_get_int(MCU_INDEX index)
enum MCU_INDEX {
    MI_UNLOCK = 0,      // 解锁信息
    MI_RNDS_GEAR,       // 挡位R(1)、N(2)、D(3)、S(4)
    MI_P_GEAR,          // p 档状态
    MI_GEAR_ERR_P,      // P挡是否有错误（0或者1）
    MI_GEAR_ERR_RNDS,   // RNDS挡是否有错误（0或者1）
    MI_EQ,              // 剩余电量百分比
    MI_HEAT,            // 加热状态（0或者1）
    MI_CHARGE_STATE,    // 充电状态
    MI_VOLTAGE,         // 充电电压(0.1v)
    MI_CURRENT,         // 充电电流(0.1A)
    MI_CHARGEDTIME,     // 已充电时间(s)
    MI_TOCHARGINGTIME,  // 剩余充电时间(s)
    MI_IS_AC,           // 是否是交流充电 0无 1 有
    MI_MATITENANCE,     // 保养提示 0 无 1 有
    MI_TBox4G,          // 有无Tbox4G信号 0无 1 有
    MI_CUR_TIME,        // 当前时间 1970年以来的秒数
    
    MI_SPEED,           // 当前车速(km/h)
    MI_SPEEDTRIP1,      // Trip1平均车速(km/h)
    MI_CONSUMETRIP1,    // Trip1百公里电耗(0.1度)
    MI_TIMETRIP1,       // Trip1行驶时间(min)
    MI_TRIP1,           // Trip1里程(0.1km)
    MI_SPEEDTRIP2,      // Trip2平均车速(km/h)
    MI_CONSUMETRIP2,    // Trip2百公里电耗(0.1度)
    MI_TIMETRIP2,       // Trip2行驶时间(min)
    MI_TRIP2,           // Trip2里程(0.1km)    
    MI_ODO,             // ODO里程(0.1km)
    MI_RANGE,           // 续航里程(0.1km)
    MI_TOTALTIME,       // 总行驶时间(min)
    MI_AC_TYPE,         // 空调类型;  0 vilg
    MI_BRIGHTNESS_LEVEL,// 屏幕亮度
    MI_LIGHT_STATUS,    // 灯光状态
    MI_UPDATE_ERR_CODE, // 更新故障码
    MI_MCUVER
};

//used for ieai_mcu_set(MCU_COMMAND cmd, int data1, long data2)
enum MCU_COMMAND {
    MC_UNLOCK = 100,        // data1=0, data2=0    解锁
    MC_SET_GEAR,            // data1=gear, data2=0
    MC_CLEAR_TRIP,          // data1=0; data2=0
    MC_POWEROFF ,           // 关机确认      1：按键下电确认   2 soc已准备好下电
    MC_INS_UPDATE,          // 通知仪表升级
    MC_AC_MODE,             // 空调模式  0x0:关闭；0x1:Cool；0x2:Heat
    MC_AC_TEMP,             // 空调温度（当前温度 -17） * 2 ，每次加 1 代表 0.5
    MC_AC_WIND_SPEED,       // 空调风速 共计10 档
    MC_AC_WIND_DIR,         // 空调风向  0x1:Blow face  0x2:Blow face & Blow foot 0x3:Blow foot 0x4:Blow foot & Defrost active除霜(挡风玻璃)

    MC_AC_SWITCH,           // 交流电插座开关
    MC_FORWORD_SPEED,       // 前向速度
    MC_BACK_SPEED,          // 后退速度

    MC_AUTO_BRAKING_LEVEL,  // 再生制动等级
    MC_GREEP,               // 爬行模式
    
    MC_EPB_AUTOHOLD,        //  EPB自动保持
    MC_EPB_REPAIR,          //  EPB维修放行命令
    MC_AUTO_POWER_OFF,         //  自动关机时间 发给mcu 与仪表关机没有关系
   

    MC_INS_UNITS_MILE,      // 里程
    MC_INS_UNITS_TEMP,      // 温度
    MC_INS_UNITS_TIME,      // 时间
    MC_INS_UNITS_TIRE,      // 胎压
    MC_INS_BRIGHTNESS_MODE, // 亮度模式
    MC_INS_UNITS_BRIGHTNESS,// 亮度
    MC_INS_UNITS_LANGUAGE,  // 语言
    MC_BTKEY_SEARCH,    // 蓝牙钥匙搜索请求
    MC_BTKEY_PAIR,      // 蓝牙钥匙配对请求
    MC_BTKEY_PAIR_RES,  // 蓝牙钥匙请求结果
    MC_BTKEY_DELETE,      // 蓝牙钥匙删除配对
};

//used for onMcuNotify(MCU_NOTIFY code, int data1, long data2)
enum MCU_NOTIFY {
    MN_ERR = 200,       //data1=err, data2=0
    MN_OTA,             //升级信息
    MN_CHARGE,          //data1=charge, data2=0
    MN_KEY,              //data1=key, data2=0
    MN_REQUEST_OFF,       // 请求关机 1
    MN_ERR_CODE,         // 设备通知的错误码
    MN_BTKEY_NOTIFY,     // 蓝牙钥匙通知
};

typedef void (*OnMcuNotify)(MCU_NOTIFY code, int data1, long data2);
typedef void (*OnMcuNotifyStr)(MCU_NOTIFY code, int data1, std::string&);