
#include "AsyncCall.h"
#include <functional>


AsyncCall::AsyncCall()
    : m_isRun( true ) , 
    m_s32PushCount(0) , 
    m_s32RunCount(0) , 
    m_stLogTag("async-call-tag")

{
    std::thread t(&AsyncCall::delayedExecution,this);
    if(t.joinable())
        t.detach();
}


AsyncCall::~AsyncCall()
{
    this->m_isRun = false ;
    this->m_objCv.notify_one();
}

void AsyncCall::SetLogTag(std::string const & tag)
{
    this->m_stLogTag = "async-call-" + tag;
}

// test delayedExecution  线程执行
void  AsyncCall::delayedExecution()
{   
    while ( this->m_isRun )
    {
        FuncAndParam ft;
        {
        	std::unique_lock<std::mutex> lock( this->m_objCmvtx);
            this->m_objCv.wait(lock,[&]{
                return  this->m_isRun && ! this->m_pFmFuncArr.empty();
                });

            if( ! this->m_isRun )
                break;
		
            ft = this->m_pFmFuncArr.front();
            this->m_pFmFuncArr.pop();
        }
        ++this->m_s32RunCount;
        ft.pFunc(ft);
    }
}

void AsyncCall::AddFunctionToQueue(FuncAndParam const & fp)
{
    ++this->m_s32PushCount;
   {    
        std::lock_guard <std::mutex> lck( this->m_objCmvtx );
        this->m_pFmFuncArr.push(fp);
   }
   this->m_objCv.notify_one();

}