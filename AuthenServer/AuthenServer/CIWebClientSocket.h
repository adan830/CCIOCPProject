/**************************************************************************************
@author: 陈昌
@content: AuthenServer作为客户端方连接WebInterfaceServer服务器的端口
**************************************************************************************/
#ifndef __CC_IWEB_CLIENT_SOCKET_H__
#define __CC_IWEB_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* AuthenServer对WebInterfaceServer服务器的连接端口
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
	void LoadConfig();           // 载入配置
	void SendHeartBeat();        // 发送心跳
private:
	unsigned int m_uiCheckTick;
	int m_iPingCount;       
};

extern CIWebClientSocket* pG_IWebSocket;

#endif //__CC_IWEB_CLIENT_SOCKET_H__