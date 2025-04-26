
/**
 *@brief: 插件型结构的接口定义
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-08-20 15:15:58
*/

#ifndef PLUGIN_INTERFACE_
#define  PLUGIN_INTERFACE_

#include <iostream>
#include <functional>
//#include <>


// 插件ID,消息枚举,数据 ,如果不将数据转化�?
using CallBackFuncType = std::function<int(int, int,std::string,double ,double ,double ,double)> ;
// 插件ID,消息枚举,int数据，字符数据，如果不将数据转化�?
using CallBackFuncIIIS = std::function<int(int, int,int,std::string )> ;


class PluginInterface {
public: 
    PluginInterface(){};
    virtual ~PluginInterface(){};
    /**
     *@description: 同步调用接口，必须实现，字符串数据，在fdbus �? 有不同的意义
     *@author: sky
     *@param key [in] 命令;的key�?
     *@param data [in] 传入的参�?
     *@param datalen [in] 传入数据的长�?

     *@return 操作状态返回�?
            -1: 错误
            其他:看插件的实现
     *@date: 2024-08-20 15:12:22
    */
    virtual int execute(int key,std::string const &data) {return 0;};
    //如果没有字符数据，第二个参数也可用来传递参数，如何使用，看不同的插件实�?
    virtual int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0)= 0;

    /**
     *@description: 异步调用接口，必须实现，字符串数�?
     *@author: sky
     *@param key [in] 命令;的key�?
     *@param data [in] 传入的参�?
     *@return 操作状态返回�?
            -1: 错误
            其他:看插件的实现
     *@date: 2024-08-20 15:12:22
    */
    //如果没有字符数据，第二个参数也可用来传递参数，如何使用，看不同的插件实�?
    virtual int executeAsync(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0){return 0;};
    /**
     *@description: 返回调用的插件名字，其他地方加使用该名字获取ID
     *@author: sky
     *@return :plugin name
     *@date: 2024-08-20 15:10:30
    */
    virtual std::string GetPluginName() = 0;
    void registerLisnter(const CallBackFuncType &cb){mcb = cb;};
    void registerLisnter(const CallBackFuncIIIS &cb){mcbi3s = cb;};
    /**
     *@brief: 设置由插件加载其分配的ID
     *@author: sky
     *@param pid[in] 输入参数2
     *@date: 2024-08-21 16:52:03
    */
    void setPid(int pid){m_id = pid;};

    /**
     *  ������SkyProxyע��Ľӿ�
    */
    void Notify(int key,std::string const &data,double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0)
    {
       if(mcb)
           mcb(m_id,key,data,p1 , p2 ,  p3 , p4);
       
    }
     void Notify(int key,int data1, std::string const & data2)
    {
       if(mcbi3s)
           mcbi3s(m_id,key,data1,data2);
       
    }

    int m_id;
private:
    CallBackFuncType mcb;
    CallBackFuncIIIS mcbi3s;
};

#endif // DEBUG