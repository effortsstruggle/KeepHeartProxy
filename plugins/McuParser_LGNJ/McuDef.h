#pragma once

//used for ieai_mcu_get_int(MCU_INDEX index)
enum MCU_INDEX {

    //MCU与SOC握手
    MI_MCU_VERSION = 0x0000,  //MCU版本号 主版本* 10000 + 次版本 * 100 + 调试版本                          //mcu版本
    MI_HARD_VERSION , //硬件版本



    //发动机
    MI_MAX_RPM ,     //当前发动机最大转速 （ 实际值 = * 0.125rpm ）
    MI_REAL_TIME_RPM,       //实时动力传输轴转速  （ 实际值 = * 0.125rpm ）
    MI_REAL_WORK_HOURS,     //发动机累计工作小时数显示 （ 单位小时 实际值= * 0.05 ）
    MI_DPF_DISABLE,         //DPF再生禁止开关状态 （ 0x00动力，0x01标准，0x02经济，0x03无载 ）
    MI_DPF_ENABLE,          //DPF再生开关状态 （ 00灯灭 01灯亮 ）
    MI_POLY_STATUS,         //多态开关状态 ( 0x00动力，0x01标准，0x02经济，0x03无载 )   

    //空调
    MI_AC_TEMP,             //空调温度*2 （（在显示时，可以将接收到的数据/2 即是相应的数据）） 
    MI_AC_TEMP_UINT,        //空调温度单位 （0：华氏温度，1：摄制温度）
    MI_AC_DIR,              //空调风向 （1：全模式，2：吹脸吹背，3:吹脚,4:吹脸吹脚除霜,5:吹脸除霜）
    MI_AC_STATUS,           //空调AC状态 (0:关闭，1：打开)
    MI_AC_AUTO_STATUS,      //空调AUTO状态 (0:关闭，1：打开)
    MI_AC_OFF_STATUS,       //空调OFF状态(0:关闭，1:打开)
    MI_AC_MODE,             //空调内外循环状态(0:外循环，1：内循环)
    MI_AC_SPEED,            //空调风速（0：无，1：1档；2：2档，3：3档，4：4档，5：5档，6：6档）
    MI_AC_GEAR,              //空调档位（对应温度18-32，数据范围1-29）

    //电控抬升
    MI_POWER_LIFT_PLOW_DEPTH,           //耕深设置, （0x00..0x03E8 - (1000)   Fault - 0xFFFE  Not Available - 0xFFFF）
    MI_POWER_LIFT_IND_LIGHT_UP,           //指示灯 ，上升 （00-Off,01-On,02-Error,03-Not available）
    MI_POWER_LIFT_IND_LIGHT_DOWN,           //指示灯 ，下降 （00-Off,01-On,02-Error,03-Not available）
    MI_POWER_LIFT_IND_LIGHT_SHOCK,           //指示灯 ，减震 （00-Off,01-On,02-Error,03-Not available）
    MI_POWER_LIFT_IND_LIGHT_ERROR,           //指示灯 ，错误 （00-Off,01-On,02-Error,03-Not available）
    MI_POWER_LIFT_FAULT_CODE,           //故障码,参考POWER_LIFT_FAULT_CODE
    MI_POWER_LIFT_HEIGHT_LIMIT,         //高度上限，（0x00..0xFA , Fault - 0xFE, Not Available - 0xFF）
    MI_POWER_LIFT_DESCEND_SPEED,         //下降速度，（0x00..0xFA , Fault - 0xFE, Not Available - 0xFF）
    MI_POWER_LIFT_FORCE_BLEND,            //力位混合，（0x00..0xFA , Fault - 0xFE, Not Available - 0xFF）
    MI_POWER_LIFT_MODE_SWITCH,            //模式开关，（）
    MI_POWER_LIFT_SHOCK_SWITCH,           //减震开关，（0x0= 关闭 ,0x1=开启,0x2= Fault 0x3= Error）


    //液压阀
    MI_CONTROL_VALUE_1_MAX_FLOW,         //控制阀1最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_1_PROGREAAION,      //控制阀1渐进曲线（0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_1_MAX_TIME,         //控制阀1最大时间（0x00 - 0x1770，0-60000ms）

    MI_CONTROL_VALUE_2_MAX_FLOW,         //控制阀2最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_2_PROGREAAION,      //控制阀2渐进曲线（0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_2_MAX_TIME,         //控制阀2最大时间（0x00 - 0x1770，0-60000ms）

    MI_CONTROL_VALUE_3_MAX_FLOW,         //控制阀3最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_3_PROGREAAION,      //控制阀3渐进曲线（0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_3_MAX_TIME,         //控制阀3最大时间（0x00 - 0x1770，0-60000ms）

    MI_CONTROL_VALUE_4_MAX_FLOW,         //控制阀4最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_4_PROGREAAION,      //控制阀4渐进曲线 （0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_4_MAX_TIME,         //控制阀4最大时间 （0x00 - 0x1770，0-60000ms）

    MI_CONTROL_VALUE_5_MAX_FLOW,         //控制阀5最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_5_PROGREAAION,      //控制阀5渐进曲线 （0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_5_MAX_TIME,         //控制阀5最大时间 （0x00 - 0x1770，0-60000ms）

    MI_CONTROL_VALUE_6_MAX_FLOW,         //控制阀6最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MI_CONTROL_VALUE_6_PROGREAAION,      //控制阀6渐进曲线 （0x00 - 0x03F，0-63）
    MI_CONTROL_VALUE_6_MAX_TIME,         //控制阀6最大时间 （0x00 - 0x1770，0-60000ms）



    //变速箱
    MI_GEAR_BOX_GEAR,                    //当前档位(00=PST档位 R3档;01=PST档位 R2档;02=PST档位 R1档;03=PST档位 空档;04-09:PST档位 F1-F6档)
    MI_GEAR_BOX_CAR_MODE,                //整车模式(0:手动模式;1:田间模式;2运输模式)
    MI_GEAR_BOX_AUTO_SHIFT,              //自动换档设置状态（0：关；1：开；ff:无动作）
    MI_GEAR_BOX_CURRENT_SPEED,          //当前车速
    MI_GEAR_BOX_START_GEAR,              //起步档位
    MI_GEAR_BOX_DIFF_LOCK,              //手自动差速锁显示状态（0 = 差速锁脱开；1 = 差速锁接合）
    MI_GEAR_BOX_FOUR_WHEEL_DRIVE,       //手自动四驱显示 （0 = 四驱脱开；1= 四驱接合）
    MI_PTO_CONTROL,                     //PTO启停控制状态 （00 = PTO离合器脱开；01 = PTO离合器接合）
    MI_PTO_RPM,                         //PTO转速控制：（0-空档；1-1档；2-2档；3-三档）

    // MI_GEAR_BOX_MAX_SPEED,               //最高车速

   
    //后视镜
    MI_REAR_MIRROR_STATUS,               //后视镜加热状态(0:关闭；1:打开)

    //氛围灯
    MI_AMBIENT_LIGHT,                            //氛围灯开关(0:无效；1:关闭；2：打开)
    MI_AMBIENT_LIGHT_MODE,                       //氛围灯模式(0:无效；1:长亮；2：流水；3：呼吸；4：动感)
    MI_AMBIENT_LIGHT_COLOR,                      //氛围灯颜色 (0x00 = 无效值 0x01: Number 1 0x02: Number 2 0x03: Number 3 0x04: Number 4 ... 0xFF: 随机颜色)

    //仪表
    MI_METER_SENSITIVITY,                        //仪表采集光感值 (LUX)
    MI_METER_BRIGHT_LEVEL,                       //仪表屏幕亮度 ， 0x00 无效值 0x01 亮度等级1 0x02 亮度等级2 0x03 亮度等级3 0x04 亮度等级4 0x05 亮度等级5

    //TBOX时间
    MI_TIME_YEAR,                                //年
    MI_TIME_MONTH,                               //月
    MI_TIME_DAY,                                 //日
    MI_TIME_HOUR,                                //时
    MI_TIME_MINUTE,                              //分
    MI_TIME_SECOND,                              //秒                    

    //作业信息
    MI_WORK_ACCUMULATED_ACREAGE, //累计亩数(精度：0.01亩  数据类型：float)
    MI_WORK_CURRENT_ACREAGE,//当前亩数(精度：0.01亩  数据类型：float)
    MI_WORK_LEFT_WIDTH_PARAM, //左幅宽参数	单位cm
    MI_WORK_RIGHT_WIDTH_PARAM,//右幅宽参数	单位cm
    MI_WORK_ERROR_CODE ,//错误码	设置计亩设置状态反馈


    //MCU->SOC下电
    MI_MCU_POWER_OFF_NOTIY , //MCU下电通知

    //油耗信息
    MI_FUEL_CONSUMPTION_CURRENT , //本次油耗
    MI_FUEL_CONSUMPTION_TOTAL , //总油耗

    //4h标定参数信息
    MI_4H_CALIBRATION_ARG_1 , //当前标定设置页面参数1
    MI_4H_CALIBRATION_ARG_2 , //当前标定设置页面参数2
    MI_4H_CALIBRATION_ARG_3 , //当前标定设置页面参数3
    MI_4H_CALIBRATION_ARG_4 , //当前标定设置页面参数4
    MI_4H_CALIBRATION_ARG_5 , //当前标定设置页面参数5
    MI_4H_CALIBRATION_ARG_6 , //当前标定设置页面参数6
    MI_4H_CALIBRATION_ARG_7 , //当前标定设置页面参数7
    MI_4H_CALIBRATION_ARG_8 , //当前标定设置页面参数8
    MI_4H_CALIBRATION_FRONT_AXLE_SENSOR , //前桥转角传感器实际值
    MI_4H_CALIBRATION_PTO_GREA_SENSOR , //PTO档位传感器实际值
    MI_4H_CALIBRATION_AUXILIARY_GEARBOX_SENSOR , //副变速箱档位角度传感器实际值
    MI_4H_CALIBRATION_MAIN_CLUTCH_SENSOR, //主离合器传感器实际值


    //TCU故障码
    MI_TCU_FAULT_DEVICE_ADDR , //TCU 故障设备地址
    MI_TCU_FAULT_CODE , //TCU故障码
    MI_TCU_FAULT_NUMBER ,  //TCU故障次数

};

//used for ieai_mcu_set(MCU_COMMAND cmd, int data1, long data2)
enum MCU_COMMAND {

    //SOC通知MCU
    MC_SOC_NOTIFY_MCU_ON = 0x8000 ,                      //SOC通知mcu程序已启动 data1无效 ， data2:无效
    MC_SOC_NOTIFY_MCU_DIFFSPEEDRATIO,                    //SOC通知mcu程序后桥差速比 data1= ( 0-100, 0xff表示无效值) ， data2:无效
    MC_SOC_NOTIFY_MCU_REARWHEELRADIUS,                  //SOC通知mcu程序后轮半径 data1= (  单位cm , 0xff表示无效值) ， data2:无效
    MC_SOC_NOTIFY_MCU_CARMODEL ,                        //SOC通知mcu程序车型配置 data1= (  0xff表示无效值) ， data2:无效

    //发动机
    MC_SET_MAX_RPM ,   // 发动机最大转速 data1 = ( {转速/0.125} ) ,data2:无效
    MC_SET_DPF_DISABLE,      //DPF再生禁止开关设置data1 =（打开：1，关闭：0), data2:无效
    MC_SET_DPF_ENABLE,       //DPF再生开关设置data1 =（打开：1，关闭：0), data2:无效
    MC_SET_POLY_STATUS,         //多态开关状态data1 = (0x00动力，0x01标准，0x02经济，0x03无载) ,data2:无效

    //空调
    MC_SET_AC_TEMP,         //空调温度设置 data1 =（1-29,摄氏度：代表18-32摄氏度，华氏度：63-91), data2:无效
    MC_SET_AC_DIR,          //空调风向 data1=（1：全模式，2：吹脸吹背，3:吹脚,4:吹脸吹脚除霜,5:吹脸除霜), data2:无效
    MC_SET_AC_SWITCH,        //空调off开关 data1 = (00：无动作； 01：OFF 开机/关机；根据当前空调状态切换 ) , data2:无效
    MC_SET_AC_AUTO,          //AUTO设定 data1 =(00：无动作； 01：打开 AUTO 模式； 其余无效) , data2:无效
    MC_SET_AC_TEMP_UINT,    //摄氏华氏切换 data1 = (00：无动作； 01：摄氏华氏切换； ), data2 = 0，
    MC_SET_AC_MODE,         //内外循环切换；根据当前空调状态切换， data1 = ( 00：无动作，01：内外循环切换；）  data2 = 0 
    MC_AC_SPEED,            //空调风量 1:风量加，2，风量减；data1={1,2} data2:无效
    MC_SET_AC_STATUS,       //压缩机 开机/关机；根据当前空调状态切换，data1=（00：无动作；01：压缩机开关；） data2 = 0
    
    //电抬升
    MC_SET_POWER_LIFT_PLOW_DEPTH,           //耕深设置， data1 = ( 0x00..0x03E8 (0-1000) ； Fault - 0xFFFE ；  Not Available - 0xFFFF) data2 : 无效
    MC_SET_POWER_LIFT_HEIGHT_LIMIT,         //高度上限, data1 = (0x00..0xFA(0-250), Fault -- 0xFE, Not Available -- 0xFF) data2 : 无效
    MC_SET_POWER_LIFT_DESCEND_SPEED,         //下降速度 ，data1 = (0x00..0xFA(0-250), Fault -- 0xFE, Not Available -- 0xFF) data2 : 无效
    MC_SET_POWER_LIFT_FORCE_BLEND,            //力位混合， data1 = (0x00..0xFA(0-250), Fault -- 0xFE, Not Available -- 0xFF) data2 : 无效
    MC_SET_POWER_LIFT_MODE_SWITCH,            //模式开关，data1 = (0x0=Stop , 0x1=Transport 提升,0x2=Control下降,0x3=FastSink 快速接收 , 无操作: 0xE , Not Available: 0xF) data2 : 无效
    MC_SET_POWER_LIFT_SHOCK_SWITCH,          //减震开关，data1 = (0x0= 关闭 ,0x1=开启,0x2= Fault 0x3= Error) data2 : 无效

    //液压阀控制
    MC_SET_CONTROL_VALUE_1_MAX_FLOW ,         //控制阀1最大流量 data1 =（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_1_PROGREAAION,      //控制阀1渐进曲线（0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_1_MAX_TIME,         //控制阀1最大时间（0x00 - 0x1770，0..6000 meams 0 - 60000ms ）
    MC_SET_CONTROL_VALUE_2_MAX_FLOW ,         //控制阀2最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_2_PROGREAAION,      //控制阀2渐进曲线（0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_2_MAX_TIME,         //控制阀2最大时间（0x00 - 0x1770，0..6000 meams 0 - 60000ms）
    MC_SET_CONTROL_VALUE_3_MAX_FLOW ,         //控制阀3最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_3_PROGREAAION,      //控制阀3渐进曲线（0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_3_MAX_TIME,         //控制阀3最大时间（0x00 - 0x1770，0..6000 meams 0 - 60000ms）
    MC_SET_CONTROL_VALUE_4_MAX_FLOW ,         //控制阀4最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_4_PROGREAAION,      //控制阀4渐进曲线 （0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_4_MAX_TIME,         //控制阀4最大时间 （0x00 - 0x1770，0..6000 meams 0 - 60000ms）
    MC_SET_CONTROL_VALUE_5_MAX_FLOW ,         //控制阀5最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_5_PROGREAAION,      //控制阀5渐进曲线 （0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_5_MAX_TIME,         //控制阀5最大时间 （0x00 - 0x1770，0..6000 meams 0 - 60000ms）
    MC_SET_CONTROL_VALUE_6_MAX_FLOW ,         //控制阀6最大流量（0x00 - 0x3E8，0-1000） means 0 - 1000‰
    MC_SET_CONTROL_VALUE_6_PROGREAAION,      //控制阀6渐进曲线 （0x00 - 0x03F，0-63）
    MC_SET_CONTROL_VALUE_6_MAX_TIME,         //控制阀6最大时间 （0x00 - 0x1770，0..6000 meams 0 - 60000ms）


    //变速箱
    MC_SET_GEAR_BOX_AUTO_SHIFT,              //自动换档设置状态 data1 =（0：关；1：开；ff:无动作）
    MC_SET_GEAR_BOX_MAX_SPEED,               //最高车速设置 data1 =（ 单位km/h 0xffff =无效值 ）
    MC_SET_GEAR_BOX_START_GEAR,              //起步档位(0-6:0-6档)
    MC_SET_PTO_CONTROL,                     //PTO启停控制状态 （00 =松开，1 = PTO离合器接合；2 = PTO离合器松开）
    MC_SET_PTO_RPM,                          //PTO转速控制：（0-松开，1-空档；2-1档；3-2档；4-三档）
    MC_SET_REAR_LIFT_HIGHT,                 //后提升指定高度


    //灯光编程
    MC_SET_LED_PROGRAM_MODE ,               //灯光编程模式
    MC_SET_FRONT_WORK_LIGHT,                     //前顶棚工作灯信号(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_REAR_WORK_LIGHT,                      //后顶棚工作灯信号(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_FRONT_LIGHT,                             //前工作灯信号(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_LINE_FRONT_WORK_LIGHT,                   //腰线前工作灯信号(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_LINE_REAR_WORK_LIGHT,                    //腰线后工作灯信号(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_POSITION_LIGHT,                          //位置信号灯(未按下 - 00; 按下 - 01;  不可用 - 10; 不可用 - 11;)
    MC_SET_LIGHT_DELAY_POWER_OFF_TIME,              //灯光延时关闭时间设置(000:延时关闭；001::延时10s；010:延时30s；011：延时60s；100：延时90s；
                                                    //101：不可用；110：不可用；111：不可用)
    //灯光总开关
    MC_SET_LIGHT_STATE,                            //,data1,每个灯光占两位,分别对应(前顶棚工作灯信号,后顶棚工作灯信号,前工作灯信号,腰线前工作灯信号,腰线后工作灯信号,位置信号灯),dat2,灯光延时时间

    //天窗控制
    MC_SET_SKY_LIGHT_ADJUST,                             //天窗控制 data1 = （0;  1; 2; 3;00 :按键松开;01 ：起翘;02 ：打开;03 ：关闭）

    //后视镜调节
    MC_SET_LEFT_REAR_MIRROR_VERTICAL,                //左后镜垂直（data1 = 0; data1 = 1;0:松开，1：垂直调节）
    MC_SET_RIGHT_REAR_MIRROR_VERTICAL,               //右后镜垂直（data1 = 0; data1 = 1;0:松开，1：垂直调节）
    MC_SET_LEFT_REAR_MIRROR_HORIZONTAL,              //左后镜水平（data1 = 0; data1 = 1;0:松开，1：水平调节）
    MC_SET_RIGHT_REAR_MIRROR_HORIZONTAL,             //右后镜水平（data1 = 0; data1 = 1;0:松开，1：水平调节）
    MC_SET_REAR_MIRROR_HEAT,                         //后镜加热开关（data1 = 0; data1 = 1;0:关闭，1：打开） 

    
    //氛围灯
    MC_SET_AMBIENT_LIGHT,                            //氛围灯开关(0:无效；1:关闭；2：打开)
    MC_SET_AMBIENT_LIGHT_MODE,                       //氛围灯模式(0:无效；1:长亮；2：流水；3：呼吸；4：动感)
    MC_SET_AMBIENT_LIGHT_COLOR,                       //氛围灯颜色（0x00 = 无效值 ， 0x01: Number 1 ，0x02: Number 2 ，0x03: Number 3 ，
                                                        //  0x04: Number 4 ， 。。。 ， 0xFF: 随机颜色 ）
    //中控设置同步
    MC_SET_METER_UNIT_LENGTH,                             // 长度格式（里程单位）       0x00 公制；0x01 英制；0x03 无效值 仪表保持上次开机值处理，或默认值
    MC_SET_METER_UNIT_TEMPER,                             // 温度格式       0x00 摄氏度；0x01 华氏度；0x03 无效值
    MC_SET_METER_TIME_FORMAT,                             // 时间格式       0x00 24小时制；0x01 12小时制；0x03 无效值
    MC_SET_TPMS_UNIT_PRESSURE,                             //设置胎压单位   ：0x00 bar；0x01 PSI；0x02 kPa；0x03 无效值
    MC_SET_METER_BRIGHT_LEVEL,                            //仪表屏幕亮度调节模式0-3：0x00 自动；0x01 手动1；0x02 手动2 0x0A 手动10 0x0F 无效值 仪表保持上次开机值处理，或默认值
    MC_SET_METER_LANGUAGE,                                // 语言          0x00 中文；0x01 英文；0x02 俄文；0x0F 无效值
    MC_SET_METER_TIME,                                    //从1970到当前时间的时间戳，单位s
    MC_SET_DAY_MODEL ,                                    //设置白天黑夜模式  0-1bit：0x00 白天；0x01 黑夜；0x03 无效值
    MC_SET_METER_THEME,                                   // 主题模式  2-5bit：0x00 默认主题；0x0F 无效值      

    //中控控制仪表指令
    MC_SET_METER_OTA ,                                   //通知仪表OTA升级  0x00无效 0x01 仪表打开升级页面

    //作业设置
    MC_SET_WORK_CTRL , //作业控制，data1 =( 00结束 ，01开始 , 0xff无效值)
    MC_SET_Left_WIDTH_PARAMETER , //左幅宽参数 data1 = ( 单位cm,0xffff表示无效 ) data2:无效
    MC_SET_RIGHT_WIDTH_PARAMETER , //右幅宽参数 data1 = ( 单位cm,0xffff表示无效 ) data2:无效
    MC_SET_WORK_TYPE , //作业类型 data1 = ( 0x00 : 作业计亩 , 0x01 : 圈点计亩 , 0xff : 无效值 ) , data2：无效 


    //4H标定设置
    MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_MIDDLE , //前桥转角传感器中位标定
    MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_LEFT , //前桥转角传感器左极限标定
    MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_RIGHT , //前桥转角传感器右极限标定
    MC_4H_CALIBRATION_PTO_H_GEAR_ANGLE , //PTO-H档角度
    MC_4H_CALIBRATION_PTO_M_GEAR_ANGLE , //PTO-M档角度
    MC_4H_CALIBRATION_PTO_L_GEAR_ANGLE , //PTO-L档角度
    MC_4H_CALIBRATION_PTO_N_GEAR_ANGLE , //PTO-N档角度
    MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_H_GEAR_ANGLE , //副变速箱H档角度
    MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_L_GEAR_ANGLE , //副变速箱L档角度
    MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_N_GEAR_ANGLE , //副变速箱N档角度
    MC_4H_CALIBRATION_CLUTCH_ANGLE_SENSOR_RELEAS , //离合器角度传感器松开
    MC_4H_CALIBRATION_CLUTCH_ANGLE_SENSOR_FLOOR , //离合器角度传感器踩到底 
    MC_4H_CALIBRATION_PAGE_NUM , //参数标定页码编号
    MC_4H_CALIBRATION_PARAM_NUM_AND_VALUE , //参数标定编号/参数值

    //SOC通知MCU重启
    MC_SET_MCU_REBOOT ,

    //工装测试
    MC_SET_TPM_BT, //蓝牙检测
    MC_SET_TPM_WIFI, //WIFI检测
    MC_SET_TPM_RGB, //RGB检测
    MC_SET_TPM_SOFTVERSION , //软件版本检测
    MC_SET_TPM_BRIGHTNESS , //背光亮度
    MC_SET_TPM_EC_VERSION , //互联版本
    MC_SET_TPM_LANGUAGE , //语言设置
    MC_SET_TPM_VOICE_CHIPS, //语音芯片
    MC_SET_TPM_CURRENT_CARTYPE , //当前车型
    MC_SET_TPM_MODEIFY_CARTYPE , //修改车型
    MC_SET_TPM_NORMAL_SHOW  , //恢复正常显示
    MC_SET_TPM_GET_SN , //获取SN
};

//used for onMcuNotify(MCU_NOTIFY code, int data1, long data2)
enum MCU_NOTIFY {
    MN_ERR = 200,       //data1=err, data2=0
    MN_CHARGE,          //data1=charge, data2=0
    MN_KEY,              //data1=key, data2=0
    MCU_NOTIFY_OTA_RESULT, //OTA升级结果
    MCU_NOTIFY_TIME ,      //通知TBOX时间
    MCU_NOTIFY_LIGHTPROGRAM_STATE , //灯管编程开关状态
    MCU_NOTIFY_HIGH_SPEED_SETTING_STATUS ,  //最高车速设置状态 
    MCU_NOTIFY_4H_CALIBRATION_STATUS, //4H标定状态通知
    MCU_NOTIFY_TPM_STATUS, //TPM测试工装通知
    MCU_NOTIFY_7E_FAULT_CODE //7E故障码
};


enum TPM_TEST_COMMAND{
    TPM_TEST_BT = 0x0000, //蓝牙检测子命令
    TPM_TEST_WIFI = 0x0002 , //wifi检测子命令
    TPM_TEST_RGB = 0x0003 , //RGB检测子命令
    TPM_TEST_SOFTVERSION = 0x0009 , //软件版本检测子命令
    TPM_TEST_GET_SN = 0x000A , // 读取SN
    TPM_TEST_BRIGHTNESS = 0x000B , //背光亮度检测子命令
    TPM_TEST_EC_VERSION = 0x000D, //互联版本检测子命令
    TPM_TEST_LANGUAGE = 0x0012 , //语言设置检测子命令
    TPM_TEST_VOICE_CHIPS = 0x0014, //语音芯片
    TPM_TEST_NORMAL_SHOW = 0x00FF, //恢复正常显示
    TPM_TEST_CURRENT_CARTYPE = 0x0100 , //当前车型
    TPM_TEST_MODEIFY_CARTYPE = 0x0200 , //修改车型
    
};

enum POWER_LIFT_FAULT_CODE{
    POWER_LIFT_FAULT_CODE_EHC                                  = 0x01,  //EHC阀门严重故障
    POWER_LIFT_FAULT_CODE_LIFT_BUTTON                          = 0x08, //提升按钮故障
    POWER_LIFT_FAULT_CODE_LOWER_BUTTON                         = 0x10, //下降按钮故障
    POWER_LIFT_FAULT_CODE_ECU                                  = 0x03, //ECU严重故障
    POWER_LIFT_FAULT_CODE_CONTROL_SWITCH                       = 0x80, //主控制开关故障
    POWER_LIFT_FAULT_CODE_EHC_EHS                              = 0x100, // EHC阀门EHS介质故障
    POWER_LIFT_FAULT_CODE_POSITION_SENSOR                      = 0x200, //位置传感器故障
    POWER_LIFT_FAULT_CODE_SETPOINT                             = 0x400, //设定点电位计故障
    POWER_LIFT_FAULT_CODE_POSITION_LIMIT                       = 0x800, // 位置限制电位计故障
    POWER_LIFT_FAULT_CODE_RIGHT_FORCE                          = 0x1000, //右侧力传感器故障 
    POWER_LIFT_FAULT_CODE_LEFT_FORCE                           = 0x2000, //左力传感器（或上力传感器）故障
    POWER_LIFT_FAULT_CODE_RIGHT_FORCE_OVERLOAD                 = 0x10000, //右侧力传感器过载
    POWER_LIFT_FAULT_CODE_LEFT_FORCE__OVERLOAD                 = 0x20000, //左力传感器（或上力传感器）过载
    POWER_LIFT_FAULT_CODE_ECU_MIONR                            = 0x40000, // ECU轻微故障
    POWER_LIFT_FAULT_CODE_SPEED_LIMIT                          = 0x80000, // 限速电位计故障
    POWER_LIFT_FAULT_CODE_DAMPING_BUTTON                       = 0x100000, // 阻尼按钮故障
    POWER_LIFT_FAULT_CODE_MIX_POTENTIOMETER                    = 0x200000, // 混合电位计故障
    POWER_LIFT_FAULT_CODE_POSITION_BLOCK                       = 0x4000000, // 位置块
    POWER_LIFT_FAULT_CODE_POSITION_SENSOR_NEED                 = 0x8000000, // 需要校准位置传感器
    POWER_LIFT_FAULT_CODE_HLS1                                 = 0x80000000, // HLS1阀门故障
};



typedef void (*OnMcuNotify)(MCU_NOTIFY code, int data1, long data2);

typedef void (*OnMcuNotifyStr)(MCU_NOTIFY code, int data1, std::string&);