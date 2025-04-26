#pragma once 

#include <pthread.h>
#include <list>
#include "McuParserBase.h"
#include "McuDef.h"
#include <map>
#include <iostream>
#include <queue>
#include <string>
struct NotifyItem {
    int type ;      //类型, 1 整形; 2 字符
    MCU_NOTIFY code;
    int data1;
    long data2;
    std::string str;
};
class LoopBuffer;
using namespace std;

class McuParser : public McuParserBase
{
public:
    McuParser();
    virtual ~McuParser();

    int GetInt(MCU_INDEX index);
    int SendCmd(MCU_COMMAND cmd, int data1, long data2);
    void SetNotify(OnMcuNotify notifier) {m_notifier = notifier;}
    void SetNotify(OnMcuNotifyStr notifier) {m_notifierStr = notifier;}

    struct keyEvent{ //按键消息结构体
        time_t keyDown;// 记录按下的时间，接收到信号时
        int sendTimes;    // 已发送次数 与时间的倍数 相关
    };

protected:

    struct errCodeRef{ //类似于smartptr 的作用。无引用时删除
    int errCode;  //错误码
    int times;    //出现的次数
    time_t adddTime;// 添加的时间
    };



    virtual int ParseData(LoopBuffer& buf);
    void ProcessBtKeyMsg();

    void AddToNotify(MCU_NOTIFY code, int data1, long data2 = 0);
    void AddToNotifyStr(MCU_NOTIFY code, int data1, char* data2);
    static void* ThreadProc(void* arg);
    //错误码处理，30s出现则发送给qml 消息
    void ThreadErrCodeCheck();
    /**
     *@description: 处理按键事件将案件处理点按
     *@author: sky
     *@return 
     *@date: 2024-10-31 14:22:45
    */
    void KeyChange(uint8_t  key_state);
    void KeyEventProc( time_t timep);
    
     /**
     *@description: 线程处理按键事件将案件处理长按
     *@author: sky
     *@return 
     *@date: 2024-10-31 14:22:45
    */
    void ThreadKeyHold();
    void Proc();
    void AddErrCode(int code);
  
    bool m_running;
    list<NotifyItem*> m_notifyItems;

    std::map<int,errCodeRef*> errCodeMap;  // 方便查找已添加的数据
   // std::queue<errCodeRef*> errCodequeue;  // 减少迭代循环量
    
    OnMcuNotify m_notifier;
    OnMcuNotifyStr  m_notifierStr;
    pthread_mutex_t	m_mutex;
    pthread_mutex_t	m_mutex2;

   
};
