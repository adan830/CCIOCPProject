/**************************************************************************************
@author: �²�
@content: GameGate��Ϊ�ͻ��˷�����DBServer�������Ķ˿�
**************************************************************************************/
#ifndef __CC_DBSERVER_CLIENT_SOCKET_H__
#define __CC_DBSERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* GameGate��DBServer�����������Ӷ˿�
*
*/

class CDBClientSocket : public CIOCPClientSocketManager
{
public:
	CDBClientSocket();
	virtual ~CDBClientSocket();
	bool SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
	void ConnectToServer(const std::string &sAddr, const int iPort);
	void ClientDisconnect(unsigned short usHandle);
	void DoHeartBeat();					    //���̵߳���
	void SetEnable(bool bFlag);
	bool IsEnable();
	std::string IsIncludeForbiddenWord(std::string &sMsg);
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void SendRegisterServer();              
	void SendHeartBeat(int iConnectCount);
	void AddForbiddenWord(char* pBuf, unsigned short usBufLen, int iCount);
private:
	int m_iPingCount;
	unsigned int m_uiLastPingTick;
	bool m_bEnable;
	std::vector<std::string> m_ForbiddenWords;
};

extern CDBClientSocket* pG_DBServer;

#endif //__CC_DBSERVER_CLIENT_SOCKET_H__