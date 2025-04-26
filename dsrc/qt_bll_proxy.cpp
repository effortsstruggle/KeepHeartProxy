#include <linux/input.h>
#include <fcntl.h>
#include "yps_ec.h"
#include "qt_bll_proxy.h"
#include "yps_app_config.h"
#include "yps_mcu.h"
#include "yps_bll.h"
#include "yps_bt.h"
#include "yps_dataset.h"
#include "yps_type.h"
#include "yps_tpms.h"
#include "prj_if.h"
#include "bllLog.h"
//#include "DVR/include/dvrApi.h"
#include "dvrApi.h"
#include <QObject>
#include <QTime>
#include <iostream>
#include <fstream>
#include <QThread>
#include <sched.h>
#include <mutex>
#include <QProcess>
//#include <QtQuick>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QWindow>
//#include "carbitUtil/properties.h"
#include "properties.h"
#include "utils.h"
// #include "mcu.h"
//#include "McuDef.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "skylog.h" 
#define IEAI
static std::mutex s_init_mtx; // 声明互斥锁
static Qt_bll_proxy *s_instance = NULL;
#define __output(...) \
    printf(__VA_ARGS__);
 
#define __format(__fmt__) "%s(%d)-<%s>: " __fmt__ "\n"
 
#define SkyTRACE(__fmt__, ...) \
    __output(__format(__fmt__), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

extern "C" void yps_bll_bt_notify(yps_bt_index_t index, yps_notify_bt_type_t type, uint32_t code, uintptr_t value)
{
//    printf("yps_bll_bt_notify ");
    if (s_instance != NULL)
    {
        int MAX_PAIRED_DEVICE_SIZE = 8;
        yps_bt_device_info_p p_paired_list;
        QString str_value;
        switch (type)
        {
        case YPS_NOTIFY_BT_MUSIC_UPDATE:
            if (value)
            {
                yps_bt_music_info_p _music_info;
                _music_info = (yps_bt_music_info_p)value;
                QJsonObject item;
                item.insert("play_state", (int)(_music_info->play_state));
                item.insert("track_tick_total", (int)(_music_info->track_tick_total));
                item.insert("track_tick_current", (int)(_music_info->track_tick_current));
                item.insert("track_volume", (int)(_music_info->track_volume));
                item.insert("track_title", QString::fromUtf8((char *)_music_info->track_title));
                item.insert("track_artist", QString::fromUtf8((char *)_music_info->track_artist));
                item.insert("track_album", QString::fromUtf8((char *)_music_info->track_album));
                QJsonDocument jsonDoc(item);
                str_value = jsonDoc.toJson();
            }
            break;
        case YPS_NOTIFY_BT_PLIST_CHANGED:
        case YPS_NOTIFY_BT_SCAN_LIST_CHANGED:
        case DI_BT_BLUETOOTH_DEVICE_LIST:
            if (value)
            {
                p_paired_list = (yps_bt_device_info_p)value;
                for (int i = 0; i < MAX_PAIRED_DEVICE_SIZE; i++)
                {
                    QJsonObject item;
                    item.insert("index", (int)(p_paired_list[i].index));
                    item.insert("signal", (int)(p_paired_list[i].signal));
                    item.insert("battery", (int)(p_paired_list[i].battery));
                    item.insert("ibr", (int)(p_paired_list[i].ibr));
                    item.insert("mac", QString::fromUtf8((char *)p_paired_list[i].mac));
                    item.insert("name", QString::fromUtf8((char *)p_paired_list[i].name));
                    item.insert("status", (int)(p_paired_list[i].status));
                    QJsonDocument jsonDoc(item);
                    str_value = str_value + jsonDoc.toJson();
                }
            }

            break;
        case YPS_NOTIFY_BT_NAME:
        case YPS_NOTIFY_BT_MAC:
        case YPS_NOTIFY_BT_BLE_NAME:
        case YPS_NOTIFY_BT_BLE_MAC:
            if (value)
            {
                str_value = QString::fromUtf8((char *)value);
            }
            break;
        case YPS_NOTIFY_BT_CALL_INFO_UPDATE:
            if (value)
            {
                yps_bt_call_log_p _call_log;
                _call_log = (yps_bt_call_log_p)value;
                QJsonObject item;
                item.insert("type", (int)(_call_log->type));
                item.insert("dateTime", QString::fromUtf8(_call_log->dateTime));
                item.insert("start", (int)(_call_log->start));
                item.insert("curTime", (int)(_call_log->curTime));
                item.insert("end", (int)(_call_log->end));
                item.insert("phone.name", QString::fromUtf8((char *)_call_log->phone.name));
                item.insert("phone.number", QString::fromUtf8((char *)_call_log->phone.number));
                item.insert("phone.number_code", (qint64)(_call_log->phone.number_code));
                QJsonDocument jsonDoc(item);
                str_value = jsonDoc.toJson();
            }
            break;

        default:
            break;
        }
        s_instance->on_yps_bll_bt_notify(index, type, code, str_value);
    }
}
extern "C" void yps_mcu_notify_handler(yps_notify_mcu_type_t type, uint32_t code, uintptr_t value)
{
    if (s_instance != NULL)

    {
        s_instance->on_yps_mcu_notify_handler(type, code, value);
    }
}
// extern "C" void ieai_mcu_notify_handler(MCU_NOTIFY code, int data1, long data2)
// {
    
//     if (s_instance != NULL)
//     {
//         s_instance->on_ieai_mcu_notify_handler(code, data1, data2);
//     }
// }


extern "C" void yps_key_event(uint32_t code, uint8_t status, uint32_t interval)
{
    Q_UNUSED(interval);
    if (s_instance != NULL)
    {
        s_instance->on_page_key_handler(code, status);
    }
}

extern "C" void yps_animation_status(int status)
{

    if (s_instance != NULL)
    {
        s_instance->on_yps_animation_status(status);
    }
}
extern "C" void yps_bll_notify_handler(yps_notify_bll_type_t type, unsigned int code, unsigned int value)
{

    if (s_instance != NULL)
    {
        s_instance->on_yps_bll_notify_handler(type, code, value);
    }
}

extern "C" void yps_ec_notify_handel(yps_notify_ec_type_t type, unsigned int code, unsigned int value)
{

    if (s_instance != NULL)
    {
        s_instance->on_yps_ec_notify_handel(type, code, value);
    }
}

void *Qt_bll_proxy::data_update_thread(void *p)
{

    Q_UNUSED(p);
    //Qt_bll_proxy *proxy = static_cast<Qt_bll_proxy*>(p);
    std::string line;
    for (int i = 0; i < 400; i++)
    {
        std::ifstream file("/dev/anim_status", std::ifstream::in);
        if (file.is_open())
        {
            std::getline(file, line);
            file.close();

            std::string status = "status=done";

            if (line == status)
            {
                platform_set_property("g_play_mp4_done", "1");
                break;
            }
        }
        QThread::usleep(50 * 1000);
    }
    yps_animation_status(1);

    return NULL;
}

Qt_bll_proxy *Qt_bll_proxy::getInstance()
{
    s_init_mtx.lock();
    if (s_instance == NULL)
    {
        s_instance = new Qt_bll_proxy();
        // qDebug() <<  "instance==================================" << tid_cpu_usage_thread;
    }
    s_init_mtx.unlock();
    return s_instance;
}
int Qt_bll_proxy::init(QString config)
{
    yps_bt_config_t _bt_config;
    _bt_config.index = YPS_BT_INDEX_PHONE;
    _bt_config.model = YPS_BT_MODEL_YPS;
    _bt_config.gpio_index = 0;
    _bt_config.option = YPS_BT_OPTION_SUPPORT_BLE | YPS_BT_OPTION_SUPPORT_PB;

    yps_bll_ec_init_param_t _ec_param;
    _ec_param.support_ec_hud = 1;
    _ec_param.ecScreenDirection = 0;
    _ec_param.ecVideoW = 1280;
    _ec_param.ecVideoH = 480;
    _ec_param.ecViewX = 80;
    _ec_param.ecViewY = 60;
    // _ec_param.ecViewWidth = 1200;
    // _ec_param.ecViewHeight = 432;
    _ec_param.ecSafeViewX = 0;
    _ec_param.ecSafeViewY = 0;
    // _ec_param.ecSafeViewWidth = 0;
    // _ec_param.ecSafeViewHeight = 0;
    _ec_param.ecFlavor = 0x01;
    _ec_param.ecMirrorMode = 0;
    _ec_param.ecProductType = 0;
    _ec_param.disablePageInRVMap = 0;
    
  //  printf("ahhahaha%d,%d,%d,%d\n",_ec_param.ecVideoW, _ec_param.ecVideoH, _ec_param.ecViewX, _ec_param.ecViewY);
    // memcpy(_ec_param.ecAppName,"net.easyconn.carman.wws",24);
    // memcpy(_ec_param.ecActivity,"net.easyconn.carman.MainActivity",33);
    // memcpy(_ec_param.ecAOAModel,"CarbitDriverOversea",20);
    // memcpy(_ec_param.ecAOAURL,"http://www.carbit.com.cn/down5/227",35);

    // _ec_param.ecFlavorOverser = 0x02;
    // memcpy(_ec_param.ecAppNameOverser ,"net.easyconn.carman.wws",24);
    // memcpy(_ec_param.ecActivityOverser,"net.easyconn.carman.MainActivity",33);
    // memcpy(_ec_param.ecAOAModelOverser,"CarbitDriverOversea",20);
    // memcpy(_ec_param.ecAOAURLOverser  ,"http://www.carbit.com.cn/down5/227",35);

    memcpy(_ec_param.ecAppName,"net.easyconn.carman.neutral",28);
    memcpy(_ec_param.ecActivity,"net.easyconn.carman.MainActivity",33);
    memcpy(_ec_param.ecAOAModel,"DriverModelNeutral",19);
    memcpy(_ec_param.ecAOAURL,"http://www.carbit.com.cn/down5/310",35);

    _ec_param.ecFlavorOverser = 0x01;
    memcpy(_ec_param.ecAppNameOverser ,"net.easyconn.carman.neutral",28);
    memcpy(_ec_param.ecActivityOverser,"net.easyconn.carman.MainActivity",33);
    memcpy(_ec_param.ecAOAModelOverser,"DriverModelNeutral",19);
    memcpy(_ec_param.ecAOAURLOverser  ,"http://www.carbit.com.cn/down5/310",35);


    char channelCode[20] = "66661505";

    char channelName[20] = "CPDM05";

    char otaSid[20] = "SID_QT_T507";
    char otaVersion[20] = "1.0.0";
    char appVersion[20] = "";
    char mcuPath[20] = "/dev/ttyS1";
    // char appConfPath[20] = "/data/app_conf.ini";
    yps_bll_default_para_t default_para;
    uint64_t supportFunction_1 = 0;
    yps_tpms_option_t tpms_option;
    char btPath[20] = "/dev/ttyS3";


    QJsonDocument jsonDoc = QJsonDocument::fromJson(config.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("btPath"))
    {
        strncpy(btPath, jsonObj.value("btPath").toString().toUtf8().data(), sizeof(btPath));
    }

    strncpy(_bt_config.path, btPath, sizeof(_bt_config.path));

    /* 配置默认参数 */
    memset(&default_para, 0, sizeof(default_para));
    if (jsonObj.contains("default_language"))
    {
        default_para.language = (yps_bll_lang_id_t)jsonObj.value("default_language").toInt();
    }

    if (jsonObj.contains("default_length_unit"))
    {
       default_para.unit_length = (yps_bll_length_unit_t)jsonObj.value("default_length_unit").toInt();
    }

    if (jsonObj.contains("default_temper_unit"))
    {
       default_para.unit_temper = (yps_bll_temper_unit_t)jsonObj.value("default_temper_unit").toInt();
    }

    if (jsonObj.contains("default_time_unit"))
    {
       default_para.uint_time = (yps_bll_time_format_t)jsonObj.value("default_time_unit").toInt();
    }
    
    if (jsonObj.contains("default_tpms_unit"))
    {
       default_para.unit_tpms = (yps_bll_tpms_unit_t)jsonObj.value("default_tpms_unit").toInt();
    }

    if (jsonObj.contains("default_drive_mode"))
    {
       default_para.driver_mode = (yps_bll_drive_mode_t)jsonObj.value("default_drive_mode").toInt();
    }

    if (jsonObj.contains("default_backlight_mode"))
    {
       default_para.backlight_mode = (yps_bll_backlight_mode_t)jsonObj.value("default_backlight_mode").toInt();
    }

    if (jsonObj.contains("default_backlight_level"))
    {
       default_para.backlight_level = (yps_bll_backlight_level_t)jsonObj.value("default_backlight_level").toInt();
    }

    if (jsonObj.contains("default_theme"))
    {
       default_para.theme = (yps_bll_theme_t)jsonObj.value("default_theme").toInt();
    }

    /* 配置胎压 */
    memset(&tpms_option, 0, sizeof(tpms_option));
    if (jsonObj.contains("tpms_input_mode"))
    {
        tpms_option.input_mode = (yps_tpms_input_t)jsonObj.value("tpms_input_mode").toInt();
    }

    if (jsonObj.contains("tpms_timeout"))
    {
        tpms_option.timeout = jsonObj.value("tpms_timeout").toInt();
    }

    if (jsonObj.contains("tpms_device_size"))
    {
        tpms_option.device_size = jsonObj.value("tpms_device_size").toInt();
    }

    if (jsonObj.contains("tpms_frontWarn_press_l"))
    {
        tpms_option.warn_limit.frontWarn_press_l = jsonObj.value("tpms_frontWarn_press_l").toInt();
    }

    if (jsonObj.contains("tpms_frontWarn_press_h"))
    {
        tpms_option.warn_limit.frontWarn_press_h = jsonObj.value("tpms_frontWarn_press_h").toInt();
    }

    if (jsonObj.contains("tpms_rearWarn_press_l"))
    {
        tpms_option.warn_limit.rearWarn_press_l = jsonObj.value("tpms_rearWarn_press_l").toInt();
    }

    if (jsonObj.contains("tpms_rearWarn_press_h"))
    {
        tpms_option.warn_limit.rearWarn_press_h = jsonObj.value("tpms_rearWarn_press_h").toInt();
    }

    if (jsonObj.contains("tpms_warn_temp_h"))
    {
        tpms_option.warn_limit.warn_temp_h = jsonObj.value("tpms_warn_temp_h").toInt();
    }

    tpms_option.data_index_warning_light = DI_MCU_LIGHT_TIRE_PRESS;
    tpms_option.data_index_id[0] = {DI_TPMS_LEFT_FRONT_ID};
    tpms_option.data_index_id[1] = {DI_TPMS_LEFT_REAR_ID};
    tpms_option.data_index_press[0] = {DI_TPMS_LEFT_FRONT_PRESS};
    tpms_option.data_index_press[1] = {DI_TPMS_LEFT_REAR_PRESS};
    tpms_option.data_index_press_flag[0] = {DI_TPMS_LEFT_FRONT_PRESS_FLAG};
    tpms_option.data_index_press_flag[1] = {DI_TPMS_LEFT_REAR_PRESS_FLAG};
    tpms_option.data_index_temp[0] = {DI_TPMS_LEFT_FRONT_TEMP};
    tpms_option.data_index_temp[1] = {DI_TPMS_LEFT_REAR_TEMP};
    tpms_option.data_index_temp_flag[0] = {DI_TPMS_LEFT_FRONT_TEMP_FLAG};
    tpms_option.data_index_temp_flag[1] = {DI_TPMS_LEFT_REAR_TEMP_FLAG};
    tpms_option.data_index_state[0] = {DI_TPMS_LEFT_FRONT_STATE};
    tpms_option.data_index_state[1] = {DI_TPMS_LEFT_REAR_STATE};


    if (jsonObj.contains("support_ec_hud"))
    {
        _ec_param.support_ec_hud = jsonObj.value("support_ec_hud").toInt();
    }

    if (jsonObj.contains("ecScreenDirection"))
    {
        _ec_param.ecScreenDirection = jsonObj.value("ecScreenDirection").toInt();
    }
    if (jsonObj.contains("ecVideoW"))
    {
        _ec_param.ecVideoW = jsonObj.value("ecVideoW").toInt();
    }
    if (jsonObj.contains("ecVideoH"))
    {
        _ec_param.ecVideoH = jsonObj.value("ecVideoH").toInt();
    }
    if (jsonObj.contains("ecViewX"))
    {
        _ec_param.ecViewX = jsonObj.value("ecViewX").toInt();
    }
    if (jsonObj.contains("ecViewY"))
    {
        _ec_param.ecViewY = jsonObj.value("ecViewY").toInt();
    }
    if (jsonObj.contains("ecViewWidth"))
    {
        _ec_param.ecViewWidth = jsonObj.value("ecViewWidth").toInt();
    }
    if (jsonObj.contains("ecViewHeight"))
    {
        _ec_param.ecViewHeight = jsonObj.value("ecViewHeight").toInt();
    }
    if (jsonObj.contains("ecSafeViewX"))
    {
        _ec_param.ecSafeViewX = jsonObj.value("ecSafeViewX").toInt();
    }
    if (jsonObj.contains("ecSafeViewY"))
    {
        _ec_param.ecSafeViewY = jsonObj.value("ecSafeViewY").toInt();
    }
    if (jsonObj.contains("ecSafeViewWidth"))
    {
        _ec_param.ecSafeViewWidth = jsonObj.value("ecSafeViewWidth").toInt();
    }
    if (jsonObj.contains("ecSafeViewHeight"))
    {
        _ec_param.ecSafeViewHeight = jsonObj.value("ecSafeViewHeight").toInt();
    }
    if (jsonObj.contains("carDpi"))
    {
        _ec_param.car_dpi = jsonObj.value("carDpi").toInt();
    }

    if (jsonObj.contains("channelCode"))
    {
        strncpy(channelCode, jsonObj.value("channelCode").toString().toUtf8().data(), sizeof(channelCode));
    }
    if (jsonObj.contains("channelName"))
    {
        strncpy(channelName, jsonObj.value("channelName").toString().toUtf8().data(), sizeof(channelName));
    }
    if (jsonObj.contains("otaSid"))
    {
        strncpy(otaSid, jsonObj.value("otaSid").toString().toUtf8().data(), sizeof(otaSid));
    }
    if (jsonObj.contains("otaVersion"))
    {
        strncpy(otaVersion, jsonObj.value("otaVersion").toString().toUtf8().data(), sizeof(otaVersion));
    }
    if (jsonObj.contains("appVersion"))
    {
        strncpy(appVersion, jsonObj.value("appVersion").toString().toUtf8().data(), sizeof(appVersion));
    }
    if (jsonObj.contains("mcuPath"))
    {
        strncpy(mcuPath, jsonObj.value("mcuPath").toString().toUtf8().data(), sizeof(mcuPath));
    }

    /* 虚拟仪表握手协议配置 */
    yps_bll_ecm_config_t _ecm_config;
    if (jsonObj.contains("projectType"))
    {
        _ecm_config.projectType = jsonObj.value("projectType").toInt();
    }
    if (jsonObj.contains("max_speed"))
    {
        _ecm_config.max_speed = jsonObj.value("max_speed").toInt();
    }
    if (jsonObj.contains("warning_speed"))
    {
        _ecm_config.warning_speed = jsonObj.value("warning_speed").toInt();
    }
    if (jsonObj.contains("max_rpm"))
    {
        _ecm_config.max_rpm = jsonObj.value("max_rpm").toInt();
    }
    if (jsonObj.contains("warning_rpm"))
    {
        _ecm_config.warning_rpm = jsonObj.value("warning_rpm").toInt();
    }
    if (jsonObj.contains("max_tempwater_level"))
    {
        _ecm_config.max_tempwater_level = jsonObj.value("max_tempwater_level").toInt();
    }
    if (jsonObj.contains("warning_tempwater_level"))
    {
        _ecm_config.warning_tempwater_level = jsonObj.value("warning_tempwater_level").toInt();
    }
    if (jsonObj.contains("tempwater_range"))
    {
        strncpy(_ecm_config.tempwater_range, jsonObj.value("tempwater_range").toString().toUtf8().data(), sizeof(_ecm_config.tempwater_range));
    }
    if (jsonObj.contains("warning_tempwater"))
    {
        _ecm_config.warning_tempwater = jsonObj.value("warning_tempwater").toInt();
    }
    if (jsonObj.contains("max_oillevel"))
    {
        _ecm_config.max_oillevel = jsonObj.value("max_oillevel").toInt();
    }
    if (jsonObj.contains("warning_oillevel"))
    {
        _ecm_config.warning_oillevel = jsonObj.value("warning_oillevel").toInt();
    }
    if (jsonObj.contains("screen_type"))
    {
        _ecm_config.screen_type = jsonObj.value("screen_type").toInt();
    }

    if (jsonObj.contains("supportVirtualInstrument"))
    {
        supportFunction_1 |= YPS_BLL_SUPPORT_FUNCTION_VIRTUAL_INSTRUMENT;
    }
//mark 后续记得移除这里，等临工做好移植
#ifdef IEAI 
    // int a;
    // ieai_mcu_set_notify(ieai_mcu_notify_handler);

 	// int res = ieai_mcu_open("/dev/ttyS4");
    // if(res < 0){
    //      printf("ieai_mcu_open failed \n");
    // }



#endif

    yps_app_init(&_ec_param, &_bt_config, channelCode, channelName, otaSid, otaVersion, 
    appVersion, supportFunction_1, mcuPath, &default_para, &_ecm_config, &tpms_option);
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX    
    //使用bll回调函数
#else
    //rom播放的开机动画,自己检查动画播放完成
    pthread_t tid_thread;
    pthread_create(&tid_thread, NULL, Qt_bll_proxy::data_update_thread, s_instance);
    pthread_detach(tid_thread);
#endif    
    return 0;
}
Qt_bll_proxy::Qt_bll_proxy()
{
    this->m_focus_object = nullptr;
    this->m_uinput_fd = 0;
    this->m_animation_status = 0;
    this->m_enable_key_event = 0;
    // 打开uinput设备
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    QVariantList qVarList;
    if (fd < 0)
    {
        qVarList << "无法打开uinput设备\n";
        log(qVarList);
        return;
    }

    qVarList << "打开uinput设备成功 " << fd << "\n";
    log(qVarList);
    qVarList.clear();


    int rtn;
    // 设置uinput设备的属性
    rtn = ioctl(fd, UI_SET_EVBIT, EV_KEY);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_ESC);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    rtn = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTSHIFT);
    if (rtn < 0)
    {
        qVarList << "UI_SET_EVBIT EV_KEY error " << rtn << "\n";
        log(qVarList);
        qVarList.clear();
    }
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "yps_Keyboard");
    uidev.id.bustype = BUS_USB; // BUS_VIRTUAL;
    uidev.id.vendor = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;
    if (write(fd, &uidev, sizeof(uidev)) == -1)
    {
        qVarList << "write errorNo:\n";
        log(qVarList);
        qVarList.clear();
    }
    else
    {
        // 创建uinput设备
        if (ioctl(fd, UI_DEV_CREATE, &uidev) == -1)
        {
            qVarList << "无法创建uinput设备 errorNo:" << errno << "\n";
            log(qVarList);
            qVarList.clear();
        }
        else
        {
            this->m_uinput_fd = fd;
            std::string line;
            std::ifstream input("/proc/bus/input/devices");
            bool found_keyboard = false;
            std::string handlers;

            while (getline(input, line))
            {
                if (line.find("Name=\"yps_Keyboard\"") != std::string::npos)
                {
                    found_keyboard = true;
                }
                if (found_keyboard && line.find("Handlers=") != std::string::npos)
                {
                    handlers = line.substr(line.find("event"));
                    handlers = handlers.erase(handlers.find_last_not_of(" \t\n\r\f\v") + 1);

                    std::string evdevTouchscreenParameters = "/dev/input/" + handlers;
                    setenv("QT_QPA_EVDEV_KEYBOARD_PARAMETERS", evdevTouchscreenParameters.c_str(), 1);
                    qVarList << "add " << QString::fromStdString(evdevTouchscreenParameters) << " to Qt Application.";
                    log(qVarList);
                    qVarList.clear();
                    break;
                }
            }
        }
    }
}

void Qt_bll_proxy::cppSlotA()
{

    std::cout << "CppObject::cppSlotA"
              << "\n"
              << std::endl;
}

void Qt_bll_proxy::cppSlotB(const QString &str, int value)
{
    std::cout << "CppObject::cppSlotB" << str.toStdString() << value << "\n"
              << std::endl;
    emit this->cppSignalB(str, value);
}

int Qt_bll_proxy::qt_dateset_get_int(int index)
{
    return yps_dateset_get_int(index);
}

int Qt_bll_proxy::qt_dateset_get_uint(int index)
{
    return yps_dateset_get_uint(index);
}

float Qt_bll_proxy::qt_dateset_get_float(int index)
{
    return yps_dateset_get_float(index);
}

QString Qt_bll_proxy::qt_dateset_get_string(int index)
{
    QString str;
    char tmp[512];
    char *rtn = yps_dateset_get_string(index, tmp, sizeof(tmp));
    if (rtn)
    {
        str.append(tmp);
    }




    return str;
}

int Qt_bll_proxy::qt_set_property(QString name, QString value)
{
    int res = property_set(name.toStdString().c_str(), value.toStdString().c_str());

    return res;

}
QString Qt_bll_proxy::qt_get_property(QString name, QString def_value)
{
    char value[PROPERTY_VALUE_MAX];
    int rtn = property_get(name.toStdString().c_str(), value, def_value.toStdString().c_str());
    if (rtn > 0)
    {
        return QString(value);
    }
    else
    {
        return def_value;
    }
}

QString Qt_bll_proxy::qt_dateset_get_object(int index)
{   
    
    int MAX_SCAN_DEVICE_SIZE = 8;
    yps_bt_device_info_t _device_list[MAX_SCAN_DEVICE_SIZE];
    yps_bt_call_log_t _call_log[YPS_BT_MAX_CALL_LOG];
    switch (index)
    {
    case DI_BT_MUSIC_INFO:
        SKYLOGD("get DI_BT_MUSIC_INFO");
        yps_bt_music_info_t info;
        if (yps_dateset_get_object(index, &info, sizeof(yps_bt_music_info_t)))
        {
            QJsonObject jsonObj;
            jsonObj.insert("play_state", (int)(info.play_state));
            jsonObj.insert("track_tick_total", (int)(info.track_tick_total));
            jsonObj.insert("track_tick_current", (int)(info.track_tick_current));
            jsonObj.insert("track_volume", (int)(info.track_volume));

            jsonObj.insert("track_title", QString::fromUtf8((char *)info.track_title));
            jsonObj.insert("track_artist", QString::fromUtf8((char *)info.track_artist));
            jsonObj.insert("track_album", QString::fromUtf8((char *)info.track_album));

            QJsonDocument jsonDoc(jsonObj);

            return jsonDoc.toJson();
        }

        break;
    case DI_BT_CALLER_CURRENT_LOG:
        yps_bt_call_log_t call_log;
        if (yps_dateset_get_object(index, &call_log, sizeof(yps_bt_call_log_t)))
        {

            QJsonObject jsonObj;

            jsonObj.insert("type", (int)(call_log.type));
            jsonObj.insert("dateTime", QString::fromUtf8(call_log.dateTime));
            jsonObj.insert("start", (int)(call_log.start));
            jsonObj.insert("curTime", (int)(call_log.curTime));
            jsonObj.insert("end", (int)(call_log.end));

            jsonObj.insert("phone.name", QString::fromUtf8((char *)call_log.phone.name));
            jsonObj.insert("phone.number", QString::fromUtf8((char *)call_log.phone.number));
            jsonObj.insert("phone.number_code", (qint64)(call_log.phone.number_code));

            QJsonDocument jsonDoc(jsonObj);

            return jsonDoc.toJson();
        }
        break;
    case DI_BT_BLUETOOTH_PHONE_BOOK_LIST:
    {
        SKYLOGD("get DI_BT_BLUETOOTH_PHONE_BOOK_LIST");

        // 先获取有多少个联系人
        int contact_len = yps_dateset_get_int(DI_BT_BLUETOOTH_PHONE_BOOK_LIST_LEN);
        //printf("contact_len = %d\n", contact_len);

        if(contact_len <= 0){
            return "{}";
        }

        yps_bt_phone_item_t phone_item[contact_len];

        yps_dateset_get_object(index, phone_item, sizeof(phone_item));
        QJsonArray jsonArray;

        for (int i = 0; i < contact_len; i++)
        {   
            QJsonObject jsonObj;
            jsonObj.insert("phone_item.name", QString::fromUtf8((char *)phone_item[i].name));
            jsonObj.insert("phone_item.number", QString::fromUtf8((char *)phone_item[i].number));
            jsonObj.insert("phone_item.number_code", (int)(phone_item[i].number_code));

            jsonArray.append(jsonObj);
        }

        QJsonDocument jsonDoc(jsonArray);

        return jsonDoc.toJson();
    
        break;
    }  
    case DI_BT_CALLER_CURRENT_RECENT_LIST:
    {
        int recent_len = yps_dateset_get_int(DI_BT_BLUETOOTH_CALL_RECENT_LIST_LEN);
        SKYLOGD("DI_BT_BLUETOOTH_CALL_RECENT_LIST_LEN = %d\n", recent_len);

        if(recent_len <= 0){
            SKYLOGD("recent_len == 0, return \n");
            return "{}";
        }
        yps_bt_call_log_t recentCallItemArray[recent_len];
        
        if (yps_dateset_get_object(index, recentCallItemArray, sizeof(recentCallItemArray)))
        {
            QJsonArray jsonArray;
            for (int i = 0; i < recent_len; i++)
            {
                QJsonObject jsonObj;







                jsonObj.insert("type", (int)(recentCallItemArray[i].type));
                jsonObj.insert("dateTime", QString::fromUtf8(recentCallItemArray[i].dateTime));
                jsonObj.insert("start", (int)(recentCallItemArray[i].start));
                jsonObj.insert("curTime", (int)(recentCallItemArray[i].curTime));
                jsonObj.insert("end", (int)(recentCallItemArray[i].end));
                jsonObj.insert("phone.name", QString::fromUtf8((char *)recentCallItemArray[i].phone.name));
                jsonObj.insert("phone.number", QString::fromUtf8((char *)recentCallItemArray[i].phone.number));
                jsonObj.insert("phone.number_code", (qint64)(recentCallItemArray[i].phone.number_code));

                jsonArray.append(jsonObj);
            }

            QJsonDocument jsonDoc(jsonArray);

            return jsonDoc.toJson();
        }
    }
        break;
    case DI_BT_HELMET1_DEVICE:
    case DI_BT_HELMET2_DEVICE:
        SKYLOGD("DI_BT_HELMET2_DEVICE \n");
        yps_bt_device_info_t device_info;
        if (yps_dateset_get_object(index, &device_info, sizeof(yps_bt_device_info_t)))
        {
            QJsonObject jsonObj;

            jsonObj.insert("index", (int)(device_info.index));
            jsonObj.insert("signal", (int)(device_info.signal));
            jsonObj.insert("battery", (int)(device_info.battery));
            jsonObj.insert("ibr", (int)(device_info.ibr));
            jsonObj.insert("mac", QString::fromUtf8((char *)device_info.mac));
            jsonObj.insert("name", QString::fromUtf8((char *)device_info.name));
            jsonObj.insert("status", (int)(device_info.status));

            QJsonDocument jsonDoc(jsonObj);

            return jsonDoc.toJson();
        }
        break;
         case DI_BT_BLUETOOTH_DEVICE_LIST:
        yps_bt_device_info_t bt_devicce_info;
        if (yps_dateset_get_object(index, &bt_devicce_info, sizeof(yps_bt_device_info_t)))
        {
            QJsonArray jsonArray;
            for (int i = 0; i < YPS_BT_MAX_NAME_SIZE; i++)
            {
                QJsonObject jsonObj;

                jsonObj.insert("index", (int)(bt_devicce_info.index));
                jsonObj.insert("signal", (int)(bt_devicce_info.signal));
                jsonObj.insert("battery", (int)(bt_devicce_info.battery));
                jsonObj.insert("ibr", (int)(bt_devicce_info.ibr));
                jsonObj.insert("mac", QString::fromUtf8((char *)bt_devicce_info.mac));
                jsonObj.insert("name", QString::fromUtf8((char *)bt_devicce_info.name));
                jsonObj.insert("status", (int)(bt_devicce_info.status));

                jsonArray.append(jsonObj);
            }
            QJsonDocument jsonDoc(jsonArray);
            return jsonDoc.toJson();
        }
        break;
    case DI_BT_HELMET_SCAN_LIST:
    case DI_BT_HELMET1_PLIST:
    case DI_BT_HELMET2_PLIST:
     SKYLOGD("DI_BT_HELMET_SCAN_LIST  DI_BT_HELMET1_PLIST DI_BT_HELMET2_PLIST\n");
        if (yps_dateset_get_object(index, _device_list, sizeof(_device_list)))
        {
            QJsonArray jsonArray;
            for (int i = 0; i < MAX_SCAN_DEVICE_SIZE; i++)
            {
                QJsonObject jsonObj;

                jsonObj.insert("index", (int)(_device_list[i].index));
                jsonObj.insert("signal", (int)(_device_list[i].signal));
                jsonObj.insert("battery", (int)(_device_list[i].battery));
                jsonObj.insert("ibr", (int)(_device_list[i].ibr));
                jsonObj.insert("mac", QString::fromUtf8((char *)_device_list[i].mac));
                jsonObj.insert("name", QString::fromUtf8((char *)_device_list[i].name));
                jsonObj.insert("status", (int)(_device_list[i].status));

                jsonArray.append(jsonObj);
            }
            QJsonDocument jsonDoc(jsonArray);

            return jsonDoc.toJson();
        }
        break;

    default:
        break;
    }

    {
        printf("yps_dateset_get_object %d failed\n", index);
    }
    return "";
}

QString Qt_bll_proxy::getCurrentThreadId()
{

    QString threadText;
    long tid = gettid();
    threadText = threadText.asprintf("(%ld)", tid);
    return threadText;
}

int Qt_bll_proxy::ec_exec_command(int type, int code, int value)
{
    return yps_bll_ec_exec_command((yps_command_ec_type_t)type, code, value);
}
int Qt_bll_proxy::mcu_exec_json_command(int type, QString value)
{
#ifdef BUILD_MCU_ADS
    if (type == PRJ_COMMAND_SET_CAR_CONFIG)
    {
        car_config_t t;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(value.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        t.car_type = jsonObj.value("car_type").toInt();
        t.abs_config = jsonObj.value("abs_config").toInt();
        t.engine_type = jsonObj.value("engine_type").toInt();
        return yps_bll_mcu_exec_command((yps_command_mcu_type_t)type, (uint32_t)0, (uintptr_t)&t);
    }
#else
    Q_UNUSED(type);
    Q_UNUSED(value);    
#endif
    return 0;
}


int Qt_bll_proxy::ieai_mcu_exec_command(int type, int code, int value)
{
    //   if(value == 1){
    //    return ieai_mcu_get_int((MCU_INDEX)type);
    // }else if(value == 0){
    //     printf("set type = %d, code = %d, value = %d \n",type, code, value);
    //    return ieai_mcu_set((MCU_COMMAND)type, code, value);
    // }else if (value == 2){
    // //    return ieai_mcu_get_uint( (MCU_INDEX)type );
    // }
}



int Qt_bll_proxy::mcu_exec_command(int type, int code, int value)
{

    LOGD("Qt_bll_proxy --> mcu_exec_command, type: %d, code: %d, value: %d", type, code, value);
    return yps_bll_mcu_exec_command((yps_command_mcu_type_t)type, (uint32_t)code, (uintptr_t)value);   

    return 0;
}

int Qt_bll_proxy::bt_exec_command(int index, int type, int code, QString value)
{
   
    uintptr_t para3 = 0;
    std::string str_value;
    if (value != nullptr && value.length() > 0)
    {
        str_value = value.toStdString();
        para3 = (uintptr_t)str_value.c_str();
    }
    return yps_bll_bt_exec_command((yps_bt_index_t)index, (yps_command_bt_type_t)type, (uint32_t)code, para3);
}

int Qt_bll_proxy::tpms_exec_command(int type, int code, QString value)
{

    Q_UNUSED(value);

    return yps_bll_tpms_exec_command((yps_command_tpms_t)type, (uint32_t)code, reinterpret_cast<uintptr_t>(nullptr));

}

void Qt_bll_proxy::on_yps_bll_bt_notify(int index, int type, int code, QString value)
{
    emit this->bt_notify(index, type, code, value);
}
void Qt_bll_proxy::on_yps_mcu_notify_handler(yps_notify_mcu_type_t type, unsigned int code, unsigned int value)
{
    emit this->mcu_notify(type, code, value);
}
static void emit_key_event(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

typedef  enum  {
    KEY_VALUE_NONE = 0x00,    // 无按键
    KEY_VALUE_RETURN = 0x01,  // 返回键  映射为 ESC
    KEY_VALUE_PREV = 0x02,    // 上一曲  映射为 up
    KEY_VALUE_NEXT = 0x04,    // 下一曲  映射为 down
    KEY_VALUE_PLAY = 0x08,    // 播放键  映射为 enter qt内为return
    KEY_VALUE_VOLUP = 0x10,   // 量加    映射为 right
    KEY_VALUE_VOLDOWN = 0x20, // 音量减  映射为 left
    KEY_VALUE_UNLOCK = 0x40,  // +亮
    //KEY_VALUE_LOCK = 0x80,  // 灭
 }rwa_key;


int tempcount = 0;
void Qt_bll_proxy::ieai_sample_key_proxy(  int data1)
{
 //   printf("data1 ieai_sample_key_proxy %d",data1);
    int key = 0;
   
 //   printf("aaa qbll-rwa_key :KEY_VALUE_RETURN ： %d\n",data1 &  0x3F);
    switch(data1 & 0xBF)
    {
        case KEY_VALUE_NONE:
            break;
        case KEY_VALUE_RETURN:
        {
       //     printf("qbll-rwa_key :KEY_VALUE_RETURN \n");
            key = KEY_ESC; //KEY_RETURN;
            break;
        }
        // 实现音乐在无效点时可播放这部分消息不加入注册到qt机制中
        case KEY_VALUE_PREV:
        {
        //     printf("qbll-rwa_key :KEY_VALUE_PREV \n");
        //  //   key =  KEY_RIGHT;
            break;
        }
        case KEY_VALUE_NEXT:
        {
        //      printf("qbll-rwa_key :KEY_VALUE_NEXT \n");
        //   //   key = KEY_LEFT;
            break;
        }
           
        case KEY_VALUE_PLAY:
        {
        //     printf("qbll-rwa_key :EY_VALUE_PLAY \n");
        // //    key = KEY_ENTER;  //qt内为return
            break;
        }
            

        case KEY_VALUE_VOLUP:
        {
        //     printf("qbll-rwa_key :KEY_VALUE_VOLUP \n");
        //    //  key = KEY_UP;
            break;
        }
           
        case KEY_VALUE_VOLDOWN:
        {
        //     printf("qbll-rwa_key :KEY_VALUE_VOLDOWN \n");
        // //    key = KEY_DOWN;
            break;
        }
          
    }

    // 模拟按下
    emit_key_event(m_uinput_fd, EV_KEY, key, 1);

    emit_key_event(m_uinput_fd, EV_SYN, SYN_REPORT, 0);

    emit_key_event(m_uinput_fd, EV_KEY, key, 0);

    emit_key_event(m_uinput_fd, EV_SYN, SYN_REPORT, 0);
   
}

void Qt_bll_proxy::on_page_key_handler(int code, int status)
{

    int key = 0;
    // 模拟按下键盘
    if (code == YPS_KEYCODE_UP)
    {
        key = KEY_UP;
    }
    else if (code == YPS_KEYCODE_DOWN)
    {
        key = KEY_DOWN;
    }
    else if (code == YPS_KEYCODE_LEFT)
    {
        key = KEY_LEFT;
    }
    else if (code == YPS_KEYCODE_RIGHT)
    {
        key = KEY_RIGHT;
    }
    else if (code == YPS_KEYCODE_ENTER)
    {
        key = KEY_ENTER;
    }
    else if (code == YPS_KEYCODE_RETURN)
    {
        key = KEY_ESC;
    }
    if (m_uinput_fd > 0)
    {
        QVariantList qVarList;
        qVarList << "on_page_key_handler   :" << status << " " << key << " enable " << this->m_enable_key_event;
        log(qVarList);
        if(this->m_enable_key_event == 0)
        {
            return ;
        }
        if (status == YPS_KEYSTATUS_MIDDLE_CLICK || status == YPS_KEYSTATUS_LONG_CLICK)
        {
            emit_key_event(m_uinput_fd, EV_KEY, KEY_LEFTSHIFT, 1);
        }
        // 模拟按下
        emit_key_event(m_uinput_fd, EV_KEY, key, 1);

        emit_key_event(m_uinput_fd, EV_SYN, SYN_REPORT, 0);

        if (status == YPS_KEYSTATUS_MIDDLE_CLICK || status == YPS_KEYSTATUS_LONG_CLICK)
        {
            emit_key_event(m_uinput_fd, EV_KEY, KEY_LEFTSHIFT, 0);
        }
        emit_key_event(m_uinput_fd, EV_KEY, key, 0);

        emit_key_event(m_uinput_fd, EV_SYN, SYN_REPORT, 0);
    }
}

void Qt_bll_proxy::on_yps_animation_status(int status)
{
    this->m_animation_status = status;
    emit this->animation_status(status);
}

void Qt_bll_proxy::on_yps_bll_notify_handler(yps_notify_bll_type_t type, unsigned int code, unsigned int value)
{
    emit this->bll_notify(type, code, value);
}

void Qt_bll_proxy::on_yps_ec_notify_handel(int type, int code, int value)
{
    emit this->ec_notify(type, code, value);
}
void Qt_bll_proxy::on_ieai_mcu_notify_handler(int code, int data1, int data2)
 {
//  //   SkyTRACE("on_ieai_mcu_notify_handler code");
//  // printf("aaa qbll-rwa_key :KEY_VALUE_RETURN ： %d\n",data1 &  0x3F);
//     if(MCU_NOTIFY::MN_KEY == code)
//     {
//         ieai_sample_key_proxy(data1);   
//         data1 &= 0x3F;
//         //return;
//       //  on_page_key_handler(data1, data2);
//     }
//     emit this->ieai_mcu_notify(code, data1, data2);
}
int Qt_bll_proxy::getAnimationStatus()
{
    return this->m_animation_status;
}

QString Qt_bll_proxy::bll_get_version()
{
    return QString::fromUtf8(yps_bll_get_version());
}

QString Qt_bll_proxy::qt_system(QString cmd, QString paras)
{
    QStringList paraList;
    // 把paras参数按空格分割，如果是双引号""引起来的空格，不拆分，存入paraList

    int start = 0;
    int end = 0;
    bool isQuote = false;
    for (int i = 0; i < paras.length(); i++)
    {
        if (i == 0 && paras[i] == '"')
        {
            isQuote = true;
        }
        else if (paras[i] == '"' && i > 0 && paras[i - 1] != '\\')
        {
            isQuote = !isQuote;
        }
        else if (paras[i] == ' ' && !isQuote)
        {
            end = i;
            paraList.append(paras.mid(start, end - start));
            start = end + 1;
        }
    }
    QVariantList qVarList;
    // 打印出所有的参数
    for (int i = 0; i < paraList.size(); i++)
    {
        qVarList << "paraList[" << i << "]:" << paraList[i] << "\n";
    }
    log(qVarList);

    QProcess process;
    process.start(cmd, paraList);

    if (process.waitForStarted() && process.waitForFinished())
    {
        QByteArray result = process.readAll();
        QString output(result);
        return output;
    }
    else
    {
        return "Command failed to execute.";
    }
}
void Qt_bll_proxy::send_touch_event(int x, int y, int type, int slot)
{
    yps_bll_ec_touch_event_param_t param;
    param.pointX = x;
    param.pointY = y;

    param.slot = slot;
    yps_bll_ec_send_touchEvent(&param, (yps_bll_ec_touch_type_param_t)type);
}
void Qt_bll_proxy::log(QVariantList args)
{
    QString result;
    for (auto arg : args)
    {
        result += arg.toString() ;
        result += " ";
    }
    std::string slog = result.toStdString();
    const char *log = slog.c_str();
    LOGAPPD("SKY QML %s", log);    
}

void Qt_bll_proxy::ec_start()
{
    yps_bll_ec_start();
}

int Qt_bll_proxy::getEnableKeyEvent()
{
    return this->m_enable_key_event;
}
void Qt_bll_proxy::setEnableKeyEvent(int enable)
{
    this->m_enable_key_event = enable;
    emit this->enable_key_event_changed(enable);
}

