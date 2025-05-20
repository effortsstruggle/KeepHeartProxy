/**
 *@brief: 配置读取存储等操作
*/


#pragma once 


#include "PluginInterface.h"
#include <Singleton/singleton.h>
#include <Async/AsyncCall.h>
#include <Sonic/sonic/sonic.h>

class PluginConfig :public PluginInterface
{
public:

    enum ePluginConfig{
        CFG_INIT,   // 打开
        CFG_CLOSE,  // 关闭
        CFG_READ,   // 读取 对于整数等类型直接返回
        CFG_WRITE,   // 写入
        CFG_RESET,   // 删除文件重置
        CFG_READ_JSON, // 要求返回json 字符串，在回调函数中
        CFG_READ_JSON_ALL , // 返回文件中所有数据

        CFG_READ_XXX ,  //read different config ( XXX_1 , XXX_2 , XXX_3 )
    };

    PluginConfig();
    virtual ~PluginConfig();
    /**
     * key : enum 
     * data : json 
     */
    virtual int execute(int key,std::string const &data = "" ) override;
    virtual int executeAsync(int key, std::string const &data = "" ) override ;

    virtual std::string GetPluginName() override 
    {
      return "PluginConfig";
    }

  
protected:

  static int asyncInit(FuncAndParam const &param );
  NotifyParam init();

  /**
   *@brief: 从json 中读取数据
  */
  static int asyncRead(FuncAndParam const &param );
  NotifyParam read( std::string stData );

  /**
   *@brief: 
  */
  static int asyncWrite(FuncAndParam const &param );
  NotifyParam write( std::string stData);

  /**
   *@brief: 删除配置文件重置
  */
  static int asyncReset(FuncAndParam const &param );
  NotifyParam reset();



  static int asyncClose(FuncAndParam const &param );
  NotifyParam close();


  static int asyncGetJson(FuncAndParam const &param );
  NotifyParam getJson( std::string str);

  static int asyncGetAll(FuncAndParam const &param );
  NotifyParam getAll();

  

private:
  /**
   *@brief: 写文件操作，同步到硬盘
  */
  NotifyParam sync();


  NotifyParam fillEmptyRoot();

  /**
   * @brief append json
   */
  std::string makeNotifyJson( NotifyParam &objNotifyParam);

private:

    sonic_json::Document m_oJsonDoc;
    AsyncCall *m_pAsyncCall ;
    std::string m_stCfgPath;
};
using Singleton_PluginConfig = Singleton<PluginConfig>;

/**
 *@description: 实例化
 *@author: sky
 *@return 返回对象指针
 *@date: 2024-06-25 17:56:45
*/
extern "C" PluginInterface* createPlugin() ;
// 释放删插件，确保插件未在使用中
extern "C" void destroyPlugin(PluginInterface* plugin) {
    // Singleton_PluginConfig::destroy();
}









//  // 创建 Document（根节点为对象）
//     sonic::json::Document doc;

//     // 添加基本字段
//     doc["user_id"] = 1001;
//     doc["username"] = "alice_123";
//     doc["is_vip"] = true;

//     // 嵌套对象：地址信息
//     sonic::json::Document address;
//     address["city"] = "Shanghai";
//     address["district"] = "Pudong";
//     address["coordinates"] = {31.2304, 121.4737};  // 数组
//     doc["address"] = address;

//     // 数组：用户标签
//     doc["tags"] = {"new_user", "vip", "active"};

//     // 数组中的对象：订单列表
//     sonic::json::Document orders;
//     for (int i = 0; i < 2; ++i) {
//         sonic::json::Document order;
//         order["order_id"] = 10000 + i;
//         order["amount"] = 99.9 * (i + 1);
//         orders.PushBack(order);
//     }
//     doc["orders"] = orders;

//     // 序列化为格式化 JSON 字符串
//     std::string json_str = doc.to_string(2);
//     std::cout << "Generated JSON:\n" << json_str << std::endl;
