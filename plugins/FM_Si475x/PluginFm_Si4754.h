#pragma once 



#include "PluginInterface.h"
#include <thread>
enum OP_TYPE{

  RD_SET_STOP,             // 收音机停止
  RD_SET_START,            // 启动
  RD_SET_SEEK,             // 启动搜台  "0",逆向搜台; "1" 正向搜台
  RD_SET_SEEK_STOP,        // 手动停止搜台
  RD_SET_SEEK_STEP_LEN,    // 设置搜台歩长 unit : 10k for fm, unit 1k for am
  RD_SET_SLEEP,            // 休眠
  RD_SET_VOLUME,           // 设置音量
  RD_SET_MUTE,             // 设置静音 1 静音 ;0 不静音
  RD_SET_TUNE,             // 设置频率  FM  10800 ~ 8750 ; AM  1710 ~ 522,AM 经常搜不到东西
  RD_SET_MODE,             // 设置当前模式 FM 0；AM 1
  RD_SET_RESET,

  RD_GET_STATUS,           // 获取收音机状态 0 关闭； 1 运行中
  RD_GET_VOLUME,           // 获取当前音量
  RD_GET_MUTE,             // 获取当前是否静音
  RD_GET_TUNE,             // 获取当前频率 
  RD_GET_MODE,             // 设置当前模式 AM / FM

  RD_NOTIFY_SEEK_OVER,        // 搜台结束回调通知
  RD_NOTIFY_SEEK_CUR_FREQ,         // 搜台当前频率
  RD_NOTIFY_SEEK_GOOD_FREQ,        // 搜台搜到的OK的频率


  RD_JSON              // JSOn 数据
};

class PluginFm_Si4754 :public PluginInterface
{
public:

    PluginFm_Si4754();
    virtual ~PluginFm_Si4754();
    int execute(int key,std::string const &data);
    
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);
    std::string GetPluginName()
    {
      return "fm_4754c";
    }

    int startSeek(int dir);
  private:
    void autoSeekThread();
    int PraseJson(const char *  raw_string);
    // 停止搜台
    void stopSeek();
  //  bool seeking;
    std::thread mth;
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
