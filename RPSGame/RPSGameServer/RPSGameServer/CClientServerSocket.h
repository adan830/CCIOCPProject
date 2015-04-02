/**************************************************************************************
@author: �²�
@content: GameServer�Կͻ������ӵļ���socket����
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* �����ͻ����������������Ӷ���
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
	void CMSelectServer(char* pBuf, unsigned short usBufLen);
	void CMCloseWindow(char* pBuf, unsigned short usBufLen);	
private:
	unsigned int m_uiLastConnectTick;
	unsigned int m_uiForceCloseTick;
};

/**
*
* GameServer�Կͻ��˵ļ���������
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
	std::string m_sWarWarning;							 //����ս����ʾ
};

extern CClientServerSocket* pG_GameSocket;

#endif //__CC_CLIENT_SERVER_SOCKET_H__