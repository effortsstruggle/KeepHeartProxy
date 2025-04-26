#include "PluginCfg.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <sys/stat.h>
#include <android/log.h>
#include <mutex>
#include <unistd.h> 

#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_CFG", __VA_ARGS__);

std::mutex filemtx;  // 创建一个互斥锁
std::mutex datamtx;  // 创建一个互斥锁
static  cJSON *root = nullptr;

char cfgpath[] = "/data/sky.cfg";
PluginCfg * instance = new PluginCfg ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}

// 线程执行
static void AsyncExecution()
{   
    usleep(1000);

    int tempcount = 0;
   instance->execute(PluginCfg::CFG_INIT,"");
}
 
PluginCfg::PluginCfg(){

    std::thread t(AsyncExecution);
    t.detach();
   
}
PluginCfg::~PluginCfg()
{
    close();
}

int PluginCfg::execute(int key,std::string const &data)
{
    return executeEx(key,data);
}

int PluginCfg::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 ){

    LOGD( "PluginCfg execute key:%d,data:%s,datalen:%d",key,data);

    int val = 0;
    int res = 0;
    
    switch (key)
    {
        case CFG_INIT:
            init(); break;
        case CFG_CLOSE:
            res = close();break;     
        case CFG_SYNC:
            res = sync();  break;
        case CFG_READ:
            res = read(data.c_str());   break;
        case CFG_WRITE:
            res = write(data.c_str());  break;   
        case CFG_RESET:
            res = reset();  break; 
        case CFG_READ_JSON:
            getJson(data.c_str());
            break;
        case CFG_READ_JSON_ALL:
            getAll();
            break;
    default:
        break;
    }
   
    return res;
 };
int PluginCfg::fillEmptyRoot()
{
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root,"pw","111111");
        cJSON_AddStringToObject(root,"enablePW","1");
        cJSON_AddStringToObject(root,"inputTimes","6");
        cJSON_AddNumberToObject(root,"diMcuUnitTemper",0);
        cJSON_AddNumberToObject(root,"diMcuLanguage",0);
        cJSON_AddNumberToObject(root,"Units",0);
        cJSON_AddNumberToObject(root,"diMcuUnitTime",1);
        cJSON_AddNumberToObject(root,"controlbrightMode",0);
        cJSON_AddNumberToObject(root,"dashBoardtbrightMode",0);
        cJSON_AddNumberToObject(root,"diRadioArea",0);
        cJSON_AddNumberToObject(root,"gyro_init_x",0);
        cJSON_AddNumberToObject(root,"gyro_init_y",0);


        // cJSON_AddNumberToObject(root,"",0);
        // cJSON_AddNumberToObject(root,"",0);
        // cJSON_AddNumberToObject(root,"",0);
        // cJSON_AddNumberToObject(root,"",0);
        // cJSON_AddNumberToObject(root,"",0);
        // cJSON_AddNumberToObject(root,"",0);

        
        sync();
}
int  PluginCfg::init()
{
    if(root)
        return -1;
    // 打开文件

    FILE *file = NULL;
    file = fopen(cfgpath, "r");
    if (file == NULL) {
        LOGD("init Open file fail！");
        fillEmptyRoot();
        return 0;
    }


    // 获得文件大小
    struct stat statbuf;
    stat(cfgpath, &statbuf);
    int fileSize = statbuf.st_size;
    LOGD("文件大小：%d", fileSize);


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
  //  LOGD("文件内容%s:", jsonStr);
    fclose(file);

    // 将读取到的json字符串转换成json变量指针
   root = cJSON_Parse(jsonStr);
  //   printf("Error before: [%s]\n", root->);

    if (!root) {
        LOGD("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(jsonStr);
        return -1;
    }
    // cJSON* next = root->child;
    // for(;next;next= next->next)
    // {
    //       LOGD("data %s : %s",next->string, next->valuestring);
    // }
    free(jsonStr);
    return 0;
}
int PluginCfg::close()
{
     if(!root)
        return -1;
    sync();
    cJSON_Delete(root);

    return 0;
}

int PluginCfg::read(const char* str)
{
    if(!root)
        return -1;
    std::lock_guard<std::mutex> lock(datamtx); 

    int res = -1;
    LOGD("PluginCfg::read %s",str);
    cJSON *  item  = cJSON_GetObjectItem(root, str);	
    if (item != NULL) {
        // 判断是不是字符串类型
        if (item->type == cJSON_String) {	
            char * v_str = item->valuestring;		// 此赋值是浅拷贝，不需要现在释放内存
            res = atoi(item->valuestring);
            LOGD("key = %s,value = %d", str,res);
        }
        else
         LOGD("PluginCfg::read unsupport type : %d",item->type);
    }else{
         LOGD("PluginCfg::read cant find tag : %s",str);
    }

    return res;
}

static int counter = 0;
int PluginCfg::write(const char* str)
{
    if(!root)
        return -1;
    std::lock_guard<std::mutex> lock(datamtx); 

    cJSON* data = cJSON_Parse(str);
    if(!data){
         LOGD("write - cJSON_Parse  ERROR. %s  \n",str);    
        return -1;  
    }
    
    int num =  cJSON_GetArraySize(data);

    LOGD("cJSON_GetArraySize :%d ",num);
    cJSON* next = data->child;

    int res = 0;
    char * temptr;
    while(next != NULL)
    {
        LOGD("next->string :%s ",next->string);
        
        cJSON* item = cJSON_DetachItemViaPointer(data,next); // 将next从Data 中移除
        // 判断是否存在此key
        if(cJSON_HasObjectItem(root,item->string)){   
           
            cJSON_DeleteItemFromObject(root,item->string);
        }

        res=  cJSON_AddItemToObject(root,item->string,item);
        if(res == false)
            LOGD("cJSON_AddItemToObject : error  \n");    
       
        next = data->child; // next 已经从data 中移除 ，所以next 需要获取的是新的子结点
    }

    cJSON_Delete(data);
 
    sync();
    return 0;
}

 int PluginCfg::reset()
 {
    cJSON_Delete(root);
    root = NULL;
    if (remove(cfgpath) == 0) {
        LOGD("文件删除成功\n");
    } 

    init();

 }
int PluginCfg::sync()
{
    if(!root)
        return -1;
    std::lock_guard<std::mutex> lock(filemtx);  // 自动锁定
         // 打开文件
    FILE *file  = fopen(cfgpath, "w");
    if (file == NULL) {
        LOGD(" sync Open file fail！\n");
        return -1;
    }

    char *cjValue = cJSON_Print(root);
    if(cjValue == nullptr)
    {
        LOGD("sync root is empty ");
         fclose(file);
         return -1;
    }
       
    // 写入文件
    int ret = fwrite(cjValue, sizeof(char), strlen(cjValue), file);
    if (ret == 0) {
        LOGD("写入文件失败！\n");
        
    }

    fclose(file);
    free(cjValue);
    return 0;
}

int PluginCfg::getJson(const char* str)
{
    std::lock_guard<std::mutex> lock(datamtx); 
    cJSON* item =  cJSON_DetachItemFromObject(root,str);
    if(!item){
        LOGD("getJson - cant find item %s",str);
        return -1;
    }
    LOGD("getJson - str %s,isnull %d",str);
    std::string stdstr = str;
    if(stdstr.size() == 0){
        getAll();
        return 0;
    }
    cJSON * data2 = cJSON_CreateObject();
    
    cJSON_AddItemToObject(data2,str,item);
    char * ptr = cJSON_PrintUnformatted(data2);
    
    Notify(m_id,CFG_READ_JSON,ptr);


    item =  cJSON_DetachItemFromObject(data2,str);

    cJSON_AddItemToObject(root,str,item);

    cJSON_free(ptr) ;
    cJSON_Delete(data2);
}

int PluginCfg::getAll(){
    LOGD("getAll ");
    char * ptr = cJSON_PrintUnformatted(root);

    Notify(CFG_READ_JSON_ALL,ptr);
    cJSON_free(ptr) ;
}   
