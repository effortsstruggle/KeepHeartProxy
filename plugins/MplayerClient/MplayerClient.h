#pragma once 
/**
 *@brief: 视频播放客户端 ，只集成了dvr 的mplayer 接口
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-10-16 10:17:34
*/

#include "PluginInterface.h"


class MplayerClient : public PluginInterface
{
public:
     enum MsgTypeForQML //本地音乐播放枚举
  {
      P_SET_PAUSE = 0, 	    // 暂停;
      P_SET_GOON,  	        // 继续播放
      P_SET_URL, 	        // 设置播放文件
      P_SET_STOP,           // 设置播放结束
      P_SET_PLAY_SPEED,     // 播放速度
      P_SET_SETPROGRESS,    // 设置播放进度
      P_SET_SETVOLUME,      // 设置音量
 
      P_NT_VOLUME,          // 获取音量
      P_NT_CURPROGRESS,     // 当前播放进度
      P_NT_TICK_TOTAL,      //  总时长
      P_NT_MUSICINFO,       // 音乐信息
      P_NO_MUSIC,           // 没有音乐文件
      P_NT_MUSICLISTS,       // 音乐文件列表 ，回调消息与发送都是这个
      P_NT_PLAYSTAE  ,     //播放结束，一首歌播放结束  0,结束，1 没有结束

      P_JSON
  };
    MplayerClient();
    virtual ~MplayerClient();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);
    int PraseJsonCommand(const char*  ptr);

    int skyMediaOperator(int p1,int p2,const char*  p3);

    void onMediaMsg(char* str);

    std::string GetPluginName()
    {
      return "mplayerClient";
    }

  
private:
 
    bool ismediaOk;
    std::string cururl;
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
