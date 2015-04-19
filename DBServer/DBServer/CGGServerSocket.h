/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ����������GameGate������
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer�����ĵ���GameGate�����Ӷ���
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
	int m_iServerIdx;
	std::string m_sNetType;
protected:
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen);
private:
	void Msg_Register(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen);
private:
	int m_iOnlineCount;
	bool m_bEnable;
	CC_UTILS::CIntegerHash m_GamePlayerHash;
};

/**
*
* DBServer��GameGate�ļ���������
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
	void SendToClientPeer(unsigned char ucGGIdx, unsigned short usHandle, char* pBuf, unsigned short usBufLen);
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
};

extern CGGServerSocket* pG_GameGateSocket;

#endif //__CC_GAMEGATE_SERVER_SOCKET_H__