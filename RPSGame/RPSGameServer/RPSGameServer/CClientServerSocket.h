/**************************************************************************************
@author: 陈昌
@content: GameServer对客户端连接的监听socket管理
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* 单个客户端与服务器间的连接对象
*
*/
class CRPSClient : public CClientConnector
{
public:
	CRPSClient();
	virtual ~CRPSClient();
	void ForceClose();
	void SendToClientPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
protected:
	virtual void Execute(unsigned int uiTick);
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen);
private:
	void CMPlayReq(const int iClientChoose);
	int CheckGamePlayConclusion(const int iClientChoose, const int iServerChoose);
private:
	unsigned int m_uiForceCloseTick;
	int m_iCurrentRound;
	int m_iTotalWins;
	int m_iTotalLosses;
	int m_iTotalTies;
};

/**
*
* GameServer对客户端的监听管理器
*
*/
class CClientServerSocket : public CIOCPServerSocketManager
{
public:
	CClientServerSocket();
	virtual ~CClientServerSocket();
	void LoadConfig(CWgtIniFile* pIniFileParser);
private:
	CClientConnector* OnCreateClientSocket(const std::string& sIP);
	void OnSocketError(void* Sender, int& iErrorCode);
	void OnClientConnect(void* Sender);
	void OnClientDisconnect(void* Sender);
private:
	unsigned int m_uiLastCheckTick;
	std::string m_sWarWarning;							 //连接战斗提示
};

extern CClientServerSocket* pG_GameSocket;

#endif //__CC_CLIENT_SERVER_SOCKET_H__