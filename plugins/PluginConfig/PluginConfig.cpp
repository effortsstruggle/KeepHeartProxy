#include "PluginConfig.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h> 


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
    return this->execute(key,data);
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
    Singleton_PluginConfig::getInstance()->init();
    return 0 ; 
}

int PluginConfig::init()
{
    if( this->m_pRootJson )
        return -1;

    // 打开文件
    FILE *file = NULL;
    file = fopen( this->m_stCfgPath.c_str() , "r");
    if (file == NULL) {
   
        fillEmptyRoot();
        return 0;
    }


    // 获得文件大小
    struct stat statbuf;
    stat( this->m_stCfgPath.c_str() , &statbuf);
    int fileSize = statbuf.st_size;



    // 分配符合文件大小的内存
    char *jsonStr = (char *)malloc(sizeof(char) * fileSize + 1);
    memset(jsonStr, 0, fileSize + 1);

    // 读取文件中的json字符串
    int size = fread(jsonStr, sizeof(char), fileSize, file);
    if (size == 0) {
        fclose(file);
        fillEmptyRoot();
        return -1;
    }
    fclose(file);

    // 将读取到的json字符串转换成json变量指针
    this->m_pRootJson  = cJSON_Parse(jsonStr);

    if (! this->m_pRootJson ) {
       
        free(jsonStr);
        return -1;
    }
  
    free(jsonStr);

    return 0;
}



int PluginConfig::asyncRead( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->read( param.data.c_str());
    return 0 ; 
}

int PluginConfig::read(const char* str)
{
    if(! this->m_pRootJson )
        return -1;

    int res = -1;
   
    cJSON *  item  = cJSON_GetObjectItem( this->m_pRootJson, str);	
    if (item != NULL) {
        // 判断是不是字符串类型
        if (item->type == cJSON_String) 
        {	
            char * v_str = item->valuestring;		// 此赋值是浅拷贝，不需要现在释放内存
            res = atoi(item->valuestring); 
        }
        else
        {


        }
       
    }else{

    }

    return res;
}


int PluginConfig::asyncWrite( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->write( param.data.c_str());
    return 0 ; 
}

int PluginConfig::write(const char* str)
{
    if(! this->m_pRootJson )
        return -1;


    cJSON* data = cJSON_Parse(str);
    if(!data)
    {
        return -1;  
    }
    
    int num =  cJSON_GetArraySize(data);

    cJSON* next = data->child;

    int res = 0;
    char * temptr;
    while(next != NULL)
    {
        
        cJSON* item = cJSON_DetachItemViaPointer(data,next); // 将next从Data 中移除
        // 判断是否存在此key
        if(cJSON_HasObjectItem( this->m_pRootJson , item->string)){   
           
            cJSON_DeleteItemFromObject( this->m_pRootJson , item->string);
        }

        res=  cJSON_AddItemToObject( this->m_pRootJson , item->string,item);
        if(res == false)
        {

        } 
       
        next = data->child; // next 已经从data 中移除 ，所以next 需要获取的是新的子结点
    }

    cJSON_Delete(data);
 
    this->sync();

    return 0;
}


int PluginConfig::asyncReset( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->reset();
    return 0 ; 
}

int PluginConfig::reset()
{
    if(! this->m_pRootJson )
        return -1;

    cJSON_Delete( this->m_pRootJson );
    this->m_pRootJson = NULL;

    if (remove( this->m_stCfgPath.c_str() ) == 0)
    {

    } 

    return this->init();

}


int PluginConfig::asyncClose( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->close();
    return 0 ; 
}

int PluginConfig::close()
{
     if(! this->m_pRootJson )
        return -1;

    this->sync();

    cJSON_Delete( this->m_pRootJson );

    return 0;
}


int PluginConfig::asyncGetJson( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->getJson( param.data.c_str() );
    return 0 ; 
}

int PluginConfig::getJson(const char* str)
{
    if(! this->m_pRootJson )
    return -1;

    cJSON* item =  cJSON_DetachItemFromObject( this->m_pRootJson , str);
    if(!item)
    {   
        return -1;
    }
    
    std::string stdstr = str;
    if(stdstr.size() == 0)
    {
        getAll();
        return 0;
    }

    cJSON * data2 = cJSON_CreateObject();

    cJSON_AddItemToObject(data2,str,item);
    char * ptr = cJSON_PrintUnformatted(data2);
    
   
    item =  cJSON_DetachItemFromObject(data2,str);

    cJSON_AddItemToObject( this->m_pRootJson , str , item);

    cJSON_free(ptr) ;
    cJSON_Delete(data2);

    return 0;
}


int PluginConfig::asyncGetAll( FuncAndParam const &param )
{
    Singleton_PluginConfig::getInstance()->getAll();
    //Notify
    return 0 ; 
}

int PluginConfig::getAll(){
    
    if(! this->m_pRootJson )
        return -1;

    char * ptr = cJSON_PrintUnformatted( this->m_pRootJson );

    cJSON_free(ptr) ;

    return 0 ;
}   



int PluginConfig::sync()
{
         // 打开文件
    FILE *file  = fopen( this->m_stCfgPath.c_str() , "w");
    if (file == NULL) 
    {
        return -1;
    }

    char *cjValue = cJSON_Print(  this->m_pRootJson );
    if(cjValue == nullptr)
    {
         fclose(file);
         return -1;
    }
       
    // 写入文件
    int ret = fwrite(cjValue, sizeof(char), strlen(cjValue), file);
    if (ret == 0) 
    {
       
        
    }

    fclose(file);
    free(cjValue);
    
    return 0;
}


int PluginConfig::fillEmptyRoot()
{
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
        return this->sync();
}
