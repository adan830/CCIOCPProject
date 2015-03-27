/**************************************************************************************
@author: �²�
@content: GameGate�Կͻ������ӵļ���socket����
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"
#include "CDBClientSocket.h"
#include "CGSClientSocket.h"
#include "CIMClientSocket.h"

typedef struct _TClientActionNode
{
	TActionType ActType;		// ��������
	unsigned char ucCDType;		// 1 ~ 100
	unsigned short usBufLen;	// ���ݳ���
	unsigned int uiCDTime;		// ��Ҫ��CDʱ��
	char* szBuf;				
	_TClientActionNode* pPrevNode;
	_TClientActionNode* pNextNode;
}TClientActionNode, *PClientActionNode;

typedef struct _TSkillCoolDelay
{
	TActionType ActType;
	unsigned char ucCDType;
	unsigned short usSkillID;
	unsigned int uiCDTime;
}TSkillCoolDelay, *PSkillCoolDelay;


typedef std::function<bool(unsigned short usIdent, int iSocketHandle, char* pBuf, unsigned short usBufLen)> TOnSendToServer;

const int MAX_CD_ID = 250;

/**
*
* GameGate�����ĵ���PlayerClient�����Ӷ���
*
*/
class CPlayerClientConnector : public CClientConnector
{
public:
	CPlayerClientConnector();
	virtual ~CPlayerClientConnector();
	void SendToClientPeer(unsigned short usIdent, unsigned int uiIdx, void* pBuf, unsigned short usBufLen);
	void DelayClose(int iReason = -1);
	void OnDisconnect();
	friend class CClientServerSocket;
protected:
	void SendActGood(unsigned short usAct, unsigned short usActParam);       // û����
	void UpdateCDTime(unsigned char bCDType, unsigned int uiCDTime);		 //û���ϸ���CD
	virtual void Execute(unsigned int uiTick);
	virtual void SocketRead(const char* pBuf, int iCount);
private:
	bool CheckGuildWords(char* pBuf, unsigned short usBufLen);
	bool CheckInputWords(char* pBuf, unsigned short usBufLen);
	void ProcessReceiveMsg(char* pHeader, char* pBuf, int iBufLen);
	void ReceiveServerMsg(char* pBuf, unsigned short usBufLen);
	bool CheckServerPkg(unsigned short usIdent, char* pBuf, unsigned short usBufLen);
	void SendMsg(const std::string &sMsg, TMesssageType msgType = msHint, unsigned char ucColor = 255, unsigned char ucBackColor = 255);
	void OpenWindow(TClientWindowType wtype, int iParam, const std::string &sMsg);
	bool NeedQueueCount(unsigned char ucCDType);
	void InitDynCode(unsigned char ucEdIdx = 0);
	TActionType AnalyseIdent(char* pBuf, unsigned short usBufLen, unsigned char &ucCDType, unsigned int &uiCDTime); // �����ͻ��˵�������������
	bool AcceptNextAction();
	void Stiffen(TActionType aType);							// ��ʼһ��Ӳֱʱ��
	void IncStiffen(unsigned int uiIncValue);					// ����һ��Ӳֱʱ��
	void AddToDelayQueue(PClientActionNode pNode);				// ���ӵ��ӳٶ���
	void ProcDelayQueue();										// ������ʱ���н��
	bool IsCoolDelayPass(PClientActionNode pNode);				// ��⶯��cd
	void SCMSkillList(char* pBuf, unsigned short usBufLen);		// ���ռ��ܱ�
	void SCMAddSkill(char* pBuf, unsigned short usBufLen);		// ��������
	void SCMUpdateCDTime(char* pBuf, unsigned short usBufLen);	// ����CD									
private:
	TOnSendToServer m_OnSendToServer;
	unsigned int uiPackageIdx;
	int m_iObjectID;
	std::string m_sRoleName;
	std::string m_sAccount;
	bool m_bTrace;
	bool m_bGM;
	unsigned short m_usLastCMCmd;
	unsigned short m_usLastSCMCmd;
	unsigned int m_uiLastPackageTick;
	unsigned int m_uiCloseTick;
	PClientActionNode m_pFirst;
	PClientActionNode m_pLast;
	int m_iQueueCount;
	unsigned int m_LastCDTicks[MAX_CD_ID];
	CC_UTILS::PCodingFunc m_EnCodeFunc; //���ܺ���
	CC_UTILS::PCodingFunc m_DeCodeFunc; //���ܺ���	
	bool m_bDisconnected;
	bool m_bDeath;
	bool m_bNormalClose;
	int m_iMapID;
	unsigned short m_usHitSpeed;
	unsigned int m_uiLastActionTick;
	unsigned short m_usStiffTime;
	std::vector<PSkillCoolDelay> m_SkillCDTable;
private:	
	/*
	//GPS���
	void GPSCheck();
	m_GPS_Request: Boolean;                                 // �Ѿ�
	m_GPS_Request_Start: Cardinal;
	m_GPS_Action_Count: integer;
	m_CSAuthObject: TCSAuthObject;                          // ����ң�������
	*/
};

/**
*
* GameGate������PlayerClient�ļ���������
*
*/
class CClientServerSocket : public CIOCPServerSocketManager
{
public:
	CClientServerSocket();
	virtual ~CClientServerSocket();
	void SMServerConfig(int iParam, char* pBuf, unsigned short usBufLen);
	void ProcServerMessage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen);
	void ClientManage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen, bool bInGame);
	void GameServerShutDown();
public:
	CGSClientSocket* m_pGameServer;
	CDBClientSocket* m_pDBServer;
	std::string m_sInternetIP;
protected:
	virtual void DoActive();
private:
	/*
	procedure OnClientDisConnect(SockHandle: Word);   ----- �������ö�������
	*/
	void LoadConfig();
	CClientConnector* OnCreateClientSocket(const std::string &sIP);
	void OnClientError(void* Sender, int& iErrorCode);
	void OnListenReady(void* Sender);
	void OnClientConnect(void* Sender);
	void OnClientDisconnect(void* Sender);
	void NotifyNotExistClient(unsigned short usHandle, int iReason);
private:
	bool m_bListenOK;
	CIMClientSocket* m_pIMServer;
	std::string m_sDBAddr;
	int m_iDBPort;
	int m_iLoopCount;
	unsigned int m_uiSlowerTick;
	std::string m_sGSIP;
};

extern CClientServerSocket* pG_ClientServerSocket;

#endif //__CC_CLIENT_SERVER_SOCKET_H__