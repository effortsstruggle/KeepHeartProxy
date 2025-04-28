/**
 *@brief: 一个简单的函数异步串行执行类 
 *@author: sky
 *@since 
 *@date: 2025-02-06 19:36:47
*/



#pragma once 
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include <queue>
#include <condition_variable>

struct FuncAndParam;
typedef int (*PFUNC)(FuncAndParam const &);

struct NotifyParam
{
    int m_s32Ret ; // -1 : error , 0 : correct 
    std::string m_stCorrectInfo ; //correct info
    std::string m_stErrorInfo; //error info
};



// 函数和参数结构体
struct FuncAndParam
{
    
    FuncAndParam(PFUNC fc = nullptr, std::string data = "" , double d1=0,double d2=0,double d3=0,double d4=0,std::string ptag = "null"):
    pFunc(fc),data(data),p1(d1),p2(d2),p3(d3),p4(d4),tag(ptag){ 
       };


    PFUNC pFunc;
    std::string data ; 
    double p1;    // 每个函数的参数
    double p2;    // 在这里用作延迟时间
    double p3;    // 在这里用作延迟时间
    double p4;    // 在这里用作延迟时间
    std::string tag; // 打印输出使用
};
// 简单的线程函数执下;有新的需要执行的函数就添加到队列，队列中负责执行;


class AsyncCall {
public:
    AsyncCall();
    ~AsyncCall();
    void SetLogTag(std::string const&);
// test 函数添加
    void AddFunctionToQueue(FuncAndParam const & fp);
    
private:
    void delayedExecution(); 
private:
    std::string m_stLogTag;

    std::mutex m_objCmvtx;  // mutex
    std::condition_variable m_objCv; //condition variable
    std::queue<FuncAndParam>  m_pFmFuncArr ; //函数数组
    
    int m_s32PushCount; // 记录推入多少函数
    int m_s32RunCount ; // 记录执行了多少函数

    bool m_isRun ;
};

