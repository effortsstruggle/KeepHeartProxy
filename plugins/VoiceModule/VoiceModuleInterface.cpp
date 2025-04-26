
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <android/log.h>
#include <mutex>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <condition_variable>

#include "McuSerial.h"
#include "VoiceModuleParser.h"
#include "VoiceModuleInterface.h"
#include "VoiceModuleDef.h"
#include <memory.h>

#include <queue>


#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_VoiceModule", __VA_ARGS__);


VoiceModuleInterface * instance = new VoiceModuleInterface ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}

VoiceModuleInterface::VoiceModuleInterface(){

    _serial  = new McuSerial;
    _parser = new VoiceModuleParser ;

    if(voice_module_open("/dev/ttyS5") == -1){
        LOGD("voice_module_open /dev/ttyS5 : FAILED");
    }

    voice_module_set_notify();

    // 获取语音模块版本
    voice_moduleu_set(NT_VER,0,0); 
}
VoiceModuleInterface::~VoiceModuleInterface()
{
    delete _serial;
    delete _parser;
}
int VoiceModuleInterface::execute(int key,std::string const &data){
     LOGD( "ERROR : VoiceModuleInterface execute ,interface deprecated");
}
int VoiceModuleInterface::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
     
    LOGD( "\nVoiceModuleInterface executeEx key:%d,data:%s ",key,data);

    voice_moduleu_set(key,p1,p2);
    
    return 0 ;
}



int VoiceModuleInterface::voice_module_open(const char* port)
{
	if(!_serial->Open(port, 115200, 8, 'N', 1))
		return -1;

	_serial->SetParser(_parser);
	_serial->StartListen();

	return 0;
}

int VoiceModuleInterface::voice_module_close()
{
	_serial->Close();

	return 0;
}


int VoiceModuleInterface::voice_moduleu_set(int cmd, int data1, long data2)
{
	return _parser->SendCmd(cmd, data1, data2);
}
static void  OnMcuNotify(int code, std::string &data)
{
    instance->Notify(code,data);
}



void VoiceModuleInterface::voice_module_set_notify()
{
	_parser->SetNotify(OnMcuNotify);

}