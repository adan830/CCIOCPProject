/**************************************************************************************
@author: �²�
@content: ��������Ϣ�ӽ��ܿ�
**************************************************************************************/
#ifndef __CC_SERVER_ENDECODE_H__
#define __CC_SERVER_ENDECODE_H__

#include <string>

namespace CC_UTILS{
	//���ָܷ���
	const int ENCODE_DIVISOR = 0xAA;

	//�ӽ��ܽṹ
	typedef struct _TEnDeRecord
	{
		char* pEnBuffer;
		char* pDeBuffer;
		unsigned short usEnBufferLen;
		unsigned short usDeBufferLen;
	}TEnDeRecord, *PEnDeRecord;

	typedef unsigned char BYTE;
	typedef BYTE* PBYTE;

	//�ӽ��ܺ���ָ��
	typedef void(*PCodingFunc)(PBYTE inBuf, unsigned short len);

	//��ȡһ�����ܺ���
	PEnDeRecord GetCode(BYTE Idx = 0);

	//...
	std::string MemToHex(PBYTE pb, unsigned short len);

	//����buffer
	void EnCode(PBYTE pb, unsigned short len);

	//����buffer
	void DeCode(PBYTE pb, unsigned short len);

	const int MAX_BUFFER_TYPE = 256;
	//����ӽ��ܵ�buffer
	extern TEnDeRecord CodeBuffer[MAX_BUFFER_TYPE];

	//��ʼ���ӽ���
	void Initialize_Code();

	//�ͷżӽ��ܵ�buff
	void Finalize_Code();
}

#endif //__CC_SERVER_ENDECODE_H__