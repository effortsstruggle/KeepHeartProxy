/**
 *@brief: 语音模块通信协议解析库，同时保持与上层qml的交互
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-10-22 10:28:55
*/

#pragma once 


#include "PluginInterface.h"
#include <thread>

class McuSerial;
class VoiceModuleParser;
class VoiceModuleInterface :public PluginInterface
{
public:

    VoiceModuleInterface();
    virtual ~VoiceModuleInterface();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);
    
    int 	voice_module_open(const char* port);
    int 	voice_module_close();
    int   voice_moduleu_set(int cmd, int data1, long data2);
    void	voice_module_set_notify();
    std::string GetPluginName()
    {
      return "voiceModule";
    }

  
  private:

  McuSerial* _serial ;
  VoiceModuleParser* _parser;

};

/**
 *@description: 实例化
 *@author: sky
 *@return 返回对象指针
 *@date: 2024-06-25 17:56:45
*/
extern "C" PluginInterface* createPlugin() ;
// 释放删插件，确保插件未在使用中
extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
