
#include "AsyncCall.h"
#include <functional>


AsyncCall::AsyncCall()
    : m_isRun( true )
{
    
    pushCount = 0;
    runCount  = 0; 
    std::thread t(&AsyncCall::delayedExecution,this);
    logtag = "async-call-tag";
    if(t.joinable())
        t.detach();

}


AsyncCall::~AsyncCall()
{
    this->m_isRun = false ;
    cv.notify_one();
}

void AsyncCall::SetLogTag(std::string const & tag)
{
    logtag = "async-call-" + tag;
}

// test delayedExecution  线程执行
void  AsyncCall::delayedExecution()
{   
   
    while ( this->m_isRun )
    {
      //  usleep(100000); // tag 临时修改方案未知原因死锁，改用循环执行;
        FuncAndParam ft;
        {
        	std::unique_lock<std::mutex> lock(cmvtx);
            cv.wait(lock,[&]{
                return  this->m_isRun && ! this->pFmFuncArr.empty();
                });

            if( ! this->m_isRun )
                break;
		
            ft = pFmFuncArr.front();
            pFmFuncArr.pop();
        }
        ++runCount;
        ft.pFunc(ft);
    }
}

void AsyncCall::AddFunctionToQueue(FuncAndParam const & fp)
{
    ++pushCount;
   {    
        std::lock_guard <std::mutex> lck(cmvtx);
        pFmFuncArr.push(fp);
   }
   cv.notify_one();

}