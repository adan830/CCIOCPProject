/**************************************************************************************
@author: 陈昌
@content: 服务器消息加解密库
**************************************************************************************/
#ifndef __CC_SERVER_ENDECODE_H__
#define __CC_SERVER_ENDECODE_H__

#include <string>

namespace CC_UTILS{
	//加密分隔符
	const int ENCODE_DIVISOR = 0xAA;

	//加解密结构
	typedef struct _TEnDeRecord
	{
		char* pEnBuffer;
		char* pDeBuffer;
		unsigned short usEnBufferLen;
		unsigned short usDeBufferLen;
	}TEnDeRecord, *PEnDeRecord;

	typedef unsigned char BYTE;
	typedef BYTE* PBYTE;

	//加解密函数指针
	typedef void(*PCodingFunc)(PBYTE inBuf, unsigned short len);

	//获取一个加密函数
	PEnDeRecord GetCode(BYTE Idx = 0);

	//...
	std::string MemToHex(PBYTE pb, unsigned short len);

	//加密buffer
	void EnCode(PBYTE pb, unsigned short len);

	//解密buffer
	void DeCode(PBYTE pb, unsigned short len);

	const int MAX_BUFFER_TYPE = 256;
	//保存加解密的buffer
	extern TEnDeRecord CodeBuffer[MAX_BUFFER_TYPE];

	//初始化加解密
	void Initialize_Code();

	//释放加解密的buff
	void Finalize_Code();
}

#endif //__CC_SERVER_ENDECODE_H__