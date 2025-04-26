#pragma once

#include <PluginFdbusAdapter.h>

#define FDBUSDEMO_NAME "fdbusDemo"


class fdbusDemo :public  PluginFdbusAdapter
{
public:
     fdbusDemo();
    ~fdbusDemo(){};

    std::string GetPluginName() 
    {
        return FDBUSDEMO_NAME;
    }
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


