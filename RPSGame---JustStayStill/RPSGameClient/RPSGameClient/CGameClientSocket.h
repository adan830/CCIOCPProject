/**************************************************************************************
@author: 陈昌
@content: 客户端连接服务器的端口
**************************************************************************************/
#ifndef __CC_GAME_CLIENT_SOCKET_H__
#define __CC_GAME_CLIENT_SOCKET_H__

#include "stdafx.h"

class CGameClientSocket : public CIOCPClientSocketManager
{
public:
	CGameClientSocket();
	virtual ~CGameClientSocket();
	void SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen);
	void DoHeartBeat();
	void PlayGame();
protected: 
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void ProcessPlayAckMsg(void* pBuf, int iBufLen);
	void LoadConfig();
private:
	unsigned int m_uiCheckTick;
	int m_iPingCount;
};

extern CGameClientSocket* pG_ClientSocket;

#endif //__CC_GAME_CLIENT_SOCKET_H__