/**************************************************************************************
@author: �²�
@content: GameGate��Ϊ�ͻ��˷�����IMServer�������Ķ˿�
**************************************************************************************/
#ifndef __CC_IMSERVER_CLIENT_SOCKET_H__
#define __CC_IMSERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* GameGate��IMServer�����������Ӷ˿�
*
*/

class CIMClientSocket : public CIOCPClientSocketManager
{
public:
	CIMClientSocket();
	virtual ~CIMClientSocket();
	bool SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
	void ConnectToServer(const std::string &sAddr, const int iPort);
	void ClientDisconnect(unsigned short usHandle);
	void DoHeartBeat();					    //���̵߳���
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

extern CIMClientSocket* pG_IMServer;

#endif //__CC_IMSERVER_CLIENT_SOCKET_H__