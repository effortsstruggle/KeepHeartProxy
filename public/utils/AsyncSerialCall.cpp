
#include "AsyncSerialCall.h"
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

// test delayedExecution  线程执行
void  AsyncSerialCall::delayedExecution()
{   
   
    while (true)
    {
      //  usleep(100000); // tag 临时修改方案未知原因死锁，改用循环执行;
        FuncAndParam ft;
        {
          //  cmvtx.try_lock
        	std::unique_lock<std::mutex> lock(cmvtx);
            cv.wait(lock,[&]{
                return !this->pFmFuncArr.empty();
                });
		
            ft = pFmFuncArr.front();
            pFmFuncArr.pop();
        }
        ++runCount;
        ft.pFunc(ft);
    }
}

void AsyncSerialCall::AddFunctionToQueue(FuncAndParam const & fp)
{
    ++pushCount;
   {    
        std::lock_guard <std::mutex> lck(cmvtx);
        pFmFuncArr.push(fp);
   }
   cv.notify_one();

}