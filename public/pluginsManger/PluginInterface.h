#ifndef _PLUGIN_INTERFACE_
#define _PLUGIN_INTERFACE_

#include <functional>

using CallBackFuncType = std::function<int(int, int,std::string)> ;
using AsynCallBackFuncType = std::function<int(int, int,std::string)> ;

class PluginInterface {
public: 
    PluginInterface(){};
    virtual ~PluginInterface(){};
    /**
     *@description: interface call
    */
    virtual int execute(int key,std::string const &data = "") = 0;
    virtual int executeAsync(int key,std::string const &data = "") = 0 ;
    
    /**
     *@description: get plugin name
    */
    virtual std::string GetPluginName() = 0;

    /**
     *  register lisnter
     */
    void registerLisnter(const CallBackFuncType &cb){ this->m_pBackCallFunc = cb;};
    void registerLisnterAsyn(const AsynCallBackFuncType &cb){ this->m_pAsncBackCallFunc = cb;};
    /**
     * @brief set plugin id
    */
    void setPid(int pid){ this->m_s32PluginId = pid;};
    int getPid(){ return this->m_s32PluginId ; };


    void Notify( int key , std::string const &data )
    {
       if( this->m_pBackCallFunc )
           this->m_pBackCallFunc( this->m_s32PluginId , key , data);   
    }

    void NotifyAsyn( int key , std::string const &data )
    {
       if( this->m_pAsncBackCallFunc )
           this->m_pAsncBackCallFunc( this->m_s32PluginId , key , data);   
    }


private:
    int m_s32PluginId;
    CallBackFuncType m_pBackCallFunc;
    AsynCallBackFuncType m_pAsncBackCallFunc ;
};

#endif // _PLUGIN_INTERFACE_