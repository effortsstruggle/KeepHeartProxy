#include <unistd.h>
#include "McuParserBase.h"
#include "McuSerial.h"

McuParserBase::McuParserBase() : m_serial(NULL)
{
}

McuParserBase::~McuParserBase()
{
    
}

void McuParserBase::SendData(const void* data, int len) 
{
    if(m_serial != NULL)
        m_serial->SendData(data, len);
}
