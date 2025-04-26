
#include "AsyncSerialCall.h"
#include <android/log.h>
#include <functional>


AsyncSerialCall::AsyncSerialCall(){
    
    pushCount = 0;
    runCount  = 0; 
    std::thread t(&AsyncSerialCall::delayedExecution,this);
    logtag = "SKY_unset-tag";
    if(t.joinable())
        t.detach();

}
void AsyncSerialCall::SetLogTag(std::string const & tag)
{
    logtag = "SKY_ASC-" + tag;
}
void AsyncSerialCall::mLOGD(const char *fmt, ...)
{
    va_list my_args;
    
    __android_log_print(ANDROID_LOG_DEBUG, logtag.c_str(), fmt, my_args );
}
// test delayedExecution  线程执行
void  AsyncSerialCall::delayedExecution()
{   
    __android_log_print(ANDROID_LOG_DEBUG, logtag.c_str(), "%s AsyncSerialCall thread  start  success ",logtag.c_str());
    while (true)
    {
      //  usleep(100000); // tag 临时修改方案未知原因死锁，改用循环执行;
        FuncAndParam ft;
        {
          //  cmvtx.try_lock
        	std::unique_lock<std::mutex> lock(cmvtx);
            cv.wait(lock,[&]{
               
                __android_log_print(ANDROID_LOG_DEBUG, this->logtag.c_str(), "function cv.wait calling pFmFuncArr size is %d",this->pFmFuncArr.size());
                return !this->pFmFuncArr.empty();
                });
		
            ft = pFmFuncArr.front();
            pFmFuncArr.pop();
        }
        ++runCount;
        __android_log_print(ANDROID_LOG_DEBUG, logtag.c_str(), "will run function [%s] ,runCount is  %d ",ft.tag.c_str(),runCount );
  
        ft.pFunc(ft);
    }
}

void AsyncSerialCall::AddFunctionToQueue(FuncAndParam const & fp)
{

    ++pushCount;
    __android_log_print(ANDROID_LOG_DEBUG, logtag.c_str(), "function FunctionToQueue  push [%s] and send notify. pushCount is %d \n", fp.tag.c_str(),pushCount );
//    mLOGD("AddFunctionToQueue ,p1 = %.1f,p2 = %.1f,p3 =  %.1f,p4 = %.1f \n",fp.p1,fp.p2,fp.p3,fp.p4);
   {    
        std::lock_guard <std::mutex> lck(cmvtx);
        pFmFuncArr.push(fp);
   }

    cv.notify_one();
    

    // mLOGD("FunctionToQueue  push  over and send notify. pushCount is %d \n",pushCount);
    // printf("FunctionToQueue  push  over and send notify. pushCount is %d \n",pushCount);
    
}