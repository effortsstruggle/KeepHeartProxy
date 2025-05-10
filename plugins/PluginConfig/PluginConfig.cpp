#include "PluginConfig.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> 

#include "./PluginConfigLog.h"


extern "C" PluginInterface* createPlugin() {

    return Singleton_PluginConfig::getInstance();
}


 
PluginConfig::PluginConfig()
    : m_stCfgPath("/home/qin/workspace/KeepHeartProxyDependsLib/data/keepheart.cfg") 
    // , m_pRootJson( nullptr ) 
    , m_pAsyncCall( nullptr )
{

     this->m_pAsyncCall = new AsyncCall();
     this->m_pAsyncCall->SetLogTag("plugin_config");
    // this->m_pAsyncCall->AddFunctionToQueue( );
}


PluginConfig::~PluginConfig()
{
    if( nullptr == this->m_pAsyncCall )
    {
        delete this->m_pAsyncCall ;
        this->m_pAsyncCall = nullptr ;
    }
}

int PluginConfig::execute(int key,std::string const &data)
{

    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;
    std::string stInputData = "" ;
   
    {
        JsonWrapper oJsonWrapper( data ) ;
        stInputData = oJsonWrapper.getString( "data1" );
    }


    switch (key)
    {
        case CFG_INIT:
        oNotifyParam = this->init();
            break;
        case CFG_CLOSE:
        oNotifyParam = this->close();
            break;     
        case CFG_READ:
        oNotifyParam = this->read( stInputData );
            break;
        case CFG_WRITE:
        oNotifyParam = this->write( stInputData );
            break;   
        case CFG_RESET:
        oNotifyParam = this->reset();
            break; 
        case CFG_READ_JSON:
        oNotifyParam = this->getJson( stInputData ) ;
            break;
        case CFG_READ_JSON_ALL:
        oNotifyParam = this->getAll();
            break;
        default:
        
        break;
    }


    {
        stNotifyJson = this->makeNotifyJson( oNotifyParam );
        this->Notify( key , stNotifyJson );
    }


    return oNotifyParam.m_s32Ret ; 
}

int PluginConfig::executeAsync( int key,std::string const &data )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError } ; 
    std::string stNotifyJson = "" ;
    std::string stInputData = "" ;

    {
        JsonWrapper objJsonWrapper( data ) ;
        stInputData = objJsonWrapper.getString( "data1" );
    }


    switch (key)
    {
    case CFG_INIT:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncInit , "" , 0 , 0 , 0 , 0 , "cfg_init")  );
        break;
    case CFG_CLOSE:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncClose , "" , 0 , 0 , 0 , 0 , "cfg_close") );
        break;     
    case CFG_READ:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncRead , stInputData , 0 , 0 , 0 , 0 , "cfg_read") );
        break;
    case CFG_WRITE:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncWrite , stInputData , 0 , 0 , 0 , 0 , "cfg_write") );
        break;   
    case CFG_RESET:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncReset , "" , 0 , 0 , 0 , 0 , "cfg_reset") );
        break; 
    case CFG_READ_JSON:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetJson , stInputData , 0 , 0 , 0 , 0 , "cfg_getjson") );
        break;
    case CFG_READ_JSON_ALL:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetAll , "" , 0 , 0 , 0 , 0 , "cfg_getall") );
        break;
    default:
        break;
    }

    return oNotifyParam.m_s32Ret ;
 }


int PluginConfig::asyncInit( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    // init
    oNotifyParam = Singleton_PluginConfig::getInstance()->init();
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_INIT , stNotifyJson );

    std::cout << "PluginConfig::asyncInit()  Generated JSON: " << stNotifyJson << std::endl;

    return oNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::init()
{   
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    FILE *pFile = nullptr;
    int s32FileSize = 0 ;
    char *pJsonStr = nullptr ; 

    // 打开文件

    pFile = ::fopen( this->m_stCfgPath.c_str() , "r");
    if ( pFile == NULL) 
    {
        oNotifyParam = this->fillEmptyRoot();
        std::cout << "PluginConfig::init() file open fail , path : " << this->m_stCfgPath << std::endl;
        return oNotifyParam ;
    }


    // 获得文件大小
    struct stat statbuf;
    ::stat( this->m_stCfgPath.c_str() , &statbuf);
    s32FileSize = statbuf.st_size;

    // 分配符合文件大小的内存
    pJsonStr = (char *)::malloc(sizeof(char) * s32FileSize + 1);
    ::memset( pJsonStr, 0, s32FileSize + 1);

    // 读取文件中的json字符串
    int size = ::fread( pJsonStr , sizeof(char),  s32FileSize ,  pFile);
    if (size == 0) {
        ::fclose( pFile );
        
        oNotifyParam = this->fillEmptyRoot();
        std::cout << "PluginConfig::init() fread file fail , path : " << this->m_stCfgPath << std::endl;
        return oNotifyParam ;

    }

    //close file
    ::fclose( pFile );

    // 将读取到的json字符串转换成json变量指针
    this->m_oJsonWrapper.parse( pJsonStr );

    ::free( pJsonStr );

    return oNotifyParam;
}



int PluginConfig::asyncRead( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" ,  NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    // read
    objNotifyParam = Singleton_PluginConfig::getInstance()->read( param.data.c_str());
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ , stNotifyJson );

    std::cout << "PluginConfig::asyncRead()  Generated JSON: " << stNotifyJson << std::endl;

    return objNotifyParam.m_s32Ret ; 

}

NotifyParam PluginConfig::read(std::string stData )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

   
    JsonWrapper oJsonWrapper = this->m_oJsonWrapper.get( stData );
    if( oJsonWrapper.isString() ) 
    {
        objNotifyParam.m_stSuccessInfo = oJsonWrapper.getString( stData );
    }
    else if( oJsonWrapper.isDouble() )
    {
        objNotifyParam.m_stSuccessInfo = oJsonWrapper.getDouble( stData );
    }
    else if( oJsonWrapper.isInt() )
    {
        objNotifyParam.m_stSuccessInfo =  oJsonWrapper.getInt( stData );
    }
    else if( oJsonWrapper.isBool() )
    {
        objNotifyParam.m_stSuccessInfo =  oJsonWrapper.getBool( stData );
    }
    else
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = eTypeError ;
    }

    return objNotifyParam;
}


int PluginConfig::asyncWrite( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;


    //WRITE
    objNotifyParam = Singleton_PluginConfig::getInstance()->write( param.data.c_str());
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_WRITE , stNotifyJson );

    std::cout << "PluginConfig::asyncWrite()  Generated JSON: " << stNotifyJson << std::endl;

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::write(std::string stData)
{

    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    // int res = 0;
    // char *temptr;
    // cJSON* pData = nullptr ;
    // if( nullptr == this->m_pRootJson )
    // {
    //     objNotifyParam.m_s32Ret = -1 ;
    //     objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

    //     //print
    //     std::cout << "PluginConfig::write() this->m_pRootJson is nullptr " << std::endl ;

    //     return objNotifyParam;
    // }

    // pData = cJSON_Parse(str);
    // if( nullptr == pData )
    // {
    //     objNotifyParam.m_s32Ret = -1 ;
    //     objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

    //     //print
    //     std::cout << "PluginConfig::write() pData is nullptr " << std::endl ;

    //     return objNotifyParam;
    // }
    
    // int num =  cJSON_GetArraySize( pData );
    // cJSON* next = pData->child;
    // while( next != nullptr )
    // {
        
    //     cJSON* item = cJSON_DetachItemViaPointer( pData , next); // 将next从Data 中移除
    //     // 判断是否存在此key
    //     if(cJSON_HasObjectItem( this->m_pRootJson , item->string)){   
           
    //         cJSON_DeleteItemFromObject( this->m_pRootJson , item->string);
    //     }

    //     res=  cJSON_AddItemToObject( this->m_pRootJson , item->string , item);
    //     if(res == false)
    //     {
    //         std::cout << " PluginConfig::write add object fail"<<  std::endl;
    //     } 
       
    //     next = pData->child; // next 已经从data 中移除 ，所以next 需要获取的是新的子结点
    // }
    // cJSON_Delete( pData );

    JsonWrapper oJsonWrapper( stData ); 

    objNotifyParam = this->sync();

    return objNotifyParam;
}


int PluginConfig::asyncReset( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //reset
    objNotifyParam = Singleton_PluginConfig::getInstance()->reset();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_RESET , stNotifyJson );
    std::cout << "PluginConfig::asyncReset()  Generated JSON: " << stNotifyJson << std::endl;


    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::reset()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError };     

    if ( -1 == ::remove( this->m_stCfgPath.c_str() ) )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;
        std::cout << "PluginConfig::reset() remove file fail. " << std::endl ;
        return objNotifyParam;
    } 

    //clear memory
    this->m_oJsonWrapper.clear();
    //reset
    objNotifyParam = this->init();

    return objNotifyParam;
}


int PluginConfig::asyncClose( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //close
    objNotifyParam =  Singleton_PluginConfig::getInstance()->close();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_CLOSE , stNotifyJson );
    std::cout << "PluginConfig::asyncClose()  Generated JSON: " << stNotifyJson << std::endl;

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::close()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    objNotifyParam = this->sync();
    
    //clear memory
    this->m_oJsonWrapper.clear();

    return objNotifyParam;
}


int PluginConfig::asyncGetJson( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //getJson
    objNotifyParam =  Singleton_PluginConfig::getInstance()->getJson( param.data.c_str() );
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON , stNotifyJson );
    std::cout << "PluginConfig::asyncGetJson()  Generated JSON: " << stNotifyJson << std::endl;

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::getJson( std::string stData )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    return objNotifyParam ;
}


int PluginConfig::asyncGetAll( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    // getAll
    objNotifyParam =  Singleton_PluginConfig::getInstance()->getAll();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON_ALL , stNotifyJson );
    std::cout << "PluginConfig::asyncGetJson()  Generated JSON: " << stNotifyJson << std::endl;

    return objNotifyParam.m_s32Ret ;
}

NotifyParam PluginConfig::getAll()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
 
    objNotifyParam.m_stSuccessInfo = this->m_oJsonWrapper.toString();

    return objNotifyParam ;
}   

NotifyParam PluginConfig::sync()
{

    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    // 打开文件
    FILE *file  = fopen( this->m_stCfgPath.c_str() , "w");
    if ( nullptr == file ) 
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        std::cout <<  "PluginConfig::sync() file is nullptr " << std::endl;

        return objNotifyParam;
    }

    std::string stData = this->m_oJsonWrapper.toString();
       
    // 写入文件
    int ret = fwrite( stData.c_str() , sizeof(char), stData.size() , file);
    fclose(file);  

       
       
    objNotifyParam.m_s32Ret = ret ;
    if (ret == 0) 
    { 
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eWriteFileError ;
        std::cout <<  "PluginConfig::sync() write fail. " << std::endl;
    }
    else //success 
    {
        objNotifyParam.m_stSuccessInfo = "write success" ;
        std::cout <<  "PluginConfig::sync() write success. " << std::endl;
    }

    return objNotifyParam ;
}

NotifyParam PluginConfig::fillEmptyRoot()
{
    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    this->m_oJsonWrapper.set("pw","111111");
    this->m_oJsonWrapper.set("enablePW","1");
    this->m_oJsonWrapper.set("inputTimes","6");
    this->m_oJsonWrapper.set("diMcuUnitTemper",0);
    this->m_oJsonWrapper.set("diMcuLanguage",0);
    this->m_oJsonWrapper.set("Units",0);
    this->m_oJsonWrapper.set("diMcuUnitTime",1);
    this->m_oJsonWrapper.set("controlbrightMode",0);
    this->m_oJsonWrapper.set("dashBoardtbrightMode",0);
    this->m_oJsonWrapper.set("diRadioArea",0);
    this->m_oJsonWrapper.set("gyro_init_x",0);
    this->m_oJsonWrapper.set("gyro_init_y",0); 

    objNotifyParam = this->sync();

    return objNotifyParam ;
}

std::string PluginConfig::makeNotifyJson( NotifyParam &objNotifyParam )
{

    JsonWrapper objJsonWrapper;
    objJsonWrapper.set( "result" ,  objNotifyParam.m_s32Ret );
    objJsonWrapper.set( "sucess_notify" , objNotifyParam.m_stSuccessInfo );
    objJsonWrapper.set( "error_notify" ,  objNotifyParam.m_eErrorCode );

    return objJsonWrapper.toString() ;
}


