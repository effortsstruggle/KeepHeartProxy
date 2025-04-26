#include <stdio.h>
#include "yps_app_config.h"
#include "yps_dataset.h"
#include "bllLog.h"
#include "yps_app.h"
#include "yps_bll.h"
#include "yps_bt.h"
#include "yps_mcu.h"
#include "yps_ec.h"
#include "yps_inner.h"
#include "bll_ini.h"
#include "yps_keypad.h"
#include <string.h>

extern void yps_key_event(uint32_t code, uint8_t status, uint32_t interval);
extern void yps_bll_bt_notify(yps_bt_index_t index, yps_notify_bt_type_t type, uint32_t code, uintptr_t value);
extern void yps_mcu_notify_handler(yps_notify_mcu_type_t type, uint32_t code, uintptr_t value);
extern void yps_animation_status(int status);
extern void yps_bll_notify_handler(yps_notify_bll_type_t type, unsigned int code, unsigned int value);
extern void yps_ec_notify_handel(yps_notify_ec_type_t type, unsigned int code, unsigned int value);

static yps_bll_init_param_t _bll_init_param;
yps_bll_inner_init_param_t _bll_inner_init_param;

static yps_bt_cb_f _yps_bt_conf;

static yps_mcu_cb_f _yps_mcu_conf;

static yps_mcu_config_t _mcu_config;

static yps_ec_cb_f _yps_ec_conf;

static void yps_mcu_direct_transfer_handler(const uint8_t *buffer, uint32_t len)
{
    (void)buffer;
    (void)(len);
}

static int yps_bll_animation_play_status_handler(int status, int interrupt)
{
    (void)status;

    
    (void)interrupt;
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX    
    yps_animation_status(YPS_ANIMATE_PLAY_DONE);
#endif    
    return 0;
}

// bll准备开始播放开机动画
static int yps_animation_start_up_prepare_handler()
{
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX
    yps_animation_status(YPS_ANIMATION_START_UP_PREPARE);
#endif    
    return 0;
}

// 已经播放完成开机动画，开机动画状态下UI需要的操作
static int yps_animation_start_up_after_handler(bool interrupt)
{
    (void)interrupt;
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX    
    yps_animation_status(YPS_ANIMATION_START_UP_AFTER);
#endif    
    return 0;
}

// bll准备开始播放关机动画
static int yps_animation_shutdown_prepare_handler()
{
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX    
    yps_animation_status(YPS_ANIMATION_SHUTDOWN_PREPARE);
#endif    
    return 0;
}

// bll已经播放完成关机动画，关机动画状态下UI需要的操作
static int yps_animation_shuwdown_after_handler()
{
#if HAVE_PLAY_ANIMATION_IN_SERIALMUX    
    yps_animation_status(YPS_ANIMATION_SHUWDOWN_AFTER);
#endif    
    return 0;
}

#if 0
int32_t keypad_code_mapping(int32_t code)
{
    int32_t ret;

    switch (code)
    {
        case 104:
            ret = YPS_KEYCODE_RETURN; 
            break;
        case 102:
            ret = YPS_KEYCODE_UP;
            break;
        case 101:
            ret = YPS_KEYCODE_DOWN;
            break;
        case 103:
            ret = YPS_KEYCODE_ENTER;
            break;
        default:
            ret = -1
            break;
    }

    return ret;
}
#endif

void yps_app_init(yps_bll_ec_init_param_t *p_ec_param, yps_bt_config_t *p_bt_config,
                  char *channelCode, char *channelName, char *otaSid, char *otaVersion,
                  char *appVersion, uint64_t function_1, char *mcuPath,
                  yps_bll_default_para_t *default_para, yps_bll_ecm_config_t *ecm_config, yps_tpms_option_t *tpms_option)
{
   
 //   printf("bll verson: %s \n",yps_bll_get_version());
    _yps_bt_conf.yps_bt_notify = yps_bll_bt_notify; // yps_bll_bt_notify_handler;
    _yps_mcu_conf.yps_mcu_direct_transfer = yps_mcu_direct_transfer_handler;
    _yps_mcu_conf.yps_mcu_notify = yps_mcu_notify_handler; // yps_mcu_notify_handler;
    _yps_mcu_conf.bll_animation_start_up_prepare = yps_animation_start_up_prepare_handler;
    _yps_mcu_conf.bll_animation_start_up_after = yps_animation_start_up_after_handler;
    _yps_mcu_conf.bll_animation_shutdown_prepare = yps_animation_shutdown_prepare_handler;
    _yps_mcu_conf.bll_animation_shutdown_after = yps_animation_shuwdown_after_handler;

    _yps_ec_conf.yps_ec_notify = yps_ec_notify_handel;

    // 配置渠道号、ota版本信息
    // #define YPS_UI_OTAUPDATE_SID "SID_QT_T507"
    // #define YPS_UI_OTA_VERSION "1.0.0"
    strncpy(_bll_inner_init_param.channelCode, channelCode,sizeof(_bll_inner_init_param.channelCode)); //"66661504"
    strncpy(_bll_inner_init_param.channelName, channelName,sizeof(_bll_inner_init_param.channelName)); //"CPDM05"
    strncpy(_bll_inner_init_param.otaSid, otaSid,sizeof(_bll_inner_init_param.otaSid));
    strncpy(_bll_inner_init_param.otaVersion, otaVersion,sizeof(_bll_inner_init_param.otaVersion));
    strncpy(_bll_inner_init_param.appVersion, appVersion,sizeof(_bll_inner_init_param.appVersion));
    
    // _bll_inner_init_param.default_para.language = default_language;
    memcpy(&_bll_inner_init_param.default_para, default_para, sizeof(yps_bll_default_para_t));
    memcpy(&_bll_inner_init_param.ecm_config, ecm_config, sizeof(yps_bll_ecm_config_t));
    _bll_inner_init_param.supportFunction_1 = function_1;

    yps_bll_key_config_t key_config;
    key_config.long_click_interval = 10000;
    key_config.middle_click_interval = 1500;
    key_config.continuous_trigger_interval = 0;
    key_config.keep_trigger_middle_click = 0;
    memcpy(&_bll_inner_init_param.key_config, &key_config, sizeof(key_config));
    
    yps_bll_inner_init_param_t *pAddr = &_bll_inner_init_param;
    memset(&_bll_init_param, 0, sizeof(yps_bll_init_param_t));
    memcpy(&_bll_init_param, &pAddr, sizeof(pAddr));

    yps_bll_cb_f yps_bll_cb_config = {
        .yps_bll_notify = yps_bll_notify_handler,
        // .yps_bll_animation_play_status = yps_bll_animation_play_status_handler,
        .yps_bll_key_event = yps_key_event,
    };

    memset(&_mcu_config, 0, sizeof(_mcu_config));
    strncpy(_mcu_config.path, mcuPath,sizeof(_mcu_config.path));

    yps_bll_init(&_bll_init_param, &yps_bll_cb_config);

    yps_bll_mcu_initialize(&_yps_mcu_conf);  
    yps_bll_mcu_start(&_mcu_config);


    yps_bll_bt_initialize(&_yps_bt_conf);


    yps_bll_ec_initialize(p_ec_param, &_yps_ec_conf);

    // yps_theme_manager_inti();
    // yps_view_manager_init();

 //   printf("before yps_bll_bt_start \n  path: %s\n,gpio_index: %d \n,model: %d\n,option: %d \n ", 
 //   p_bt_config->path,p_bt_config->gpio_index,p_bt_config->model,p_bt_config->option);
   yps_bll_bt_start(p_bt_config);

  
   // yps_bll_tpms_init(tpms_option);

    printf("yps_app_init success \n");
#if 0
    yps_keypad_param_t key_para;
    key_para.reserve = 0;

    yps_keypad_cb_f keypad_cb;
    keypad_cb.yps_keypad_code_mapping_f = keypad_code_mapping;

    yps_bll_keypad_initialize(&key_para, &keypad_cb);
    yps_bll_keypad_start();
#endif
}

void yps_app_deinit()
{
    return;
}
