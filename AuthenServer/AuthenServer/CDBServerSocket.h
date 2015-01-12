/**************************************************************************************
@author: 陈昌
@content: AuthenServer对DB服务器连接的监听socket管理
**************************************************************************************/
#ifndef __CC_DB_SERVER_SOCKET_H__
#define __CC_DB_SERVER_SOCKET_H__

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
	void SendBuffer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void SendBuffer(unsigned short usIdent, int iParam, const std::string &str);
protected:
	virtual void Execute(unsigned long ulTick);
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	/*
	procedure InitDynCode;
	procedure MsgProcess(wIdent: Word; nParam: integer; PData: PAnsiChar; wBehindLen: Word);
	procedure Msg_Ping(Count: integer);
	procedure Msg_RegisterServer(ServerID: Integer);
	procedure Msg_UserAuthenRequest(Param: integer; Buf: PAnsiChar; BufLen: Word); // step:1
	procedure Msg_NewAccountRequest(Param: integer; Buf: PAnsiChar; BufLen: Word);
	procedure Msg_DBResponse(Ident, Param: integer; Buf: PAnsiChar; BufLen: Word);
	procedure Msg_SafeCardAuthen(Param: integer; Buf: PAnsiChar; BufLen: Word);
	procedure SQLWorkCallBack(Cmd, Param: integer; const str: ansistring);
	procedure OnAuthenFail(SessionID: Integer; nRetCode: Integer; sMsg: AnsiString; AuthType, AuthenApp: Integer);
	*/
private:
	int m_iServerID;                //服务器实际区号
	int m_iHumanCount;              //玩家数量
	bool m_bCheckCredit;			//检测充值
	bool m_bCheckItem;				//检测送道具
	//-----------------------------------
	//-----------------------------------
	//-----------------------------------
	//FEnCode, FDeCode: TCodingProc;
};

/**
*
* AuthenServer对DBServer的监听管理器
*
*/
class CDBServerSocket : public CIOCPServerSocketManager
{
public:
	CDBServerSocket(const std::string& sName);
	virtual ~CDBServerSocket();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	int SelectServer(CDGClient* pClient);
	void SendSelectServer(CDGClient* pClient);
	void SendServerInfoToPig(CPigClientSocket* pPigClient);
	void SendPigMsg(char* pBuf, unsigned short usBufLen);
	int GetPlayerTotalCount();
	void ShowDBMsg(int iServerID, int iCol, const std::string &msg);
	bool RegisterDBServer(const std::string &sAddress, int iServerID, CDBConnector* pDBServer);
protected:
	virtual void DoActive();
private:
	bool OnCheckIPAddress(const std::string& sIP);
	CClientConnector* OnCreateDBSocket(const std::string& sIP);
	void OnSocketError(void* Sender, int& iErrorCode);
	void OnDBConnect(void* Sender);
	void OnDBDisconnect(void* Sender);
	void OnSetListView(void* Sender);

	void LoadServerConfig();
	std::string OnLineDBServer(int iServerID);
	void RemoveServerInfo(void* pValue, const std::string &sKey);
private:
	std::string m_sAllowDBServerIP;				// 允许的IP
	int m_iSessionID;           
	std::string m_sServerName;
	int m_iConfigFileAge;
	unsigned long m_ulLastCheckTick;
	CC_UTILS::CLogSocket* m_pLogSocket;			// 连接日志服务的端口
	CC_UTILS::CStringHash m_ServerHash;         // 区组列表 
};

extern CDBServerSocket* pG_DBSocket;

#endif //__CC_DB_SERVER_SOCKET_H__