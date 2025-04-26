/**
 *@brief: 导入目录下的插件,这个后边应该做成�?
 *@author: sky
 *@version 

 *@since 
 *@date: 2024-08-21 16:28:26
*/

#include <functional>
#include "PluginInterface.h"
typedef int rtn_int;
 extern "C"
{
    /**
     *@brief: 自动导入加载目录下的插件
     *@author: sky
     *@date: 2024-08-20 15:27:33
    */
    int LoadPlugnins();

    /**
     *@description: 
     *@author: sky
     *@param path[in] 加载路径下的 .so
     *@return 
        成功，返回id
        不成功返�? -1
     *@date: 2024-08-20 15:28:33
    */
    int LoadPlugin(std::string const & path);


    int getPluginByName(std::string name);

    /**
     *@description: 
     *@author: sky
     *@param plufinID[in] 输入参数1
     *@param key[in] 操作的枚举�?
     *@param data[in/out] 操作的数�? 
     *@param len[out] 输入数据的长�?
     *@return 
        -1 失败

     *@date: 2024-08-20 15:36:33
    */
   rtn_int executeFunction(int plufinID,int key,std::string const & data  = "");

   rtn_int executeFunctionEx(int plugID,int key, std::string const & data  = "",double p1 = 0,double p2= 0,double p3= 0,double p4= 0);

    int  closePlugin(int pid);
    
    void addMonitor(const CallBackFuncType & cb);
    void addMonitorEx(const CallBackFuncIIIS & cb );
}
 

