#include "ProxyLoadPlugin.h"
#include <dlfcn.h> // 动态链接库
#include <dirent.h>
#include <cstring>
#include <string.h>
#include <vector>
#include <map>
#include <thread>
#include <sys/types.h> 
#include <unistd.h>
#include "PluginInterface.h"

extern "C"
{
static std::vector<PluginInterface*> plugins; // 创建出的插件实例
static std::vector<void*>  pluginHandles;     // 加载的插件库的句�?,卸载时使�?
static std::map<std::string,int>   name_id;             // 插件名对应的ID

static int plugin_count = 1;                       

typedef PluginInterface* (*CreatePluginFunc)();

int LoadPlugnins()
{
    std::cout << "Loading Plugnins from  /apps/plugins " << std::endl;
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

           
            std::cout << "Founded plugins:  " <<  entry->d_name << std::endl ;
        }
    }
    closedir(dir);

    std::cout << "plginspath.size(): " << plginspath.size() <<  std::endl;
    plugins.resize(plginspath.size()+2);
    pluginHandles.resize(plginspath.size()+2);

    for(auto iter:plginspath) {
        int res = LoadPlugin(path + "/" + iter);
    }
    return 0;
}

int LoadPlugin(std::string const & path) {

    // 加载插件
    void* pluginHandle = dlopen(path.c_str(),RTLD_LAZY) ;//(path.c_str(),RTLD_LAZY);//("./plugins/libplug1.so", RTLD_LAZY);
    if (!pluginHandle) {
        
        std::cout << "Failed  load plugin: " << path.c_str() <<" ,dlerror: " << dlerror() ;
        return -1;
    }

    //
    CreatePluginFunc createPlugin = reinterpret_cast<CreatePluginFunc>(dlsym(pluginHandle, "createPlugin"));
    if (!createPlugin) {
        std::cout << "Failed to  get function createplugin : " << dlerror() << std::endl;
        dlclose(pluginHandle);
        return -1;
    }

    PluginInterface* plugin = createPlugin();
    if(!plugin)
    {
        std::cout << "ERROR   create Plugin: " << path.c_str() << std::endl;
        return -1;
    }

    std::cout <<  "createPlugin path : " << path.c_str() << std::endl;
    std::string pname = plugin->GetPluginName();
    
    plugins[plugin_count] = plugin;
    
    pluginHandles[plugin_count] = pluginHandle;

    name_id[pname] =plugin_count;
    plugin->setPid(plugin_count);
 
    ++plugin_count;
    return plugin_count;
}



rtn_int getPluginByName(std::string name)
{
    if(name_id.count(name) >0)
        return name_id[name];

    std::cout << "ERROR requset unkonw plugin name ,maybe load faild or spelling error" << name.c_str() << std::endl;
    return -1;  
}

/**
*
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
 * 
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

    //
    dlclose(pluginHandles[pid]);

    return 0;
 }
 

void addMonitor(const CallBackFuncType & cb )
{
    for(auto & iter:plugins){
        if(iter)
        {
            iter->registerLisnter(cb);
        }
            
    }
}

void addMonitorEx(const CallBackFuncIIIS & cb )
{
    for(auto & iter:plugins){
        if(iter)
        {
            iter->registerLisnter(cb);
        }
           
    }
}

}