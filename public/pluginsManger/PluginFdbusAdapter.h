/**
 *@brief: 作为插件接口和fdbus的通用适配接口
          集成该类来使用,其实是用plugin 的插件接口封装了 fdbus 的接口
 *@since 
 *@date: 2025-01-08 17:36:33
*/


#pragma once
#include <PluginInterface.h>
#include <FDbusClientInterface.h>

class FDBMsg;
class PluginFdbusAdapter:public PluginInterface,private FDbusClientInterface
{
 public:
   
  // PluginFdbusAdapter():FDbusClientInterface("qqqqqq"){};

  PluginFdbusAdapter(std::string name):FDbusClientInterface(name),cutNum(-1){}

    ~PluginFdbusAdapter(){};


    void ConnectServer(std::string const & name);

    /**
     *@brief: 用来简单区分同步异步的方式，大于 num 的 进行同步调用，不包括 num
     *@author: sky
     *@version 
     *@param num[in] 输入参数
     *@since 
     *@date: 2025-01-10 15:30:00
    */
    int SetAsyncSplitNum(int num);
   
     void AddSubscribe(std::vector<int> &msgIDs);


    void AddSubscribe(int minid,int maxid);
    int execute(int key,std::string const &data);  // 该接口将弃用

 


  
  
    /**
    *@brief: 同步调用接口，目前受 cutNum 影响来分别同步异步调用
    *@author: sky
    *@version 
    *@param i2[in] 输入参数2
    *@param o3[out] 输出参数1
    *@since 
    *@date: 2025-01-10 15:30:00
    */

    virtual int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

   
    virtual int executeAsync(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

    int OnNotify(int msgid,FDBMsg const &msg);

  private:
    int cutNum ;
};

