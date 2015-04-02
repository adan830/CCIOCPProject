/**************************************************************************************
@author: �²�
@content: ��Ϸ�ĵĳ����ͽṹ����
**************************************************************************************/

#ifndef __CC_GAME_GLOBAL_H__
#define __CC_GAME_GLOBAL_H__

// ͨѶ��Э��ͷ
typedef struct _TServerSocketHeader
{
	unsigned int uiSign;				// �ָ��� SS_SEGMENTATION_SIGN
	int iParam;							// ��չ����
	unsigned short usIdent;				// Э���
	unsigned short usBehindLen;			// �������ݳ���
}TServerSocketHeader, *PServerSocketHeader;

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // �ͻ����������֮��ͨ��Э����ʼ��־

// �ͻ������������Э��
                               

// Ĭ�ϵ������˿�
const int DEFAULT_GameGate_PORT = 8300;                             // GameGate     <- CLIENT

//��־�ȼ�
const int lmtMessage = 0;
const int lmtWarning = 1;
const int lmtError = 2;
const int lmtException = 3;
const int lmtDebug = 255;


#endif //__CC_GAME_GLOBAL_H__