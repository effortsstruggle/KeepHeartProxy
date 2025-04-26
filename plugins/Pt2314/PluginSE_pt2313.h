/**
 *@brief: 音效芯片插件实现
 *@author: sky

 *@since 
 *@date: 2024-09-12 10:18:43
*/



#pragma once 


#include "PluginInterface.h"

#include <thread>
enum PT2314_TYPE{

};

class PluginSe_pt2313 :public PluginInterface
{
public:

    enum{
        SE_INIT,        // 初始化 ，只调用一次
        SE_CLOSE,
        SE_SET_VOLUME,  // 设置音量 0..63
        SE_SET_MUTE,    // 设置静音 1 静音； 0 取消静音
        SE_SET_SOURCE, // 目前已知的通道 1，2，3，收音机使用的是 3 蓝牙 ，2 收音机 ，1 soc 
        SE_GET_STATUS, // 是否打开成功
        SE_MAX_SIZE
    };
    PluginSe_pt2313();
    virtual ~PluginSe_pt2313();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

    std::string GetPluginName()
    {
      return "pt2313";
    }


  private:
  
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

