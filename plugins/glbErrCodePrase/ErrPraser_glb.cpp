#include "ErrPraser_glb.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <sys/stat.h>
#include <android/log.h>
#include <stdio.h>
#include <AsyncSerialCall.h>
#include <csv.h>
#include <map>

#define __out(...) \
    __android_log_print(ANDROID_LOG_DEBUG, "SKY_ERR_INFO", __VA_ARGS__);
 
#define __format(__fmt__) "(%d)-<%s>: " __fmt__ "\n"
 
#define LOGD(__fmt__, ...) \
    __out(__format(__fmt__),  __LINE__, __FUNCTION__, ##__VA_ARGS__);



static std::string larr[ErrPraser_glb::L_MAX];
static  ErrPraser_glb * instance = new ErrPraser_glb ;

extern "C" PluginInterface * createPlugin() {

    return instance;
}

int async_readFile(FuncAndParam const & fp){

    std::string tag;
    switch ((int)fp.p1)
    {
    case ErrPraser_glb::L_ENGLIASH:
        tag = "english";
        break;
    case ErrPraser_glb::L_CHINESE:
        tag = "chinese";
        break;
    
    default:
        break;
    }

    if((int)fp.p1 == ErrPraser_glb::L_ENGLIASH){
        std::string path = "/data/languages/" + tag+ ".csv";
        instance->ReadFile(path,instance->err_en);
    }
    else{
        instance->err_other.clear();
        std::string path = "/data/languages/" + tag + ".csv";
        instance->ReadFile(path,instance->err_other);
    }
}


int getErr(FuncAndParam const & fp){

    LOGD("fp.data %s",fp.tag.c_str())
    ErrPraser_glb::ErrInfo * errinfo = nullptr;
    
    if(instance->err_other.size() != 0)
    {
        auto iter =  instance->err_other.find(fp.tag);
        if(iter != instance->err_other.end())
            errinfo =  &iter->second;
    }
    else if(instance->err_en.size())
    {
        auto iter =  instance->err_en.find(fp.tag);
        if(iter != instance->err_en.end())
            errinfo =  &iter->second;

        LOGD("errinfo addr: %d ,err_en addr :%d",errinfo, &instance->err_en[fp.tag])
    }
    
    cJSON * data2 = cJSON_CreateObject();
    
    if(errinfo){
        cJSON_AddStringToObject(data2,"code",errinfo->code.c_str());
        cJSON_AddStringToObject(data2,"name",errinfo->name.empty()? "unknow":errinfo->name.c_str());
        cJSON_AddStringToObject(data2,"described",errinfo->described.empty()?"unknow":errinfo->described.c_str());
    }
    else{
        cJSON_AddStringToObject(data2,"code",fp.tag.c_str());
        cJSON_AddStringToObject(data2,"name","unknow");
        cJSON_AddStringToObject(data2,"described","unknow");
    }
    
    char * ptr = cJSON_PrintUnformatted(data2);
    // LOGD("NT_ERR_INFO %s",ptr)
    instance->Notify(ErrPraser_glb::NT_ERR_INFO,ptr);
    cJSON_free(ptr) ;
    cJSON_Delete(data2);

}
ErrPraser_glb::ErrPraser_glb(){
    asyncCall = new AsyncSerialCall;
    LOGD("load glb err ");
    cur_language = L_MAX;
    asyncCall->SetLogTag("ErrPraser");

    larr[L_ENGLIASH] = "english";
    
    asyncCall->AddFunctionToQueue(FuncAndParam(async_readFile,L_ENGLIASH,0,0,0,"english"));
}
ErrPraser_glb::~ErrPraser_glb()
{
    delete asyncCall;
}
int ErrPraser_glb::execute(int key,std::string const &data){

    return executeEx(key,data);
}

int ErrPraser_glb::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{

    // LOGD("error data %s",data.c_str());
    switch (key)
    {
        case SET_LANGUEAGE:{
            asyncCall->AddFunctionToQueue(FuncAndParam(async_readFile,L_ENGLIASH,0,0,0,data));
            cur_language = (LANGUAGE_DEF)p1;
            break;
        }
        case GET_ERR_INFO:
        {

            asyncCall->AddFunctionToQueue(FuncAndParam(getErr,L_ENGLIASH,0,0,0,data));
            break;
        }
    default:
        break;
    }
    
    return 1;
}
int ErrPraser_glb::ReadFile(std::string path,std::map<std::string,ErrInfo> & err_arr)
{
    LOGD("path %s",path.c_str());
    try {
        io::CSVReader<3,io::ignore_extra_column|io::ignore_missing_column,
                    io::trim_chars<' ', '\t'>,
                    io::double_quote_escape<',','\"'>,
                    io::throw_on_overflow,io::empty_line_comment> in(path);

        in.read_header( "code", "name", "described");
        std::string code,  name, described;
        name =described =  "empty";
        while(in.read_row(code, name, described)){


            err_arr.emplace(code,ErrInfo{code,name,described});
        }
        }catch (const std::exception& e) {
           LOGD("ERROR Caught exception: %s", e.what());
        }   
}