#include <unistd.h>
#include <stdio.h>
#include "McuParser.h"
#include "McuDef.h"
#include "McuData.h"
#include <string.h>
#include <android/log.h>
#include "LoopBuffer.h"
#include <thread>
#include <mutex>
#include <functional>
#include <sys/time.h>
#include "cJSON.h"


#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_MCU", __VA_ARGS__);

pthread_mutex_t* g_pThread = NULL;


static std::mutex	m_errMtx;
static std::mutex	m_keyMtx;

static tag_mcu_to_soc package = {0};
static tag_mcu_to_soc package2 = {0};

static tag_mcu_to_soc *newPack = &package;      //作为交换内存使用，避免数据的一次拷贝
static tag_mcu_to_soc *curPack = &package2;

static packetMcu2SocD002 packet_btkey = {0};
static std::map<int ,packetMcu2SocD002> packet_btkey_scan_map; //mac 地址作为 crc
static std::map<int ,packetMcu2SocD002> packet_btkey_bind_map; //mac 地址作为 crc
static s_packet_unlock    packet_unlock {{0xCA53,sizeof(s_packet_unlock)-4,0xC001}};
static s_packet_gear      packet_gear {{0xCA53,sizeof(s_packet_gear)-4,0xC002}};

static s_packet_trip2     packet_trip2 {{0xCA53,sizeof(s_packet_trip2)-4,0xC003}};

static s_packet_climate   packet_climate {{0xCA53,sizeof(s_packet_climate)-4,0xC004}};

static s_packet_close     packet_close {{0xCA53,sizeof(s_packet_close)-4,0xC005}};

static s_packet_instument packet_instument {{0xCA53,sizeof(s_packet_instument)-4,0xC006}};

static s_packet_ins_update packet_ins_update{{0xCA53,sizeof(s_packet_ins_update)-4,0xC007}};
static s_packet_vcu_setting packet_vcu_setting {{0xCA53,sizeof(s_packet_vcu_setting)-4,0xC008}};
static s_packet_bt_key_cmd  packet_btkey_cmd{{0xCA53,sizeof(s_packet_bt_key_cmd)-4,0xC009}};
static uint16_t mxc_crc16s_calc(const uint8_t *q, uint32_t len);

class skyAuoLock{
public:
    skyAuoLock(pthread_mutex_t * mt):mtx(mt){
          pthread_mutex_lock(mtx);
    };
    ~skyAuoLock(){
        pthread_mutex_unlock(mtx);
    }
private :
    pthread_mutex_t * mtx;
};
McuParser::McuParser()
{
    m_notifier = NULL;
    pthread_mutex_init(&m_mutex, NULL);
    pthread_mutex_init(&m_mutex2, NULL);

    g_pThread = &m_mutex;

    m_running = true;
    pthread_t id;
    pthread_create(&id, NULL, ThreadProc, this);

    static thread t(std::bind(&McuParser::ThreadErrCodeCheck,this));
    t.detach();

    static thread tk(std::bind(&McuParser::ThreadKeyHold,this));
    tk.detach();
}

McuParser::~McuParser()
{
    m_running = false;
    pthread_mutex_destroy(&m_mutex);
    pthread_mutex_destroy(&m_mutex2);
}

int McuParser::ParseData( LoopBuffer& buf)
{
  
    int len  = buf.GetSize();

    // 是否够一个包头的大小
    if( len < sizeof(tagPacketHead)){
        LOGD("len < sizeof(tagPacketHead): %d",len);  
         return -1;
    }         
    

    tagPacketHead   taghead ;
    buf.GetData2((unsigned char*)&taghead,sizeof(tagPacketHead));

    // 检查标记 ,如果不对 扔掉 一个字节
    if(taghead.flag != 0xCA53){
        buf.Pop(1);
        return 1;
    }
    //是否读取到一个包的数据 ，包里边记的长度不包含flag 和 length，所以加上4 
    if(len < taghead.length + 4)
    {
        LOGD(" (len < rawdata->length + 4)");  
        return -1;
    }
    

   // 检查是不是升级包
    if(taghead.command == 0xB001 )
    {
        updataePacket upapck;
        buf.GetData2((unsigned char*)&upapck,sizeof(updataePacket));
        if(mxc_crc16s_calc((unsigned char*)&upapck, sizeof(updataePacket)-2) == upapck.crc){
            LOGD("ota package"); 
            AddToNotify(MN_OTA, 0,0);
        }
        buf.Pop(len);
        return -1;
    } 
    else if(taghead.command == 0xD002)
    {
        
        buf.GetData2((unsigned char*)&packet_btkey,sizeof(packetMcu2SocD002));
       // ProcessBtKeyMsg();
       // pthread_mutex_lock(&m_mutex);
        skyAuoLock ll(&m_mutex);
        unsigned char * data = (unsigned char *)&packet_btkey;
        // for(int i = 0;i < sizeof(packetMcu2SocD002);i++)
        // {
        //     LOGD("data[%d]: %#X",i,data[i]);  
        // }
        uint16_t crc_res = mxc_crc16s_calc( (unsigned char *)&packet_btkey, sizeof(packet_btkey)-2);
        if(crc_res != packet_btkey.crc){
            LOGD(" packet_btkey  crc error cur :%d,frompackage: %d ,packlen %d...",crc_res,packet_btkey.crc,sizeof(packet_btkey));  
            buf.Pop(len);
       //     pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        buf.Pop(sizeof(packetMcu2SocD002));

        ProcessBtKeyMsg();
    //    pthread_mutex_unlock(&m_mutex);
        return -1;
    }

    //普通数据包
    buf.GetData2((unsigned char*)newPack,sizeof(tag_mcu_to_soc));
    
    unsigned char * data = (unsigned char *)newPack;
    // for(int i = 0;i < len;i++)
    // {
    //     LOGD("data[%d]: %#X",i,data[i]);  
    // }

    //crc 校验
    uint16_t crc_res = mxc_crc16s_calc(data, MCU2SOCCRC);
    if(crc_res != newPack->crc)  
    {
        LOGD("unknow crc error cur :%d,frompackage: %d ,skip...",crc_res,newPack->crc);  
        buf.Pop(len);
        return -1;
    }  
    
   
    pthread_mutex_lock(&m_mutex);

   
    if(newPack->batteyInfo.battery_charge_state != curPack->batteyInfo.battery_charge_state)
    {
        LOGD("battery_charge_state %#X",newPack->batteyInfo.battery_charge_state);
        AddToNotify(MN_CHARGE, newPack->batteyInfo.battery_charge_state);
    }
    

    if(newPack->key_state != curPack->key_state)
    {
        KeyChange(newPack->key_state);
       
     //   LOGD("key_state %#X",newPack->key_state)
    }

    if(newPack->error_code){

        AddErrCode(newPack->error_code);
      
        // LOGD("newPack->error_code %d:",newPack->error_code )  
    }

  //  if(newPack->request_poweroff != curPack->request_poweroff)
    {
        AddToNotify(MN_REQUEST_OFF, newPack->request_poweroff);
      //  LOGD("request_poweroff %#X",newPack->request_poweroff)
    }
  //  LOGD(" inst_bright %#X",newPack->inst_bright)
    // if(curPack->gearInfo.gear!=newPack->gearInfo.gear  ){
    //     AddToNotify(MN_REQUEST_OFF, newPack->request_poweroff);
    // }

   
    swap(newPack,curPack);

    pthread_mutex_unlock(&m_mutex);
   

    buf.Pop(sizeof(tag_mcu_to_soc));
    return  -1;
        
}
// update  ProcessBtKeyMsg
void McuParser::ProcessBtKeyMsg()
{
    

    LOGD("ProcessBtKeyMsg   ");
    int uid = mxc_crc16s_calc((uint8_t*)packet_btkey.vMacAddr,12);
    if(packet_btkey.vMsgType == 0){
        auto iter = packet_btkey_scan_map.find(uid); // 扫描列表
        if(iter != packet_btkey_scan_map.end())
        {
            LOGD("packet_btkey_scan_map  重复id  ");
            return;
            
        }
        LOGD("packet_btkey_scan_map : %s ",packet_btkey.vMacAddr);
        packet_btkey_scan_map.emplace(uid,packet_btkey);
    }
    else{
        auto iter = packet_btkey_bind_map.find(uid); // 绑定列表
        if(iter != packet_btkey_bind_map.end())
        {
            LOGD("packet_btkey_bind_map  重复id  ");
            return;
        }
        LOGD("packet_btkey_bind_map : %s ",packet_btkey.vMacAddr);
        packet_btkey_bind_map.emplace(uid,packet_btkey);
   }

    cJSON* root = cJSON_CreateObject();
   
    cJSON_AddNumberToObject(root,"vMsgType",packet_btkey.vMsgType);  // 消息类型，0：扫描设备列表，1：绑定设备列表
  
    cJSON_AddNumberToObject(root,"uid",uid);  //  MAC地址 生成的uid

    cJSON_AddStringToObject(root,"vName",(char*)packet_btkey.vName);  //蓝牙名称
  
    
    char* ptr = cJSON_PrintUnformatted(root);
    //packet_btkey 
    AddToNotifyStr( MN_BTKEY_NOTIFY,0, ptr);
    cJSON_free(ptr);
    cJSON_Delete(root);
}
void McuParser::AddToNotify(MCU_NOTIFY code, int data1, long data2)
{
    NotifyItem* item = new NotifyItem;
    item->type = 1;
    item->code = code;
    item->data1 = data1;
    item->data2 = data2;

    pthread_mutex_lock(&m_mutex2);
    m_notifyItems.push_back(item);
    pthread_mutex_unlock(&m_mutex2);
}
 void McuParser::AddToNotifyStr(MCU_NOTIFY code, int data1, char* data2)
 {
    NotifyItem* item = new NotifyItem;
    item->type = 2;
    item->code = code;
    item->data1 = data1;
    item->str = data2;

    pthread_mutex_lock(&m_mutex2);
    m_notifyItems.push_back(item);
    pthread_mutex_unlock(&m_mutex2);

 }
// tag  McuParser GetInt 获取数据
int McuParser::GetInt(MCU_INDEX index)
{
    int ret = -1;

    pthread_mutex_lock(&m_mutex);

    switch (index)
    {
    case MI_UNLOCK:
        ret =curPack->unlock;
        break;

    case MI_RNDS_GEAR:
       // LOGD("MI_GEAR %d",curPack->gearInfo.gear);
        ret = curPack->gearInfo.RNDSgear;  
        break;   
    case MI_P_GEAR:
        ret = curPack->gearInfo.Pgear;  
    case MI_GEAR_ERR_P:
        ret = curPack->gearInfo.PFault;
        break;

    case MI_GEAR_ERR_RNDS:
        ret = curPack->gearInfo.DNRFault;
        break;         
 
    case MI_EQ:
        if(curPack->batteyInfo.battery_soc > 100){
            ret = 100;   break;
        }
            
        if(curPack->batteyInfo.battery_soc <0){
            ret = 0; break;

        }
            
        ret = curPack->batteyInfo.battery_soc;     
        break;     

    case MI_HEAT:
        ret = curPack->batteyInfo.battery_heat_state;
        break;       

    case MI_CHARGE_STATE:
        ret = curPack->batteyInfo.battery_charge_state;
        break;
    case MI_VOLTAGE:
        ret = curPack->batteyInfo.battery_voltage;
        break;

    case MI_CURRENT:
        ret = curPack->batteyInfo.battery_current;
        break;

    case MI_CHARGEDTIME:
        ret = curPack->batteyInfo.charge_time;
        break;

    case MI_TOCHARGINGTIME:
        ret = curPack->batteyInfo.charge_time_left;
        break;
    case    MI_IS_AC:           //是否是交流充电 0无 1 有
        ret = curPack->alternatingCurrent;break;
    case MI_MATITENANCE:     // 保养提示 0 无 1 有
        ret = curPack->maintenance;break;
    case MI_TBox4G:          // 有无Tbox4G信号 0无 1 有
        ret = curPack->tbox4G;break;
    case MI_CUR_TIME:        // 当前时间 1970年以来的秒数
        ret = curPack->realtime;break;
    case MI_SPEED:
        ret = curPack->speed;
        break;                

    case MI_SPEEDTRIP1:
        ret = curPack->trip1.trip1_avg_speed;
        break;               
    
    case MI_CONSUMETRIP1:
        ret = curPack->trip1.trip1_elec_consume;
        break;   

    case MI_TIMETRIP1:
        ret = curPack->trip1.trip1_time_info;    
        break;

    case MI_TRIP1:
        ret = curPack->trip1.trip1_info;
        break;   

    case MI_SPEEDTRIP2:
        ret = curPack->trip2.trip2_avg_speed;
        break;               
    
    case MI_CONSUMETRIP2:
        ret = curPack->trip2.trip2_elec_consume;
        break;   

    case MI_TIMETRIP2:
        ret = curPack->trip2.trip2_time_info;    
        break;

    case MI_TRIP2:
        ret = curPack->trip2.trip2_info;
        break;  

    case MI_ODO:
        ret = curPack->odo_info.odo_info;
        break;               
    
    case MI_RANGE:
        ret = curPack->odo_info.range_left;      
        break;     

    case MI_TOTALTIME:
        ret = curPack->odo_info.odo_time_info;
        break;              
    case MI_AC_TYPE:
        ret = curPack->ac_type;
        break;
    case MI_BRIGHTNESS_LEVEL:
        ret =  curPack->inst_bright;

        break;
    case MI_UPDATE_ERR_CODE:{
        std::lock_guard<std::mutex> lock(m_errMtx);
        for(auto iter  =  errCodeMap.begin();iter!= errCodeMap.end();iter++)
        {
            LOGD("MI_UPDATE_ERR_CODE, %d )",iter->second->errCode);
            AddToNotify(MN_ERR_CODE, iter->second->errCode,1);  
        }
        }
        break;
    case MI_MCUVER:
        ret =  curPack->mcuVersion;
       // LOGD("mcuVersion %d",curPack->mcuVersion);
        break;
    default:
         break;
    }

    pthread_mutex_unlock(&m_mutex);

    return ret;    
}

int McuParser::SendCmd(MCU_COMMAND cmd, int data1, long data2)
{
    int ret = 0;
    LOGD("McuParser::SendCmd cmd:%d, data1:%d, data2:%d \n",cmd,data1,data2);
    unsigned short size = 0;
    unsigned char*ptr = nullptr;
    int flag = 0;
    switch (cmd)
    {
    
    case MC_INS_UPDATE:
        size = sizeof(packet_ins_update);
        packet_ins_update.cmd = data1;
        packet_ins_update.crc = mxc_crc16s_calc((unsigned char*)&packet_ins_update, sizeof(packet_ins_update)-2);

        ptr = (unsigned char*)&packet_ins_update;
        LOGD("仪表升级 MC_INS_UPDATE %d ",data1);
        break;
    case MC_POWEROFF:
        size = sizeof(s_packet_close);
        packet_close.cmd = data1;
        packet_close.crc = mxc_crc16s_calc((unsigned char*)&packet_close, sizeof(packet_close)-2);

        ptr = (unsigned char*)&packet_close;
        
        break;
    case MC_UNLOCK:
    {
        size = sizeof(s_packet_unlock);
        packet_unlock.lock_state = 0;
        packet_unlock.crc = mxc_crc16s_calc((unsigned char*)&packet_unlock, sizeof(packet_unlock)-2);

        ptr = (unsigned char*)&packet_unlock;

        break;
    }

    case MC_SET_GEAR:
    {
        size = sizeof(s_packet_gear);
        packet_gear.gear_state.rnds_state = data1;
        packet_gear.gear_state.p_state = data2;
        LOGD("MC_SET_GEAR %d,%d" ,data1,data2);
        packet_gear.crc = mxc_crc16s_calc((unsigned char*)&packet_gear, size-2);

        ptr = (unsigned char*)&packet_gear;
        break;        
    }

    case MC_CLEAR_TRIP:
    {
        size = sizeof(packet_trip2);
       
        packet_trip2.crc = mxc_crc16s_calc((unsigned char*)&packet_trip2, size-2);

        ptr = (unsigned char*)&packet_trip2;
  
        break;        
    }    
     case MC_AC_MODE:
    {
        size = sizeof(packet_climate);
        packet_climate.ac_mode = data1;

        packet_climate.crc = mxc_crc16s_calc((unsigned char*)&packet_climate, size-2);
        ptr = (unsigned char*)&packet_climate;
       
  
        break;        
    }  
     case MC_AC_TEMP:
    {
        size = sizeof(packet_climate);
        packet_climate.ac_temp = data1;

        packet_climate.crc = mxc_crc16s_calc((unsigned char*)&packet_climate, size-2);
       
        ptr = (unsigned char*)&packet_climate;
        break;        
    }    
     case MC_AC_WIND_SPEED:
    {
        size = sizeof(packet_climate);
        packet_climate.wind_speed = data1;

        packet_climate.crc = mxc_crc16s_calc((unsigned char*)&packet_climate, size-2);
       
        ptr = (unsigned char*)&packet_climate;
        
        break;        
    }    
     case MC_AC_WIND_DIR:
    {
        size = sizeof(packet_climate);
        packet_climate.wind_direction = data1;
        packet_climate.crc = mxc_crc16s_calc((unsigned char*)&packet_climate, size-2);
        ptr = (unsigned char*)&packet_climate;
        
        break;        
    }    
    case MC_AC_SWITCH  :   // 交流电插座开关
         flag =1  ;
         packet_vcu_setting.vACSocketSwitch = data1;
         LOGD("交流电插座开关 %d",packet_vcu_setting.vACSocketSwitch);
    case MC_FORWORD_SPEED      : // 前向速度
        if(!flag) {
             packet_vcu_setting.vForwardMaximumSpeed =data1;
             LOGD("前向速度 %d",packet_vcu_setting.vForwardMaximumSpeed);
             flag = 1;
        }
    case MC_BACK_SPEED         : // 后退速度
     if(!flag) {
             packet_vcu_setting.vBackwardMaximumSpeed =data1;
             LOGD("后退速度 %d",packet_vcu_setting.vBackwardMaximumSpeed);
             flag = 1;
        } ;

    case MC_AUTO_BRAKING_LEVEL : // 再生制动等级
     if(!flag) {
             packet_vcu_setting.vRegenLevel = data1;
             LOGD("再生制动等级 %d",packet_vcu_setting.vRegenLevel);
             flag = 1;
        }  
    case MC_GREEP  : // 爬行模式
        if(!flag) {
             packet_vcu_setting.vCreepOrHillHold = data1;
             LOGD("爬行模式 %d",packet_vcu_setting.vCreepOrHillHold);
             flag = 1;
        }  
    case MC_EPB_AUTOHOLD      : //  EPB自动保持
        if(!flag) {
             packet_vcu_setting.vEPBAutoHold = data1;
             LOGD("EPB自动保持 %d",packet_vcu_setting.vEPBAutoHold);
             flag = 1;
        }  
    case MC_EPB_REPAIR        : //  EPB维修放行命令
         if(!flag) {
             packet_vcu_setting.vEPBRepair = data1;
             LOGD("EPB维修放行命令 %d",packet_vcu_setting.vEPBRepair);
             flag = 1;
        }  
    case MC_AUTO_POWER_OFF  : //  自动关机时长
         if(!flag) {
             packet_vcu_setting.vAutoPoweroffTime = data1;
             LOGD("自动关机时长 %d",packet_vcu_setting.vAutoPoweroffTime);
             flag = 1;
        }  
        size = sizeof(packet_vcu_setting);
       
        packet_vcu_setting.crc = mxc_crc16s_calc((unsigned char*)&packet_vcu_setting, size-2);
        
        ptr = (unsigned char*)&packet_vcu_setting;
        break ;    

    case MC_INS_UNITS_MILE:      // 里程
        flag = 1;
        packet_instument.units_mileage =data1;
    case MC_INS_UNITS_TEMP:      // 温度
        if(!flag) {
             packet_instument.units_temp =data1;
             LOGD("MC_INS_UNITS_TEMP %d",packet_instument.units_temp)
             flag = 1;
        }
    case MC_INS_UNITS_TIME:      // 时间
        if(!flag) {
            packet_instument.units_time =data1;
            LOGD("MC_INS_UNITS_TIME %d",packet_instument.units_time)
            flag = 1;
        }

    case MC_INS_UNITS_TIRE:      // 胎压
        if(!flag) {
            packet_instument.units_tirePressure =data1;
            LOGD("MC_INS_UNITS_TIRE %d",packet_instument.units_tirePressure)
            flag = 1;
        }
    case MC_INS_BRIGHTNESS_MODE:// 亮度
            if(!flag) {
            packet_instument.brightness_mode =data1 ;
            LOGD("MC_INS_BRIGHTNESS_MODE %d",packet_instument.brightness_mode)
            flag = 1;
            }
    case MC_INS_UNITS_BRIGHTNESS:// 亮度
        if(!flag) {
            packet_instument.brightness_level =data1 ;
            LOGD("MC_INS_UNITS_BRIGHTNESS %d",packet_instument.brightness_level)
            flag = 1;
            }
    case MC_INS_UNITS_LANGUAGE:  // 语言
        if(!flag) {
                packet_instument.language =data1;
                flag = 1;
                LOGD("MC_INS_UNITS_LANGUAGE %d",packet_instument.language)
        }
        size = sizeof(packet_instument);
       
        packet_instument.crc = mxc_crc16s_calc((unsigned char*)&packet_instument, size-2);
        
        ptr = (unsigned char*)&packet_instument;

        break;
        
    case MC_BTKEY_SEARCH:  //
        packet_btkey_cmd.vCmd = 0;
        size = sizeof(packet_btkey_cmd);
       
        packet_btkey_cmd.crc = mxc_crc16s_calc((unsigned char*)&packet_btkey_cmd, size-2);
        packet_btkey_scan_map.clear();
        ptr = (unsigned char*)&packet_btkey_cmd;
        LOGD("MC_BTKEY_SEARCH %s",ptr)
        break;

     case  MC_BTKEY_PAIR:{
        auto iter = packet_btkey_scan_map.find(data1);
        if(iter != packet_btkey_scan_map.end())
        {
            size = sizeof(packet_btkey_cmd);
            packet_btkey_cmd.vCmd = 1;
            strncpy(packet_btkey_cmd.vMacAddr,iter->second.vMacAddr,12) ;
            packet_btkey_cmd.vAddrType = iter->second.vAddrType;

            packet_btkey_cmd.crc = mxc_crc16s_calc((unsigned char*)&packet_btkey_cmd, size-2);
        
            ptr = (unsigned char*)&packet_btkey_cmd;
            LOGD("MC_BTKEY_PAIR %s",ptr);
            packet_btkey_scan_map.erase(iter);
        }
        break;
     }
     case  MC_BTKEY_PAIR_RES:{
            packet_btkey_cmd.vCmd = 3;
            size = sizeof(packet_btkey_cmd);
       
            packet_btkey_cmd.crc = mxc_crc16s_calc((unsigned char*)&packet_btkey_cmd, size-2);
            ptr = (unsigned char*)&packet_btkey_cmd;
            LOGD("MC_BTKEY_PAIR_RES %s",ptr)
        break;
     }
    case MC_BTKEY_DELETE:
    {
        auto iter = packet_btkey_bind_map.find(data1);
        if(iter != packet_btkey_bind_map.end())
        {
            LOGD("MC_BTKEY_DELETE %s ",iter->second.vMacAddr);
            packet_btkey_cmd.vCmd = 2;
            packet_btkey_cmd.vLinkIndex = iter->second.vLinkIndex;
            strncpy(packet_btkey_cmd.vMacAddr,iter->second.vMacAddr,12);
            size = sizeof(packet_btkey_cmd);
            packet_btkey_cmd.crc = mxc_crc16s_calc((unsigned char*)&packet_btkey_cmd, size-2);
        
            ptr = (unsigned char*)&packet_btkey_cmd;
     
            packet_btkey_bind_map.erase(iter);
        }
        else
            LOGD("MC_BTKEY_DELETE “未在绑定列表找到”%s ",iter->second.vMacAddr);
        break;
    }
    default:
        ret = -1;
        break;
    } 
    if(ptr){

         SendData(ptr,  size);
       //  LOGD("SendData %s",ptr)

        //  for(int i = 0;i < size;i++)
        //  {
        //     LOGD("%02X ",ptr[i]);
        //  }
         printf("\n");
    }
       
    return ret;
}

void McuParser::Proc()
{
	while(m_running)
	{
		if(m_notifyItems.size() > 0)
		{
            NotifyItem* item;

            pthread_mutex_lock(&m_mutex2);
            item = *m_notifyItems.begin();
            m_notifyItems.pop_front();
            pthread_mutex_unlock(&m_mutex2);   

            if(m_notifier != NULL && item->type == 1)
            {
                m_notifier(item->code, item->data1, item->data2);
                delete item;
            }
            else if(m_notifierStr != NULL && item->type == 2){
                m_notifierStr(item->code,item->data1,item->str);
                delete item;
            }
		}
        else 
            usleep(1000*50);
	}
}

void* McuParser::ThreadProc(void* arg) 
{
	McuParser* parser = (McuParser*)arg;
	parser->Proc();

	return (void*)NULL;
}


// 错误码30s未接收到 会通知 ，此时P2 是 2
void McuParser::ThreadErrCodeCheck()
{
    while (true)
    {
        time_t curtime;
        time (&curtime);
       // LOGD("ThreadErrCodeCheck,curtime %d,errCodeMapsize:%d",curtime,errCodeMap.size());
        usleep(1000*1000); //水一秒

        std::lock_guard<std::mutex> lock(m_errMtx);
        // LOGD(" errCodeMap.size %d ",errCodeMap.size());
        for(auto iter  =  errCodeMap.begin();iter!= errCodeMap.end();)
        {
            // LOGD(" code : %d,adddTime: %d,curtime: %d ", iter->second->errCode,iter->second->adddTime,curtime);
            if(curtime - iter->second->adddTime > 30){    
                // 通知qml 去除这个码
                LOGD(" remove   MN_ERR_CODE, %d )",iter->second->errCode);
                AddToNotify(MN_ERR_CODE, iter->second->errCode,2);  
                iter = errCodeMap.erase(iter);  
            }
            else{
                // AddToNotify(MN_ERR_CODE, iter->second->errCode,1);   // 移除应当在开机初始化后主动获取
                iter++;
            }
        }
    }
}

  

void McuParser::AddErrCode(int code){
    time_t timep;
    time (&timep);
    if(errCodeMap.size()> 100){ // 故障码超过100则不再响应
         LOGD("errcode exceeds a preset range  100");
        return ;
    }
        
    std::lock_guard<std::mutex> lock(m_errMtx); 
    auto iter  = errCodeMap.find(code);
    if(iter == errCodeMap.end())  {
        auto ret = errCodeMap.emplace(code,new errCodeRef{code,1,timep});
        iter = ret.first;
        AddToNotify(MN_ERR_CODE, code,1); // 通知qml 添加
        LOGD(" new  (MN_ERR_CODE, %d,1)",code);
    }
    else{
        iter->second->times++;
        iter->second->adddTime = timep;
      //  LOGD(" has add (MN_ERR_CODE, %d,1)",code);
    }
    //errCodequeue.push(iter->second); 

}


static McuParser::keyEvent keyarr[8] = {0};

__time_t getTime(){
    struct timeval start;
    gettimeofday( &start,NULL );
   
    __time_t timeuse =1000 * start.tv_sec  + start.tv_usec * 0.001; // 
   
    return timeuse;
}

void McuParser::KeyChange(uint8_t  key_state)
{
    __time_t timep = getTime();
   
    LOGD("MCU KeyChange %ld",timep);
    int curindex = 0;
    switch (key_state)
    {
    case KEY_VALUE_NONE:
        // 如果是0 检测每一个keyDown 值非0 的键 ，依据时间发送消息
        KeyEventProc(timep);
        break;
    case KEY_VALUE_RETURN : //   = 0x01,         // 返回键
        if(keyarr[1].keyDown  == 0)
            keyarr[1].keyDown = timep;
        break;
    case KEY_VALUE_PREV :   //     = 0x02,         // 上一曲
         if(keyarr[2].keyDown  == 0)
            keyarr[2].keyDown = timep;
            break; 
    case KEY_VALUE_PLAY:  //     = 0x04,         // 播放键
         if(keyarr[3].keyDown  == 0)
            keyarr[3].keyDown = timep;
            break;  
    case KEY_VALUE_NEXT :  //    = 0x08,         // 下一曲
         if(keyarr[4].keyDown  == 0)
            keyarr[4].keyDown = timep;
            break; 
    case KEY_VALUE_VOLUP :  //    = 0x10,         // 音量加
         if(keyarr[5].keyDown  == 0)
            keyarr[5].keyDown = timep;
            break; 
    case KEY_VALUE_VOLDOWN:  //    = 0x20,         // 音量减
         if(keyarr[6].keyDown  == 0)
            keyarr[6].keyDown = timep;
            break; 
    case KEY_VALUE_UNLOCK :  //    = 0x40,         // 解锁键 
         if(keyarr[7].keyDown  == 0)
            keyarr[7].keyDown = timep;
            break;
    default:
        break;
    }

}
// todo 短按
void McuParser::KeyEventProc( time_t timep)
{
 //   LOGD(" KeyEventProc"); 
    for(int i = 1;i < 8;i++) 
    {
        if(keyarr[i].keyDown)
        {
            int dif = timep - keyarr[i].keyDown ;
            if(dif< 900  &&  dif > 0){
                AddToNotify(MN_KEY, i);
               // LOGD(" AddToNotify(MN_KEY, i);"); 
            }       
            keyarr[i].keyDown = 0;
            keyarr[i].sendTimes = 0;
        }
    }
}
// todo 持续按压处理,解决按压时间感觉长的问题
void McuParser::ThreadKeyHold()
{
    while (true)
    {
       __time_t timep = getTime();
       // LOGD("ThreadErrCodeCheck,curtime %d,errCodeMapsize:%d",curtime,errCodeMap.size());
        usleep(1000*100); //水 100m s
      //  LOGD("ThreadKeyHold tt: %s",tt);
        for(int i = 1;i < 8;i++) // 检测一遍
        {
            int diff = timep - keyarr[i].keyDown;
            if(keyarr[i].keyDown && (diff > 900)){
                
                int tt = diff/1000 ;
                //LOGD("ThreadKeyHold diff : %d ,tt: %d",diff,tt);
                if(keyarr[i].sendTimes < tt){
                    ++keyarr[i].sendTimes;
                    AddToNotify(MN_KEY, i*100,diff);
                }
            }   
        }
        
    }
}

/* MXC PROTOCOL CRC16 CALCULATE */
static const uint16_t ccitt_table [256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


uint16_t mxc_crc16s_calc(const uint8_t *q, uint32_t len)
{
    uint16_t crcValue = 0;

    while (len > 0)
    {
        crcValue =  ((crcValue & 0xff) << 8) ^ ccitt_table [(((crcValue & 0xFF00) >> 8) ^ *q) & 0xff];
        len--;
        q++;
    }

    return crcValue;
}