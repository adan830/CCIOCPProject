/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameServer的连接
**************************************************************************************/
#ifndef __CC_GAMESERVER_SERVER_SOCKET_H__
#define __CC_GAMESERVER_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer监听的单个GameServer的连接对象
*
*/
class CGSConnector : public CClientConnector
{
public:
	CGSConnector();
	virtual ~CGSConnector();
	void SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	bool IsEnable(){ return m_bEnable; }
public:
	int m_iOnlineCount;
protected:
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen);
private:
	void Msg_Register(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen);
private:
	bool m_bEnable;
};

/**
*
* DBServer对GameServer的监听管理器
*
*/
class CGSServerSocket : public CIOCPServerSocketManager
{
public:
	CGSServerSocket();
	virtual ~CGSServerSocket();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	PServerAddress GetGameServerInfo();
	bool IsGameServerOK();
	void GameServerShutDown();
	void ProcGameServerMessage(PInnerMsgNode pNode);
	bool SendToGameServer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void BroadcastToGS(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	bool CanClosed();
	bool IsOnlineFull(int iAddCount = 0);
	int GetHumanCount();
	CGSConnector* GetActiveGameServer();
public:
    int m_iMaxOnlineCount;

protected:
private:
	bool RegisterGameServer(const std::string &sGSAddr, int iGSPort);
	void Msg_DataRead(int iSessionID, char* pBuf, unsigned short usBufLen);
	void Msg_DataWrite(int iSessionID, char* pBuf, unsigned short usBufLen);
	void Msg_GameActCode(int iSessionID, char* pBuf, unsigned short usBufLen);
	/*
	function CheckConnectIP(const ConnectIP : ansistring) : Boolean;
	procedure SocketError(Sender: TObject; var ErrorCode : integer);
	function CreateCustomSocket(const IP : ansistring) : TCustomClient;
	procedure GSConnect(Sender: TObject);
	procedure GSDisConnect(Sender: TObject);
	procedure ProcGMCmd(SessionID: Integer; CmdStr: AnsiString);
	*/
private:
	std::string m_sAllowIPs;
	TServerAddress m_ServerInfo;
	char* m_pSendCache;
	bool m_bShutDown;
	unsigned int m_uiShutDownTick;
};

extern CGSServerSocket* pG_GameServerSocket;

#endif //__CC_GAMESERVER_SERVER_SOCKET_H__