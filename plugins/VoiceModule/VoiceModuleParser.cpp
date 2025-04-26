#include <unistd.h>
#include <stdio.h>
#include "VoiceModuleParser.h"
#include "VoiceModuleDef.h"
#include "VoiceData.h"
#include <string.h>
#include "LoopBuffer.h"
#include <thread>
#include <mutex>
#include <functional>
#include <sys/time.h>
#include "cJSON.h"
#include "ver.h"
// #include <AsyncSerialCall.h>

static const short MAX_NT =0x100;
static tagPacketData g_packdata;
static tagPacketVer g_packVer;

static  uint8_t CMD_ARR[CMD_MAX] ;

static  short NT_ARR[MAX_NT];

 

static void PrintData( uint8_t * raw,const char * tag)
{
    tagPacketHead * taghead  = (tagPacketHead *)raw;
    if(taghead->length == 4)
    {
        LOGD("%s,%#02X %#02X %#02X %#02X ",tag,raw[0],raw[1],raw[2],raw[3]);
    }
    else if (taghead->length == 5){
         LOGD("%s version ,%#02X %#02X %#02X %#02X %#02X ",tag,raw[0],raw[1],raw[2],raw[3],raw[4]);
    }
    

}
VoiceModuleParser::VoiceModuleParser()
{
    LOGD("BuildVersion: %s",BuildVersion);
    memset(NT_ARR,0,sizeof(NT_ARR));
    m_notifier = NULL;    

    CMD_ARR[CMD_CHECK_SUCCESS] = 0x80;
    CMD_ARR[CMD_CHECK_FAILD] = 0x81;
    CMD_ARR[CMD_GET_VER] = 0x82;
    CMD_ARR[CMD_NO_SUPPORT] = 0x8F;
    CMD_ARR[CMD_NO_PLAYING] = 0x90;
    CMD_ARR[CMD_OPEN_AC_FIRST] = 0x91;
    CMD_ARR[CMD_HAO_DE] = 0x83;
    CMD_ARR[CMD_HAO_MUSIC_STOPED] = 0x84;
    CMD_ARR[CMD_HAO_AC_OPENED] = 0x85;
    CMD_ARR[CMD_HAO_AC_CLOSED] = 0x86;
    CMD_ARR[CMD_HAO_TEMP_RISEN] = 0x87;
    CMD_ARR[CMD_HAO_TEMP_DROPED] = 0x88;
    CMD_ARR[CMD_HAO_OPEND_HOME] = 0x89;
    CMD_ARR[CMD_HAO_OPEND_360] = 0x8A;
    CMD_ARR[CMD_HAO_OPEND_SETTING] = 0x8B;
    CMD_ARR[CMD_HAO_AUTO_DRIVE] = 0x8C;
    CMD_ARR[CMD_HAO_OPEND_EC] = 0x8D;
    CMD_ARR[CMD_HAO_OPEND_APPCENTER] = 0x8E;
    CMD_ARR[CMD_WAKE_UP_SAY_HI] = 0x92;
    CMD_ARR[CMD_QUIT_SEY_BYE] = 0x95;
   
   
//----------------------------------------------------------
    NT_ARR[1] = NT_PLAY_MUSIC;
    NT_ARR[2] = NT_PLAY_MUSIC;

    NT_ARR[3] = NT_PRE_MUSIC;
    NT_ARR[4] = NT_PRE_MUSIC;

    NT_ARR[5] = NT_NEXT_MUSIC;
    NT_ARR[6] = NT_NEXT_MUSIC;

    NT_ARR[7] = NT_MUSIC_PAUSE;
    NT_ARR[8] = NT_MUSIC_PAUSE;

    NT_ARR[9] = NT_OPEN_AC;
    NT_ARR[10] = NT_OPEN_AC;

    NT_ARR[0x0B] = NT_CLOSE_AC;
    NT_ARR[0x0C] = NT_CLOSE_AC;

    NT_ARR[0x0D] = NT_TEMP_RISEN;
    NT_ARR[0x0E] = NT_TEMP_RISEN;

    NT_ARR[0x0F] = NT_TEMP_DROPED;
    NT_ARR[0x10] = NT_TEMP_DROPED;

    NT_ARR[0x11] = NT_OPEN_HOME;
    NT_ARR[0x12] = NT_OPEN_HOME;

    NT_ARR[0x13] = NT_OPEN_360;
    NT_ARR[0x14] = NT_OPEN_360;

    NT_ARR[0x15] = NT_OPEN_SETTING;
    NT_ARR[0x16] = NT_OPEN_SETTING;

    NT_ARR[0x17] = NT_OPEN_AUTO_DRIVE;
    NT_ARR[0x18] = NT_OPEN_AUTO_DRIVE;

    NT_ARR[0x19] = NT_OPEN_OPEND_EC;
    NT_ARR[0x1A] = NT_OPEN_OPEND_EC;

    NT_ARR[0x1B] = NT_OPEN_APPCENTER;
    NT_ARR[0x1C] = NT_OPEN_APPCENTER;

    NT_ARR[0x1D] = NT_VOICE_MODE_QUIT;
    NT_ARR[0x1E] = NT_VOICE_MODE_QUIT;
    NT_ARR[0x93] = NT_SOUND_WAKE_UP;

    NT_ARR[0x96] = NT_SOUND_PLAY_OVER;
    NT_ARR[0x9C] = NT_SOUND_PLAY_OVER;
    NT_ARR[0x94] = NT_SOUND_PLAY_OVER;
    NT_ARR[0x9E] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xA0] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xA2] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xA4] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xA6] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xA8] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xAA] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xAC] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xAE] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xB0] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xB2] = NT_SOUND_PLAY_OVER;
 // NT_ARR[0x94] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xB2] = NT_SAY_BYE_OVER;
    NT_ARR[0xB4] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xB5] = NT_SOUND_PLAY_OVER;
    NT_ARR[0xB6] = NT_SOUND_PLAY_OVER;

}

VoiceModuleParser::~VoiceModuleParser()
{

}

int VoiceModuleParser::SendCmd(int cmd, int data1, long data2){

    tagPacketData tpd ;
    tpd.head.flag = 0xBA;
    tpd.head.length = 0x04;
    tpd.command  = CMD_ARR[cmd];
    
    tpd.crc = tpd.command + tpd.head.flag + tpd.head.length;
    char * ptr;
    ptr = (char*)&tpd;

    PrintData(( uint8_t*)ptr,"sendData");
    SendData(ptr, tpd.head.length);

 }

int VoiceModuleParser::ParseData( LoopBuffer& buf)
{
    int len  = buf.GetSize();

    // 是否够一个包头的大小
    if( len < sizeof(tagPacketHead)){
        LOGD("len < sizeof(tagPacketHead): %d",len);  
        return -1;
    }         
    
    tagPacketHead   taghead ;
    buf.GetData2((unsigned char*)&taghead,sizeof(tagPacketHead));
 
    // 检查标�? ,如果不对 扔掉 一个字�?
    if(taghead.flag != 0xBA){
        buf.Pop(1);
        return 1;
    }
    //一个包的长度，包头 + 数据 + 校验�?
    int pack_size =  taghead.length;
    
    if(len < pack_size)
    {
        LOGD("len = %d ,taghead.length = %d",len,pack_size);  
        return -1;
    }

     uint8_t * ptr  = nullptr;
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        if(pack_size == 4){
            buf.GetData2((unsigned char*)&g_packdata,pack_size);
            ptr = ( uint8_t *)&g_packdata;
        }
        else if(pack_size == 5)
        {
            buf.GetData2((unsigned char*)&g_packVer,pack_size);
            ptr = ( uint8_t *)&g_packVer;
        }
        buf.Pop(sizeof(pack_size));
    }

    
    
    uint8_t crc  = 0;// ;= g_packdata.head.flag + g_packdata.head.length + g_packdata.command;
    for(int i = 0 ;i < pack_size -1;i++)
    {
        crc += ptr[i];
    }
    // crc 校验
    PrintData(ptr,"Recv Data");

    uint16_t crc_res =crc != ptr[pack_size-1] ;
    if(crc_res ){
        
        LOGD("packet_btkey  crc error crc = %#02X,packagecrc = %#02X",crc, ptr[pack_size-1] );  
        PrintData(ptr,"ERR Data");
        buf.Pop(len);

        return -1;
    } 
   

    if(pack_size == 4)
    {
        if(g_packdata.command < 0 || g_packdata.command > MAX_NT)
        {
            LOGD("ERROR Unknow Command %#02X",g_packdata.command);  
            return -1;
        }
        if(NT_ARR[g_packdata.command])
            VoiceNotify(NT_ARR[g_packdata.command],0);
    }
    else if(pack_size == 5){
         int ver = g_packVer.command[0] << 8|  g_packVer.command[1];
         LOGD( "VOICE VERSION : %d " , ver);
         VoiceNotify(NT_VER,ver);
    }
    return  -1;
        
}
void VoiceModuleParser::VoiceNotify(int cmd,int data)
{
    std::string str = std::to_string(data);
        if(m_notifier)
            m_notifier(cmd,str);
}
