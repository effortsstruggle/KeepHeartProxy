#ifndef __YPS_APP_CONFIG_H__
#define __YPS_APP_CONFIG_H__
#include "yps_type.h"
#include "yps_ec.h"
#include "yps_bt.h"
#include "yps_tpms.h"

#ifdef __cplusplus
extern "C"
{
#endif
void yps_app_init(yps_bll_ec_init_param_t *p_ec_param, yps_bt_config_t *p_bt_config,
                  char *channelCode, char *channelName, char *otaSid, char *otaVersion,
                  char *appVersion, uint64_t function_1, char *mcuPath,
                  yps_bll_default_para_t *default_para, yps_bll_ecm_config_t *ecm_config, yps_tpms_option_t *tpms_option);



typedef enum
{
    YPS_ANIMATION_NONE,
    YPS_ANIMATION_START_UP_PREPARE,
    YPS_ANIMATION_START_UP_AFTER,
    YPS_ANIMATION_SHUTDOWN_PREPARE,
    YPS_ANIMATION_SHUWDOWN_AFTER,
    YPS_ANIMATE_PLAY_DONE
    

    
} yps_animate_status_t;


void yps_app_deinit();
#ifdef __cplusplus
}
#endif

#endif // __YPS_APP_CONFIG_H__
