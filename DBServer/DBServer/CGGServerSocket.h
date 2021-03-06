/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameGate的连接
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer监听的单个GameGate的连接对象
*
*/
class CGGConnector : public CClientConnector
{
public:
	CGGConnector();
	virtual ~CGGConnector();
	void SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	int GetOnlineCount(){ return m_iOnlineCount; }
	bool IsEnable(){ return m_bEnable; }
public:
	int m_iOnlineCount;
	int m_iServerIdx;
	std::string m_sNetType;
protected:
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen);
private:
	void Msg_Register(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen);
private:
	bool m_bEnable;
	CC_UTILS::CIntegerHash m_GamePlayerHash;
};

/**
*
* DBServer对GameGate的监听管理器
*
*/
class CGGServerSocket : public CIOCPServerSocketManager
{
public:
	CGGServerSocket();
	virtual ~CGGServerSocket();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	void GetComfyGate(int &iAddr, int &iPort, unsigned char ucNetType);
	void ProcGameGateMessage(PInnerMsgNode pNode);
	void KickOutClient(unsigned char ucIdx, unsigned short usHandle, int iReason);
	void AddOnlineCount(unsigned char ucGGIdx, int iCount=1);
	void SendToGameGate(unsigned char ucGGIdx, unsigned short usHandle, char* pBuf, unsigned short usBufLen);
	void SetGameGateNet(unsigned char ucGGIdx, const std::string &sNetType);
	void ResendFilterWords();
	std::string GetAllowIPs();
private:
	bool OnCheckConnectIP(const std::string &sConnectIP);
	void OnSocketError(void* Sender, int& iErrorCode);
	CClientConnector* OnCreateGGSocket(const std::string &sIP);
	void OnGGConnect(void* Sender);
	void OnGGDisconnect(void* Sender);
	bool RegisterGameGate(CGGConnector* Sender, const std::string &sAddr, int iPort);
	void SMPlayerConnect(PInnerMsgNode pNode);
private:
	std::string m_sAllowIPs;
	TServerAddressEx m_ServerArray[MAX_GAMEGATE_COUNT];

	friend class CGGConnector;
};

extern CGGServerSocket* pG_GameGateSocket;

#endif //__CC_GAMEGATE_SERVER_SOCKET_H__