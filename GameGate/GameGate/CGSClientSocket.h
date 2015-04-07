/**************************************************************************************
@author: �²�
@content: GameGate��Ϊ�ͻ��˷�����GameServer�������Ķ˿�
**************************************************************************************/
#ifndef __CC_GAMESERVER_CLIENT_SOCKET_H__
#define __CC_GAMESERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* GameGate��GameServer�����������Ӷ˿�
*
*/

class CGSClientSocket : public CIOCPClientSocketManager
{
public:
	CGSClientSocket();
	virtual ~CGSClientSocket();
	bool SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
	void ConnectToServer(const std::string &sAddr, const int iPort);
	void ClientDisconnect(unsigned short usHandle);
	void DoHeartBeat();					    //���̵߳���
public:
	bool m_bShutDown;						//GS�رձ��
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void SendRegisterServer();              //ע�������
private:
	int m_iPingCount;
	unsigned int m_uiLastPingTick;
};

extern CGSClientSocket* pG_GameServer;

#endif //__CC_GAMESERVER_CLIENT_SOCKET_H__