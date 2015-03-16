/**************************************************************************************
@author: �²�
@content: AuthenServer��Ϊ�ͻ��˷�����WebInterfaceServer�������Ķ˿�
**************************************************************************************/
#ifndef __CC_IWEB_CLIENT_SOCKET_H__
#define __CC_IWEB_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* AuthenServer��WebInterfaceServer�����������Ӷ˿�
*
*/

class CIWebClientSocket : public CIOCPClientSocketManager
{
public:
	CIWebClientSocket();
	virtual ~CIWebClientSocket();
	void DoHeartBeat();	
	bool SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
	bool SendToServerPeer(unsigned short usIdent, int iParam, const std::string &str);
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, const char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void LoadConfig();           // ��������
	void SendHeartBeat();        // ��������
private:
	unsigned int m_uiCheckTick;
	int m_iPingCount;       
};

extern CIWebClientSocket* pG_IWebSocket;

#endif //__CC_IWEB_CLIENT_SOCKET_H__