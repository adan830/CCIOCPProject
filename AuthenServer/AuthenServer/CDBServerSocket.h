/**************************************************************************************
@author: 陈昌
@content: AuthenServer对DB服务器连接的监听socket管理
**************************************************************************************/
#ifndef __CC_DB_SERVER_SOCKET_H__
#define __CC_DB_SERVER_SOCKET_H__

#include "stdafx.h"
#include "CCTcpServerSocket.h"

//dispatch上使用的服务器区组配置
typedef struct _TServerConfigInfo
{
	int iMaskServerID;
	std::string sServerName;
	int iRealServerID;
	std::string sServerIP;
	bool bDenyRecharge;
	bool bDenyGiveItem;
}TServerConfigInfo, *PServerConfigInfo;

/**
*
* AuthenServer监听的单个DBServer的连接对象
*
*/
class CDBConnector : public CClientConnector
{
public:
	CDBConnector();
	virtual ~CDBConnector();
	int GetServerID();
	int GetHumanCount();
	void SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void SendToClientPeer(unsigned short usIdent, int iParam, const std::string &str);
protected:
	virtual void Execute(unsigned long ulTick);
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void InitDynCode();
	void Msg_Ping(int iCount);
	void Msg_RegisterServer(int iServerID);
	void Msg_UserAuthenRequest(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_NewAccountRequest(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_DBResponse(int iIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_SafeCardAuthen(int iParam, char* pBuf, unsigned short usBufLen);
	void SQLWorkCallBack(int iCmd, int iParam, const std::string &str);
	void OnAuthenFail(int iSessionID, int iRetCode, const std::string &sMsg, int iAuthType, int iAuthenApp);
private:
	int m_iServerID;					//服务器实际区号
	int m_iHumanCount;					//玩家数量
	bool m_bCheckCredit;				//检测充值
	bool m_bCheckItem;					//检测送道具
	CC_UTILS::PCodingFunc m_EnCodeFunc; //加密函数
	CC_UTILS::PCodingFunc m_DeCodeFunc; //解密函数
};

/**
*
* AuthenServer对DBServer的监听管理器
*
*/
class CDBServerSocket : public CIOCPServerSocketManager
{
public:
	CDBServerSocket(const std::string &sServerName);
	virtual ~CDBServerSocket();
	/*
	//merge in
	procedure OnCreate; override;
	procedure OnDestroy; override;
	*/
	void SQLJobResponse(int iCmd, int iHandle, int iParam, int iRes, const std::string &str);
	void InCreditNow();
	void InSendItemNow();
	void BroadCastKickOutNow(const std::string &sAccount, int iParam);
protected:
	void DoActive();
private:
	bool OnChildNotify(int iServerID, PGameChildInfo p);
	void ProcResponseMsg();
	void Clear();
	void LoadServerConfig();
	void OnSetListView(void* Sender);
	void OnLogSocketDisConnect(void* Sender);
	void AddRechargeQueryJob(int iServerID, int iSocketHandle);
	void AddQueryGiveItemJob(int iServerID, int iSocketHandle);
	void RemoveServerConfig(void* pValue, const std::string &sKey);
	bool CheckConnectIP(const std::string &sConnectIP);
	void LoadConfig();
	void SocketError(void* Sender, int iErrorCode);
	CClientConnector CreateCustomSocket(const std::string &sIP);
	void DBConnect(void* Sender);
	void DBDisConnect(void* Sender);
	bool RegisterDBServer(CDBConnector* Socket, int iServerID);
	std::string OnLineDBServer(int iServerID);
	void ShowDBMsg(int iServerID, int iCol, const std::string &sMsg);
	void RechargeFail(const std::string &sOrderID);
private:
	std::string m_sServerName;
	CC_UTILS::CStringHash m_ServerHash;
	//--------------std::vector<> m_ServerIDList;
	unsigned long m_ulLastQueryRechargeTick, m_ulQueryRechargeInterval;
	unsigned long m_ulLastQueryItemTick, m_ulQueryItemInterval;
	unsigned long m_ulLastCheckTick;
	int m_iConfigFileAge;
	int m_iGameID;
	std::string m_sAllowDBServerIPs;
	CC_UTILS::CLogSocket* m_pLogSocket;			// 连接日志服务的端口
	/*
	FCS: TRTLCriticalSection;
	FFirst, FLast: PJSONJobNode;
	*/
	friend class CDBConnector;
};

extern CDBServerSocket* pG_DBSocket;

#endif //__CC_DB_SERVER_SOCKET_H__