#pragma once 


#include "PluginInterface.h"
#include <thread>
enum OP_TYPE{
  FM_INIT,   //
  FM_STOP,
  FM_START,
  FM_SEEK,
  FM_SEEK_STOP,        // 手动停止搜台
  FM_SEEK_STEP_LEN,    // 设置搜台歩长
  FM_SEEK_OVER,        // 搜台结束
  FM_SLEEP,            // 休眠
  FM_VOLUME,           // 设置音量
  FM_RESET,
  FM_SET_TUNE,
  FM_SET_MODE
};

class PluginRDAFM5808 :public PluginInterface
{
public:

    PluginRDAFM5808();
    virtual ~PluginRDAFM5808();
    int execute(int key,std::string const &data);
    int executeExexecuteEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0){};
    

    std::string GetPluginName()
    {
      return "rad5807";
    }

    int startSeek();
  private:
    void autoSeekThread();
    // 停止搜台
    void stopSeek();
    bool seeking;
    std::thread mth;
};

/**
 *@description: 实例化
 *@author: sky
 *@return 返回对象指针
 *@date: 2024-06-25 17:56:45
*/
extern "C" PluginInterface* createPlugin() {
    return new PluginRDAFM5808();
}
// 释放删插件，确保插件未在使用中
extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
