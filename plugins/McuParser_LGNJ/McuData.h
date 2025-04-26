#pragma once
#include<cstdint>

#pragma pack(1)


//共用结构体
typedef struct  {
    uint16_t m_u16Flow;//最大流量
    uint8_t  m_u8NumberOfProgressiveCurv;//渐进曲线
    uint16_t m_u16TMax;//最大时间
} ValveProperty;

// 包头的定义
typedef struct
{
    uint16_t m_u16Flag ;       // 协议头 0xCA53 固定的协议包头
    uint16_t m_u16Length;      // 根据具体的协议，长度会有所不同  /总长度 减 4 ，不包含flag 和 length
    uint16_t m_u16Command;     // 请求指令   0xB001  // 发送给mcu 0xC00*  接受mcu 0XD00*
} tagPacketHead;

// 接收MCU的升级请求 
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t  m_u8Result;      // MCU校验结果:0 无升级，1 有升级，2 进入升级状态，3 升级完成，4 未知错误
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagMcuUpdatePacket;

enum CarMode{
        CarMode_Invaild = 0,
        CarMode_4H = 40,
        CarMode_6M = 60,
        CarMode_6P1 = 61,
        CarMode_6P2 = 62,
        CarMode_7E = 71,
        CarMode_8H = 80,
        CarMode_8E = 81,
        CarMode_8G = 82
};


/*--------------------------------- MCU -> SOC Begin ---------------------------------*/

// MCU与SOC握手 command:0x0000
typedef struct {
    tagPacketHead m_objPackHead;
    uint32_t  m_u32McuVersion; //软件版本号
    uint8_t  m_u8HardVersion;//硬件版本号
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
}tagMCUShakeHandPacket;

//发动机 command:0x0001
typedef struct  {
    tagPacketHead m_objPackHead;
    uint16_t m_u16EngineSpeed;//当前发动机最大转速
    uint16_t m_u16OutputRmp;//实时动力传输轴转速
    uint32_t m_u32WorkTime;//发动机累计工作小时
    uint8_t  m_u8DPFProhibitSwitchStatus;//DPF再生禁止开关状态
    uint8_t  m_u8DPFSwitchStatus;//DPF再生开关状态
    uint8_t  m_u8MultiType;//多态开关
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
}tagEngineStatusPacket;

//空调状态 //command:0x0002
typedef struct  {
    tagPacketHead m_objPackHead;
    uint8_t m_u8Temp;//温度
    uint8_t m_u8TempUnit;//温度单位
    uint8_t m_u8WindDirection = 1 ;//风向
    uint8_t m_u8ACStatus;//AC状态
    uint8_t m_u8AUTOStatus;//AUTO状态
    uint8_t m_u8OFFStatus = 1 ;//OFF状态 默认关机状态
    uint8_t m_u8LoopStatus;//内外循环状态
    uint8_t m_u8WindSpeed;//风速
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagACStatusPacket;

//空调扩展数据 (不在协议中，通过空调上传的数据计算得来)
typedef struct{
    uint8_t m_u8AcGear ; //空调档位
} tagACStatusPacketEx;


//电控抬升状态 //command:0x0003
typedef struct {
    tagPacketHead m_objPackHead;
    uint16_t m_u16TillingDepth;//耕深
    uint8_t  m_u8ErrorLed : 2 ; //错误指示灯
    uint8_t  m_u8DampingLed : 2; //减震指示灯
    uint8_t  m_u8DownLed : 2 ; //下降指示灯
    uint8_t  m_u8UpLed : 2; //上升指示灯
    uint32_t m_u32ErrorCode;//故障码
    uint8_t  m_u8UpLimit;//高度上限
    uint8_t  m_u8DownSpeed;//下降速度
    uint8_t  m_u8MixturePot;//力位混合
    uint8_t  m_u8RockerSwitch;//模式开关
    uint8_t  m_u8DampingKey;//减震开关
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagElectricCtrlLiftingStatusPacket;


//液压阀状态 //command:0x0004
typedef struct {
    tagPacketHead m_objPackHead;
    ValveProperty m_objValve1{0x01f4 , 0x01 , 0x0064};
    ValveProperty m_objValve2{0x01f4 , 0x01 , 0x0064};
    ValveProperty m_objValve3{0x01f4 , 0x01 , 0x0064};
    ValveProperty m_objValve4{0x01f4 , 0x01 , 0x0064};
    ValveProperty m_objValve5{0x01f4 , 0x01 , 0x0064};
    ValveProperty m_objValve6{0x01f4 , 0x01 , 0x0064};
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
}tagHydraulicValveStatusPacket ;


//变速箱状态 //command:0x0005
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t  m_u8Gear = 0xff ;//档位
    uint8_t  m_u8CarMode = 0xff;//整车模式
    uint8_t  m_u8AutoGearStatus;//自动换挡设置状态*
    uint16_t m_u16CurVehicleSpeed;//当前车速
    uint8_t  m_u8StartGear;//起步档位*
    uint8_t  m_u8DiffLock;//差速锁
    uint8_t  m_u8FourDriver;//四驱显示
    uint8_t  m_u8PTOStatus;//PTO启停状态
    uint8_t  m_u8PTOGear;//PTO档位
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagGearboxStatusPacket;


//后视镜状态 //command:0x0006
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t m_u8Heating;//后视镜加热状态
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagRearviewMirrorStatusPacket;


//氛围灯状态 //command:0x0007
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t m_u8Switch;//氛围灯开关
    uint8_t m_u8Mode;//氛围灯模式
    uint8_t m_u8Color;//氛围灯颜色
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagAtmosphereLightsStatusPacket;


//仪表采集数据 //command:0x0008
typedef struct {
    tagPacketHead m_objPackHead;
    uint16_t m_u16Lux;//光亮强度
    uint8_t  m_u8ScreenBrightness;//屏幕亮度
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagSOCSensorStatePacket;


//TBox时间 //command:0x0009
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t  m_u8Year;//年
    uint8_t  m_u8Month;//月
    uint8_t  m_u8Day;
    uint8_t  m_u8Hour;
    uint8_t  m_u8Minute;
    uint8_t  m_u8Second;
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagTBoxTimeStatePacket;

//TBox扩展数据 (不在协议中，通过TBox上传的数据计算得来)
typedef struct{
    int32_t m_s32Date ; //日期
    int32_t m_s32Time ; //时间
} tagTBoxTimeStatePacketEx;


//记亩作业信息 //command:0x000A
typedef struct {
    tagPacketHead m_objPackHead;
    uint32_t  m_u32CumulativeVal;//累计亩数
    uint32_t  m_u32CurVal;//当前亩数
    uint16_t  m_u16LeftWidth;//左幅宽参数
    uint16_t  m_u16RightWidth;//右幅宽参数
    uint8_t   m_u8ErrorCode;//记亩错误反馈，1.幅宽设置失败正在计亩2. 幅宽设置失败参数无效3. 设置失败，计亩已结束4. 无定位5. 其他原因设置失败0.设置成功
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagWorkInfoPacket ;


//mcu电源模式，通知SOC下电 command //command:0x000B
typedef struct {
    tagPacketHead m_objPackHead;
    uint8_t  m_u8Mode;//1: 下电
    uint16_t m_u16Crc;        // crc16值 除校验位之外的crc校验
} tagMcuPowerModePacket ;
// #define MCU2SOCCRC  sizeof(tagMcuPowerModePacket) - sizeof(uint16_t)

//灯管编程状态切换
typedef struct  {
    tagPacketHead m_objPackHead; //command:0x000C
    uint8_t  m_u8Cmd;//0: 无动作，1:切换灯光编程A状态，2：切换灯光编程B状态
    uint16_t m_u16Crc; 
} tagLightProgramSwitchPacket ;

//车速加 ，车速减
typedef struct {
    tagPacketHead m_objPackHead; //command:0x000D
    uint8_t  m_u8Cmd;//0: 无动作，1:最高车速加 2：最高车速减
    uint16_t  m_u16Crc;
} tagHightSpeedSettingPacket ;


typedef struct  {
    tagPacketHead m_objPackHead;//command:0x000E
    uint32_t m_u32TripFuelConsumption;//本次油耗，0.5L/bit
    uint32_t m_u32TotalFuelConsumption;//总油耗，0.5L/bit
    uint16_t m_u16Crc;
} tagFuelConsumptionPacket;



//4h标定参数信息
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x000F
    uint16_t m_u16Arg1;//当前标定设置页面参数1
    uint16_t m_u16Arg2;//当前标定设置页面参数2
    uint16_t m_u16Arg3;//当前标定设置页面参数3
    uint16_t m_u16Arg4;//当前标定设置页面参数4
    uint16_t m_u16Arg5;//当前标定设置页面参数5
    uint16_t m_u16Arg6;//当前标定设置页面参数6
    uint16_t m_u16Arg7;//当前标定设置页面参数7
    uint16_t m_u16Arg8;//当前标定设置页面参数8
    uint16_t m_u16FrontAxleSensorValue;//前桥转角传感器实际值
    uint16_t m_u16PTOGreaSensorValue;//PTO档位传感器实际值
    uint16_t m_u16AuxiliaryGearboxSensorValue;//副变速箱档位角度传感器实际值
    uint16_t m_u16MainClutchSensorValue;//主离合器传感器实际值
    uint16_t m_u16Crc;
} tagCalibrationParameters4HPacket;

//4H标定状态
typedef struct {
    tagPacketHead m_objPackHead;//command:0x0010
    uint8_t m_u8CalibrationResult;//标定状态反馈 1-成功，2-失败，3-标定中，0-不显示任何信息
    uint8_t m_u8ArgCalibrationResult;//参数标定状态反馈 1-成功，2-失败，3-标定中，0-不显示任何信息
    uint16_t  m_u16Crc;
} tagCalibrationResult4HPacket ;

typedef struct  {
    tagPacketHead m_objPackHead;//command:0x0011
    uint8_t  m_u8SrouceAddr;//故障设备地址
    uint32_t  m_u32DTC : 24;//故障码SPN+FMI
    uint8_t   m_u8OC : 7;//故障发生次数
    uint8_t   m_u8CM : 1;//预留
    uint16_t  m_u16Crc;
} tagDiagnosisTroubleCodePacket;

//测试工装响应
typedef struct   {
    tagPacketHead m_objPackHead;//command:0x0081
    uint16_t  m_u16Cmd;//命令码
    uint8_t   m_u8Arg[64];//命令参数
    uint16_t  m_u16Crc;
} tagTPMPacket;



/*--------------------------------- MCU -> SOC End ---------------------------------*/



/*--------------------------------- SOC -> MCU Begin -------------------------------*/
typedef struct {
    tagPacketHead m_objPackHead;//command:0x8000
    uint8_t  m_u8Reserve1;
    uint8_t  m_u8Reserve2;
    uint8_t  m_u8CarModel;
    uint16_t m_u16Crc;
}reqMCUShakeHandCtrl;

//发动机控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8001
    uint16_t m_u16EngineSpeed;//当前发动机最大转速
    uint8_t  m_u8DPFProhibitSwitchStatus;//DPF再生禁止开关状态
    uint8_t  m_u8DPFSwitchStatus;//DPF再生开关状态
    uint8_t  m_u8MultiType;//多态开关
    uint16_t m_u16Crc;
}reqEngineCtrl;

//空调控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8002
    uint8_t m_u8Temp;//温度
    uint8_t m_u8WindDirection;//风向
    uint8_t m_u8OffSwitch;//OFF开关
    uint8_t m_u8AUTOStatus;//AUTO状态
    uint8_t m_u8TempUnit;//温度单位
    uint8_t m_u8LoopStatus;//内外循环状态
    uint8_t m_u8WindSpeed;//风速
    uint8_t m_u8AcCtrl;//压缩机控制
    uint16_t m_u16Crc;
}reqACCtrl;


//电控抬升控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8003
    uint16_t m_u16TillingDepth;//耕深
    uint8_t  m_u8UpLimit;//高度上限
    uint8_t  m_u8DownSpeed;//下降速度
    uint8_t  m_u8MixturePot;//力液混合
    uint8_t  m_u8RockerSwitch;//模式开关
    uint8_t  m_u8DampingKey;//减震开关
    uint16_t m_u16Crc;
}reqElectricControlLiftingCtrl;

//液压阀控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8004
    ValveProperty m_objValve1;
    ValveProperty m_objValve2;
    ValveProperty m_objValve3;
    ValveProperty m_objValve4;
    ValveProperty m_objValve5;
    ValveProperty m_objValve6;
    uint16_t m_u16Crc;
} reqHydraulicValveCtrl ;

//变速箱控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8005
    uint8_t  m_u8AutoGear;//自动换挡设置状态
    uint16_t m_u16MaxVehicleSpeed;//最高车速
    uint8_t  m_u8StartGear;//起步档位
    uint8_t  m_u8PTOStatus;//PTO启停状态
    uint8_t  m_u8PTOGear;//PTO档位
    uint8_t  m_u8RearLiftHight ;//后提升指定高度
    uint16_t m_u16Crc;
}reqGearboxCtrl;

//灯光编程控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8006
    uint8_t  m_u8LedProgramMode;//当前灯光编程模式，0：其他；1：灯光编程A;2：灯光编程B，切换到对应灯光编程模式后下发，AB都不开启发0
    uint8_t  m_u8FrontCeilingLed;//前顶棚
    uint8_t  m_u8BackCeilingLed;//后顶棚
    uint8_t  m_u8WorkLedLed;//前工作灯
    uint8_t  m_u8FrontWaistLine;//腰线前工作灯
    uint8_t  m_u8BackWaistLine;//腰线后工作灯
    uint8_t  m_u8PositionLed;//位置灯
    uint8_t  m_u8PeriodTime;//延时断电时间
    uint16_t m_u16Crc;
}reqLedProgramCtrl ;

//天窗控制
typedef struct {
    tagPacketHead m_objPackHead;//command:0x8007
    uint8_t m_u8Cmd;//天窗控制
    uint16_t m_u16Crc;
}reqSkiyLightCtrl;

//后视镜控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8008
    uint8_t  m_u8RMSwitch; //后视镜调节开关
    uint8_t  m_u8RMvertical; //后视镜方向调节垂直
    uint8_t  m_u8RMhorizon; //后视镜方向调节水平
    uint8_t  m_u8Heating;//后视镜加热状态
    uint16_t m_u16Crc;
} reqRearviewMirrorCtrl;


//氛围灯控制
typedef struct {
    tagPacketHead m_objPackHead;//command:0x8009
    uint8_t m_u8Switch;//氛围灯开关
    uint8_t m_u8Mode;//氛围灯模式
    uint8_t m_u8Color;//氛围灯颜色
    uint16_t m_u16Crc;
}reqAtmosphereLightsCtrl;


//中控设置同步控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x800A
    uint8_t  m_u8OddUnit : 2; //里程单位
    uint8_t  m_u8TempUnit : 2; //温度单位
    uint8_t  m_u8TimeUnit : 2; //时间制式
    uint8_t  m_u8TireUnit : 2; //胎压单位
    uint8_t  m_u8ScreenBrightness : 4;//屏幕亮度
    uint8_t  m_u8Language : 4; //语言
    uint32_t m_u32Time; //时间戳
    uint8_t  m_u8DarkDay : 2;//白天黑夜模式 
    uint8_t  m_u8AutoScreenOff : 4;//主题
    uint8_t  : 2;//保留
    uint16_t m_u16Crc;
} reqIVISettingCtrl ;


//中控控制仪表指令
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x800B
    uint8_t m_u8Cmd;
    uint16_t m_u16Crc;
} reqIVICtrlSOC;


//记亩作业设置
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x800C
    uint8_t   m_u8Cmd;//作业控制，00结束，01开始，0xff无效值
    uint16_t  m_u16LeftWidth;//左幅宽参数
    uint16_t  m_u16RightWidth;//右幅宽参数
    uint8_t   m_u8WorkType;//作业控制，作业类型
    uint16_t m_u16Crc;
} reqWorkCtrl;

typedef struct  {
    tagPacketHead m_objPackHead;//command:0x800D
    uint8_t  m_u8FrontAxleSensorMiddle : 1;//前桥转角传感器中位标定
    uint8_t  m_u8FrontAxleSensorLeft : 1;//前桥转角传感器左极限标定
    uint8_t  m_u8FrontAxleSensorRight : 1;//前桥转角传感器右极限标定
    uint8_t  m_u8PTOHGearAngle : 1;//PTO-H档角度
    uint8_t  m_u8PTOMGearAngle : 1;//PTO-M档角度
    uint8_t  m_u8PTOLGearAngle : 1;//PTO-L档角度
    uint8_t  m_u8PTONGearAngle : 1;//PTO-N档角度
    
    uint8_t  m_u8AuxiliaryGearBoxHGearAngle : 1;//副变速箱H档角度
    uint8_t  m_u8AuxiliaryGearBoxLGearAngle : 1;//副变速箱L档角度
    uint8_t  m_u8AuxiliaryGearBoxNGearAngle : 1;//副变速箱N档角度
    uint8_t  m_u8ClutchAngleSensorReleas : 1;//离合器角度传感器松开
    uint8_t  m_u8ClutchAngleSensorFloor : 1;//离合器角度传感器踩到底
    uint8_t  : 4;
    
    uint8_t  m_u8PageNum;//参数标定页码编号
    uint8_t  m_u8ParamNum;//参数标定编号
    uint16_t  m_u8Param;//标定参数值
    uint16_t m_u16Crc;         // 前8字节的crc16值
} reqCalibrationRequest4HCtrl;

//SOC通知MCU重启
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x800E
    uint8_t  m_u8Reserve;   //保留字节
    uint16_t m_u16Crc;         // 前8字节的crc16值
} reqSOCCtrlMCU;



//SOC测试工装控制
typedef struct  {
    tagPacketHead m_objPackHead;//command:0x8081
    uint16_t  m_u16Cmd;//命令码
    uint8_t   m_u8Response[64];//命令参数
    uint16_t  m_u16Crc;
} reqTPMCtrl;

/*--------------------------------- SOC -> MCU End -------------------------------*/


typedef struct  {
    tagPacketHead m_objPackHead;//command:0xB000
    uint16_t m_u16LocalVer;   //本地MCU的版本信息
    uint16_t m_u16Crc;         // 前8字节的crc16值
} M2C_Msg_Head;


#pragma pack()