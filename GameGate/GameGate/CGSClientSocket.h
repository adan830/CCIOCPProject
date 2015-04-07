/**************************************************************************************
@author: 陈昌
@content: GameGate作为客户端方连接GameServer服务器的端口
**************************************************************************************/
#ifndef __CC_GAMESERVER_CLIENT_SOCKET_H__
#define __CC_GAMESERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* GameGate对GameServer服务器的连接端口
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
	void DoHeartBeat();					    //主线程调用
public:
	bool m_bShutDown;						//GS关闭标记
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void SendRegisterServer();              //注册服务器
private:
	int m_iPingCount;
	unsigned int m_uiLastPingTick;
};

extern CGSClientSocket* pG_GameServer;

#endif //__CC_GAMESERVER_CLIENT_SOCKET_H__