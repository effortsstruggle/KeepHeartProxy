
#ifndef __SOCKETHELPER__
#define __SOCKETHELPER__
#include <iostream>


struct MsgHeader
{
	MsgHeader()
	{
		version = 1;
	}
	//消息版本
	int version;

	// 消息类型
	int msgType;

	//   附加信息
	int addtional;

	//有效数据长度
	int dataLen;  //有时没有data 也作为参数消息来使用

	//// 数据,保持这个结果在最后
	char data[];
	void msgPrint(std::string tag){
	std::cout << tag << "msg: \n";
    std::cout << "msgType:"<< msgType <<std::endl;
    std::cout << "adtional:"<< addtional <<std::endl;
    std::cout << "dataLen:"<< dataLen <<std::endl;
    std::cout << "data:"<< data <<std::endl;
	}
};

#define HEADERSIZE sizeof(MsgHeader)
//一个消息缓冲区中可用的数据大小
#define VALIDSIZE 1460- sizeof(MsgHeader)

// msg 消息，data 需要发送的字符串,长度在msg的datalen中
// destbuf 目标存储数组，无需指导尺寸外部指定
int MsgConversion(MsgHeader* msg,char* data,char* destbuf);



//消息状态
enum MsgAddtional
{
	MD_NONE, //无附加消息
	
	MD_FILE_NAME,//数据为文件名
	MD_FILE_DATA,//数据为一个文件的全部数据
	MD_FILES_FISHED,//发送文件操作完成 ，是多个文件

	DATA_NEW,    //新数据
	DATA_FISHED, //数据完成，及本信息为最后一条 
	DATA_SERRISE,// 连续数据
	MD_DATA_TEST_INFO,  //试验信息
	MD_DATA_IPS,        //发送的是IP
	MD_DATA_NORMAL,     // 普通数据
	RECV_OVER,   //接收完成，一般用于接收法的返回值
	RECV_FAILED, //数据接收失败
};


#endif