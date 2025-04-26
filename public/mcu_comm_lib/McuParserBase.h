#pragma once 

class McuSerial;
class LoopBuffer;
class McuParserBase
{
public:
    McuParserBase();
    virtual ~McuParserBase();

protected:
    // 原始数据，返回解析的数据长度
    virtual int ParseData(LoopBuffer&) = 0;
    void SendData(const void* data, int len);

    McuSerial*      m_serial;

    friend class McuSerial;
};
