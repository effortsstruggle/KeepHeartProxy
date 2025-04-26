#pragma once

class LoopBuffer
{
public:
	LoopBuffer(int nMaxSize);
	~LoopBuffer();

	int GetData(unsigned char* pBuf, int nLength);
	int GetData2(unsigned char* pBuf, int nLength);
    int GetSize() { return m_nLen; }    
	bool AddData(const unsigned char* pData, int nDataLength);
    unsigned char operator [] (int index);
    void Pop(int count = 1);

protected:
	unsigned char*	m_pBuffer;		//循环缓冲区
	int 	        m_nSize;		//缓冲区容量
	int 	        m_nPos;			//数据起始位置
	int 	        m_nLen;			//数据长度
};