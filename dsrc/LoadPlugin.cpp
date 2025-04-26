#include "LoadPlugin.h"
#include <dlfcn.h> // 动态链接库相关头文�?
#include <dirent.h>
#include <cstring>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include "PluginInterface.h"
#include "skylog.h"
#include <thread>
#include <sys/types.h> 
#include <unistd.h>
extern "C"
{
static std::vector<PluginInterface*> plugins; // 创建出的插件实例
static std::vector<void*>  pluginHandles;     // 加载的插件库的句�?,卸载时使�?
static std::map<std::string,int>   name_id;             // 插件名对应的ID

static int plugin_count = 1;                       

typedef PluginInterface* (*CreatePluginFunc)();

int LoadPlugnins()
{
    std::cout << "Loading Plugnins from \"/apps/plugins\" ..." << std::endl;
    std::string path = "/apps/plugins"; // 插件目录
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory " << path << std::endl;
        return -1;
    }
    std::vector<std::string> plginspath;
    struct dirent *entry;
   
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
             char tmp[3] = {0};
             std::string fname = entry->d_name;
          
            int res = fname.find_last_of('.');
            if(res > fname.size())
                continue;
                
             if(fname.substr(res) == ".so")
                plginspath.emplace_back(fname);

           
            SKYLOGD("Founded plugins: %s", entry->d_name);
        }
    }
    closedir(dir);

    std::cout << "plginspath.size(): " << plginspath.size() <<  std::endl;
    plugins.resize(plginspath.size()+2);
    pluginHandles.resize(plginspath.size()+2);

  //  std::cout << "222222222222" << plginspath.size() <<  std::endl;
    for(auto iter:plginspath) {
        int res = LoadPlugin(path + "/" + iter);
    }

 //   std::cout << "load over " <<  std::endl;
    return 0;
}

int LoadPlugin(std::string const & path) {

    // 加载插件
    void* pluginHandle = dlopen(path.c_str(),RTLD_LAZY) ;//(path.c_str(),RTLD_LAZY);//("./plugins/libplug1.so", RTLD_LAZY);
    if (!pluginHandle) {
        
        SKYLOGD( "Failed  load plugin:  %s ,dlerror:%s " ,path.c_str(),dlerror());
        return -1;
    }

    //
    CreatePluginFunc createPlugin = reinterpret_cast<CreatePluginFunc>(dlsym(pluginHandle, "createPlugin"));
    if (!createPlugin) {
        SKYLOGD( "Failed to  get function createplugin : %s" ,dlerror());
        dlclose(pluginHandle);
        return -1;
    }
    // ��ȡ������
    PluginInterface* plugin = createPlugin();
    if(!plugin){
        SKYLOGD("ERROR   create Plugin :%s  FAILED",path.c_str());
        return -1;
    }

    SKYLOGD("createPlugin : path %s ",path.c_str());
    std::string pname = plugin->GetPluginName();
    
    plugins[plugin_count] = plugin;
    
    pluginHandles[plugin_count] = pluginHandle;

    name_id[pname] =plugin_count;
    plugin->setPid(plugin_count);
  //  std::cout << "loaded plugins : " << pname << ",plugins id:" << plugin_count << std::endl;
    LOGD("LoadPlugin-plugin name %s,plugins id:",pname,plugin_count);
    ++plugin_count;
    return plugin_count;
}



rtn_int getPluginByName(std::string name)
{
    if(name_id.count(name) >0)
        return name_id[name];

    SKYLOGD("ERROR requset[ %s ] unkonw plugin name ,maybe load faild or spelling error",name.c_str());
    return -1;  
}
 /**
  * ִ�в��
  * �ҵ���Ӧ�����������ִ�нӿ�
  */
rtn_int  executeFunction(int plugID,int key, std::string const& data )
{
    if(plugin_count > plugID  && plugID > 0)
        return  plugins[plugID]->execute(key, data);
    else
        std::cerr << "execute unknow plugin id  " << plugID  << std::endl;
    
}

 // 使用插件功能
rtn_int  executeFunctionEx(int plugID,int key, std::string const & data ,double p1,double p2,double p3,double p4)
{
   if(plugin_count> plugID  && plugID > 0)
        return  plugins[plugID]->executeEx(key,data,p1,p2,p3,p4);
    else
        std::cerr << "executeEx unknow plugin id  " << plugID  << std::endl;
    
}


/**
 * �ͷŲ��
 */
int  closePlugin(int pid)
 {
    typedef void (*DestroyPluginFunc)(PluginInterface*);
    DestroyPluginFunc destroyPlugin = reinterpret_cast<DestroyPluginFunc>(dlsym(pluginHandles[pid], "destroyPlugin"));
    if (!destroyPlugin) {
        std::cerr << "Failed to get destroyPlugin function: " << dlerror() << std::endl;
        dlclose(pluginHandles[pid]);
        return 1;
    }
    
    destroyPlugin(plugins[pid]);

    //�رղ�����
    dlclose(pluginHandles[pid]);

    return 0;
 }
 

void addMonitor(const CallBackFuncType & cb )
{
    for(auto & iter:plugins){
        if(iter){
            //std::cout << iter->GetPluginName() << std::endl;
            iter->registerLisnter(cb);
        }
            
    }
}

void addMonitorEx(const CallBackFuncIIIS & cb )
{
    for(auto & iter:plugins){
        if(iter){
           // std::cout << iter->GetPluginName() << std::endl;
            iter->registerLisnter(cb);
        }
           
    }
}

}