#pragma once 

#include <pthread.h>
#include <list>
#include "McuParserBase.h"
#include "VoiceData.h"
#include <map>
#include <iostream>
#include <queue>
#include <string>
#include <mutex>

class AsyncSerialCall;


class VoiceModuleParser : public McuParserBase
{
public:
    VoiceModuleParser();
    virtual ~VoiceModuleParser();


    int SendCmd(int cmd, int data1, long data2);
    void SetNotify(OnVoiceNotifyStr notifier) {m_notifier = notifier;}


protected:


    virtual int ParseData(LoopBuffer& buf);

    void VoiceNotify(int cmd,int data);
    
    OnVoiceNotifyStr m_notifier;

    std::mutex m_mutex;

    AsyncSerialCall * asyncCall;
   
};
