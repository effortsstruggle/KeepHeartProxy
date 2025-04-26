/**
 *@brief: 获取消息后通过回调通知消息
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-10-22 10:28:55
*/

#pragma once 


#include "PluginInterface.h"
#include <thread>
#include <map>

class AsyncSerialCall;
class ErrPraser_glb :public PluginInterface
{
public:
   struct ErrInfo
   {
      std::string code;
      std::string name;
      std::string described;
   };
   
    enum ERR_PRASER{
      SET_LANGUEAGE,   // 设置语言

      GET_ERR_INFO,   // 通过故障码获取故障信息

      NT_ERR_INFO,
    };

    enum LANGUAGE_DEF{
      L_ENGLIASH = 0,
      L_CHINESE,

      L_MAX ,      //  
    };

    ErrPraser_glb();
    ~ErrPraser_glb();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);
    int executeAsync(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0){};
    
    std::string GetPluginName()
    {
      return "errPraser_glb";
    }

    
  
  void NotifyCaller(std::string code,std::string name,std::string described);
  int GetErrCode(std::string code);
  int ReadFile(std::string path,std::map<std::string,ErrInfo> & );

    // 文件读取
  LANGUAGE_DEF cur_language;
  std::map<std::string,ErrInfo>err_en;        // 英语的故障码
  std::map<std::string,ErrInfo>err_other;     // 其他语言的故障码
  AsyncSerialCall * asyncCall;
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
/*
  默认加载英语的故障玛，英语为常驻
  
  当设置语言类型时,加载该类型的故障码csv 
    加载失败时，设置当前语言为MAX，
  


  获取故障玛时，未设置语言类型;
    则显示英语的，
      如果英语的也不存在;上报unknow
  



  串行异步操作


表格中有大量的空白列，是否是有对表格整行进行单元格格式的设置？导致整行所有列的单元格都被激活了为有效区域了，按照下面步骤操作就可以的

1）删除空白列：选定含有数据区域的最后一列，ctrl+shift+方向键向右--右键--删除

2）然后回到第一个单元格，按住CTRL+SHIFT+END键 就会重新框选表格中所有的有效区域



读取语言配置后设置语言，

*/