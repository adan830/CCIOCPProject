/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"
#include "CDBClientSocket.h"
#include "CGSClientSocket.h"
#include "CIMClientSocket.h"

typedef struct _TClientActionNode
{
	TActionType ActType;		// 动作类型
	unsigned char ucCDType;		// 1 ~ 100
	unsigned short usBufLen;	// 数据长度
	unsigned int uiCDTime;		// 需要的CD时间
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
* GameGate监听的单个PlayerClient的连接对象
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
	void SendActGood(unsigned short usAct, unsigned short usActParam);       // 没用上
	void UpdateCDTime(unsigned char bCDType, unsigned int uiCDTime);		 //没用上更新CD
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
	TActionType AnalyseIdent(char* pBuf, unsigned short usBufLen, unsigned char &ucCDType, unsigned int &uiCDTime); // 分析客户端到服务器的数据
	bool AcceptNextAction();
	void Stiffen(TActionType aType);							// 开始一个硬直时间
	void IncStiffen(unsigned int uiIncValue);					// 增加一个硬直时间
	void AddToDelayQueue(PClientActionNode pNode);				// 增加到延迟队列
	void ProcDelayQueue();										// 处理延时队列结点
	bool IsCoolDelayPass(PClientActionNode pNode);				// 检测动作cd
	void SCMSkillList(char* pBuf, unsigned short usBufLen);		// 接收技能表
	void SCMAddSkill(char* pBuf, unsigned short usBufLen);		// 新增技能
	void SCMUpdateCDTime(char* pBuf, unsigned short usBufLen);	// 更新CD									
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
	CC_UTILS::PCodingFunc m_EnCodeFunc; //加密函数
	CC_UTILS::PCodingFunc m_DeCodeFunc; //解密函数	
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
	//GPS相关
	void GPSCheck();
	m_GPS_Request: Boolean;                                 // 已经
	m_GPS_Request_Start: Cardinal;
	m_GPS_Action_Count: integer;
	m_CSAuthObject: TCSAuthObject;                          // 反外挂，检测对象
	*/
};

/**
*
* GameGate对所有PlayerClient的监听管理器
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
	procedure OnClientDisConnect(SockHandle: Word);   ----- 函数不用独立出来
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