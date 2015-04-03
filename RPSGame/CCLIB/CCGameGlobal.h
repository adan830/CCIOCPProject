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

//���������ظ��ͻ��˵�ÿ�ֱȽϺ����Ϣ
typedef struct _TGamePlayAckPkg
{
	int iRoundCount;
	int iClientChoose;
	int iServerChoose;
	int iConclusion;
	int iTotalWins;
	int iTotalLooses;
	int iTotalTies;
}TGamePlayAckPkg, *PGamePlayerAckPkg;

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // �ͻ����������֮��ͨ��Э����ʼ��־

//��Ϸ�ͻ��˷��͸�����������Ϣ
const int CM_PING = 10;										 // ����
const int CM_PLAY_REQ = 11;								     // ������Ϸ���󣬿ͻ��˳�һ��
const int CM_QUIT = 20;		    							 // �����˳�


//�������˷�����Ϸ�ͻ��˵���Ϣ
const int SCM_PING = 100;									 // ����
const int SCM_PLAY_ACK = 11;								 // ��Ϸ��Ӧ�����ؿͻ��˺ͷ��������ģ��Լ���ǰ�����ͳ�ƽ��
const int SCM_QUIT = 101;                                    // �˳�

// �������������˿�
const int DEFAULT_LISTENING_PORT = 8300;                        

//��־�ȼ�
const int lmtMessage = 0;
const int lmtWarning = 1;
const int lmtError = 2;
const int lmtException = 3;
const int lmtDebug = 255;

//��Ϸ����
enum RPS_ENUM
{
	rps_rock,
	rps_paper,
    rps_scissors
};
const std::string G_RPS_STRING[3] = { "Rock", "Paper", "Scissors" };

const std::string G_RPS_CONCLUSION[3] = { "TIE", "WIN", "LOOSE" };


#endif //__CC_GAME_GLOBAL_H__