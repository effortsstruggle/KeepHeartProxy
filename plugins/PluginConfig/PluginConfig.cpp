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
    : m_stCfgPath("/data/keepheart.cfg") 
    , m_pRootJson( nullptr ) 
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
    int val = 0;
    int ret = 0;
    
    switch (key)
    {
        case CFG_INIT:
            this->init();
            break;
        case CFG_CLOSE:
            this->close();
            break;     
        case CFG_READ:
            this->read( data.c_str() );
            break;
        case CFG_WRITE:
            this->write( data.c_str() );
            break;   
        case CFG_RESET:
            this->reset();
            break; 
        case CFG_READ_JSON:
            this->getJson( data.c_str() ) ;
            break;
        case CFG_READ_JSON_ALL:
            this->getAll();
            break;
        default:
        
        break;
    }
   
    return ret;
}

int PluginConfig::executeAsync( int key,std::string const &data ){

    int val = 0;
    int res = 0;
    
    switch (key)
    {
        case CFG_INIT:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncInit , "" , 0 , 0 , 0 , 0 , "cfg_init")  );
            break;
        case CFG_CLOSE:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncClose , "" , 0 , 0 , 0 , 0 , "cfg_close") );
            break;     
        case CFG_READ:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncRead , data , 0 , 0 , 0 , 0 , "cfg_read") );
            break;
        case CFG_WRITE:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncWrite , data , 0 , 0 , 0 , 0 , "cfg_write") );
            break;   
        case CFG_RESET:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncReset , "" , 0 , 0 , 0 , 0 , "cfg_reset") );
            break; 
        case CFG_READ_JSON:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetJson , data , 0 , 0 , 0 , 0 , "cfg_getjson") );
            break;
        case CFG_READ_JSON_ALL:
            this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetAll , "" , 0 , 0 , 0 , 0 , "cfg_getall") );
            break;
    default:
        break;
    }
   
    return res;
 };


int PluginConfig::asyncInit( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncInit() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam = Singleton_PluginConfig::getInstance()->init();

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);
    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncInit() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncInit()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_INIT , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::init()
{   
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    if( this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::init() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    // 打开文件
    FILE *file = NULL;
    file = ::fopen( this->m_stCfgPath.c_str() , "r");
    if (file == NULL) 
    {

        objNotifyParam = this->fillEmptyRoot();

        std::cout << "PluginConfig::init() file open fail , path : " << this->m_stCfgPath << std::endl;

        return objNotifyParam ;
    }


    // 获得文件大小
    struct stat statbuf;
    ::stat( this->m_stCfgPath.c_str() , &statbuf);
    int fileSize = statbuf.st_size;

    // 分配符合文件大小的内存
    char *jsonStr = (char *)::malloc(sizeof(char) * fileSize + 1);
    ::memset(jsonStr, 0, fileSize + 1);

    // 读取文件中的json字符串
    int size = ::fread(jsonStr, sizeof(char), fileSize, file);
    if (size == 0) {
        ::fclose(file);
        
        objNotifyParam = this->fillEmptyRoot();
        std::cout << "PluginConfig::init() fread file fail , path : " << this->m_stCfgPath << std::endl;
        return objNotifyParam ;

    }

    //close file
    ::fclose(file);

    // 将读取到的json字符串转换成json变量指针
    this->m_pRootJson  = ::cJSON_Parse(jsonStr);
    if (nullptr == this->m_pRootJson ) 
    {
        ::free(jsonStr);
                
        std::cout << "PluginConfig::init() json parse fial." << std::endl;
        return objNotifyParam ;
    }
  
    ::free(jsonStr);

    return objNotifyParam;
}



int PluginConfig::asyncRead( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" ,  NotifyErrorCode::eInvialdError }; 


    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncRead() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam = Singleton_PluginConfig::getInstance()->read( param.data.c_str());


    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);

    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncRead() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncRead()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 

}

NotifyParam PluginConfig::read(const char* str)
{

    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    if( this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::read() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    cJSON* pItem  = cJSON_GetObjectItem( this->m_pRootJson, str);	
    if( nullptr == pItem )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::read() pItem is nullptr " << std::endl ;
        return objNotifyParam;

    }

    // 判断是不是字符串类型
    if ( pItem->type == cJSON_String) 
    {	
        char * pStr = pItem->valuestring;		// 此赋值是浅拷贝，不需要现在释放内存

        objNotifyParam.m_s32Ret = 0 ;
        objNotifyParam.m_stSuccessInfo = pStr ;
    }
    else
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eTypeError ;

        //print
        std::cout << "PluginConfig::read() pItem->type is error " << std::endl ;
    }

    return objNotifyParam;
}


int PluginConfig::asyncWrite( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 


    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncWrite() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam = Singleton_PluginConfig::getInstance()->write( param.data.c_str());

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);
    
    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncWrite() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncWrite()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_WRITE , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::write(const char* str)
{

    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    int res = 0;
    char *temptr;
    cJSON* pData = nullptr ;

    if( this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::write() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    pData = cJSON_Parse(str);
    if( nullptr == pData )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::write() pData is nullptr " << std::endl ;

        return objNotifyParam;
    }
    
    int num =  cJSON_GetArraySize( pData );

    cJSON* next = pData->child;


    while( next != nullptr )
    {
        
        cJSON* item = cJSON_DetachItemViaPointer( pData , next); // 将next从Data 中移除
        // 判断是否存在此key
        if(cJSON_HasObjectItem( this->m_pRootJson , item->string)){   
           
            cJSON_DeleteItemFromObject( this->m_pRootJson , item->string);
        }

        res=  cJSON_AddItemToObject( this->m_pRootJson , item->string , item);
        if(res == false)
        {
            std::cout << " PluginConfig::write add object fail"<<  std::endl;
        } 
       
        next = pData->child; // next 已经从data 中移除 ，所以next 需要获取的是新的子结点
    }

    cJSON_Delete( pData );
 
    objNotifyParam = this->sync();

    return objNotifyParam;
}


int PluginConfig::asyncReset( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 


    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncReset() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam = Singleton_PluginConfig::getInstance()->reset();

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);


    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncReset() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncReset()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_RESET , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::reset()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    if( nullptr == this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::reset() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }
    

    if ( -1 == remove( this->m_stCfgPath.c_str() ) )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        std::cout << "PluginConfig::reset() remove file fail. " << std::endl ;
        return objNotifyParam;
    } 

    cJSON_Delete( this->m_pRootJson );
    this->m_pRootJson = NULL;

    objNotifyParam =  this->init();

    return objNotifyParam;
}


int PluginConfig::asyncClose( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncClose() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam =  Singleton_PluginConfig::getInstance()->close();

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);
    
    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncClose() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncClose()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_CLOSE , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 

}

NotifyParam PluginConfig::close()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    if( nullptr == this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::close() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }


    objNotifyParam = this->sync();


    cJSON_Delete( this->m_pRootJson );
    this->m_pRootJson = nullptr ;

    return objNotifyParam;
}


int PluginConfig::asyncGetJson( FuncAndParam const &param )
{

    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 


    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncGetJson() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam =  Singleton_PluginConfig::getInstance()->getJson( param.data.c_str() );

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);

    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncGetJson() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncGetJson()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::getJson(const char* str)
{

    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
 
    if( nullptr == this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::getJson() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    cJSON* pItem =  cJSON_DetachItemFromObject( this->m_pRootJson , str);
    if( nullptr == pItem )
    {   
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;
        //print
        std::cout << "PluginConfig::getJson() pItem is nullptr " << std::endl ;
        return objNotifyParam ;
    }
    

    std::string stdstr = str;
    if(stdstr.size() == 0)
    {
        objNotifyParam = this->getAll();
        return objNotifyParam;
    }

    cJSON * data2 = cJSON_CreateObject();

    cJSON_AddItemToObject(data2,str, pItem );
    char * ptr = cJSON_PrintUnformatted(data2);
    
   
    pItem =  cJSON_DetachItemFromObject(data2,str);

    cJSON_AddItemToObject( this->m_pRootJson , str , pItem);

    cJSON_free(ptr) ;
    cJSON_Delete(data2);

    return objNotifyParam ;
}


int PluginConfig::asyncGetAll( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 


    cJSON *pNotifyJson = nullptr ;
    char *pJson = nullptr ;
    
    pNotifyJson = cJSON_CreateObject();
    if ( nullptr == pNotifyJson ) 
    {
        std::cout << " PluginConfig::asyncGetAll() cJSON_CreateObject is fail" << std::endl;
        return -1;
    }

    // init
    objNotifyParam =  Singleton_PluginConfig::getInstance()->getAll();

    cJSON_AddNumberToObject(pNotifyJson, "result", objNotifyParam.m_s32Ret);
    cJSON_AddStringToObject(pNotifyJson, "sucess_notify", objNotifyParam.m_stSuccessInfo.c_str());
    cJSON_AddNumberToObject(pNotifyJson, "error_notify", objNotifyParam.m_eErrorCode);
    
    pJson = cJSON_Print(pNotifyJson);
    if (nullptr == pJson  ) {
        std::cout << " PluginConfig::asyncGetAll() cJSON_Print is fail" << std::endl;
        cJSON_Delete( pNotifyJson );  // 释放内存
        return -1;
    }
    
    std::cout << "PluginConfig::asyncGetAll()  Generated JSON: " << pJson << std::endl;

    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON_ALL , pJson );

    //释放内存
    free( pJson );
    cJSON_Delete( pNotifyJson );

    return objNotifyParam.m_s32Ret ;
}

NotifyParam PluginConfig::getAll()
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
 
    if( nullptr == this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::getAll() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    char * ptr = cJSON_PrintUnformatted( this->m_pRootJson );

    cJSON_free(ptr) ;

    return objNotifyParam ;
}   

NotifyParam PluginConfig::sync()
{

    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    if( this->m_pRootJson )
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        //print
        std::cout << "PluginConfig::sync() this->m_pRootJson is nullptr " << std::endl ;

        return objNotifyParam;
    }

    // 打开文件
    FILE *file  = fopen( this->m_stCfgPath.c_str() , "w");
    if ( nullptr == file ) 
    {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

        std::cout <<  "PluginConfig::sync() file is nullptr " << std::endl;

        return objNotifyParam;
    }

    char *cjValue = cJSON_Print( this->m_pRootJson );
    if(cjValue == nullptr)
    {
         fclose(file);
         
         objNotifyParam.m_s32Ret = -1 ;
         objNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;

         std::cout <<  "PluginConfig::sync() cjValue is nullptr " << std::endl;

         
         return objNotifyParam;
    }
       
    // 写入文件
    int ret = fwrite(cjValue, sizeof(char), strlen(cjValue), file);
    if (ret == 0) 
    {

        fclose(file);
        free(cjValue);
         
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eWriteFileError ;

        std::cout <<  "PluginConfig::sync() write fail. " << std::endl;

        return objNotifyParam ;
    }

    fclose(file);
    free(cjValue);

    return objNotifyParam ;
}


NotifyParam PluginConfig::fillEmptyRoot()
{
    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    this->m_pRootJson = cJSON_CreateObject();
    cJSON_AddStringToObject(this->m_pRootJson ,"pw","111111");
    cJSON_AddStringToObject(this->m_pRootJson ,"enablePW","1");
    cJSON_AddStringToObject(this->m_pRootJson ,"inputTimes","6");
    cJSON_AddNumberToObject(this->m_pRootJson ,"diMcuUnitTemper",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"diMcuLanguage",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"Units",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"diMcuUnitTime",1);
    cJSON_AddNumberToObject(this->m_pRootJson ,"controlbrightMode",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"dashBoardtbrightMode",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"diRadioArea",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"gyro_init_x",0);
    cJSON_AddNumberToObject(this->m_pRootJson ,"gyro_init_y",0); 

    objNotifyParam = this->sync();

    return objNotifyParam ;
}


