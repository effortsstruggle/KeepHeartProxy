#include "ProxyLoadPlugin.h"
#include <dlfcn.h> // 动态链接库
#include <dirent.h>
#include <string.h>
#include "KeepHeartProxyLog.h"

ProxyLoadPlugin::ProxyLoadPlugin() 
    : m_s32PluginCount(0)
{


}   

ProxyLoadPlugin::~ProxyLoadPlugin()
{
    std::cout << "ProxyLoadPlugin::~ProxyLoadPlugin(1)" << std::endl; 
}               


int ProxyLoadPlugin::LoadPlugnins( const std::string &path )
{
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

    this->m_vecPlugins.resize(plginspath.size()+2);
    this->m_vecPluginHandles.resize(plginspath.size()+2);

    for(auto iter:plginspath) {
        int res = LoadPlugin(path + "/" + iter);
    }
    return 0;
}

int ProxyLoadPlugin::LoadPlugin(std::string const & path) {

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
    plugin->setPid( this->m_s32PluginCount );

    std::cout <<  "createPlugin path : " << path.c_str() << std::endl;

    std::string pname = plugin->GetPluginName();

    this->m_vecPlugins[ this->m_s32PluginCount ] = plugin;
    this->m_vecPluginHandles[ this->m_s32PluginCount ] = pluginHandle;
    this->m_mapNameToId[pname] = this->m_s32PluginCount ;
    

    ++this->m_s32PluginCount;

    return this->m_s32PluginCount;
}



int ProxyLoadPlugin::executeFunction(int plugID,int key, std::string const& data )
{
    if(this->m_s32PluginCount > plugID  && plugID >= 0)
        return  this->m_vecPlugins[plugID]->execute(key, data);
    else
        std::cerr << "execute unknow plugin id  " << plugID  << std::endl;
    
}

int  ProxyLoadPlugin::executeFunctionAsyn(int plugID,int key, std::string const & data )
{
   if( this->m_s32PluginCount > plugID  && plugID >= 0)
        return  this->m_vecPlugins[plugID]->executeAsync( key,data );
    else
        std::cerr << "executeEx unknow plugin id  " << plugID  << std::endl;
    
}


int ProxyLoadPlugin::closePluginHandles()
{

    std::cout << "ProxyLoadPlugin::closePluginHandles(1)" << std::endl;

    for( int i = 0 ; i < this->m_s32PluginCount ; i++ )
    {
        this->closePluginHandle(i);
    }

    std::cout << "ProxyLoadPlugin::closePluginHandles(2)" << std::endl;

    return 0;
}

int ProxyLoadPlugin::closePluginHandle(int pid)
{

    std::cout << "------- ProxyLoadPlugin::closePluginHandle(1) -------" << std::endl;

    dlclose( this->m_vecPluginHandles[pid] );

    std::cout << "------- ProxyLoadPlugin::closePluginHandle(2) -------" << std::endl;
    
    return 0;
}


void ProxyLoadPlugin::addMonitor(const CallBackFuncType & cb )
{
    for(auto & iter: this->m_vecPlugins )
    {
        if(iter)
        {
            iter->registerLisnter(cb);
        }      
    }
}

void ProxyLoadPlugin::addMonitorAsync(const AsynCallBackFuncType & cb )
{
    for(auto & iter: this->m_vecPlugins )
    {
        if(iter)
        {
            iter->registerLisnterAsyn(cb);
        }  
    }
}


int ProxyLoadPlugin::getPluginByName(std::string name)
{
    if( this->m_mapNameToId.count(name) <= 0 )
    {
        std::cout << "ERROR requset unkonw plugin name ,maybe load faild or spelling error" << name.c_str() << std::endl;
        return -1;  
    }

    return this->m_mapNameToId[name];
}
