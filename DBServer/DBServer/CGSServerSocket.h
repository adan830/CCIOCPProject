/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameServer的连接
**************************************************************************************/
#ifndef __CC_GAMESERVER_SERVER_SOCKET_H__
#define __CC_GAMESERVER_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer监听的单个GameServer的连接对象
*
*/
class CGSConnector : public CClientConnector
{
public:
protected:
private:
private:
};

/**
*
* DBServer对GameServer的监听管理器
*
*/
class CGSServerSocket : public CIOCPServerSocketManager
{
public:
protected:
private:
private:
};

extern CGSServerSocket* pG_GameServerSocket;

#endif //__CC_GAMESERVER_SERVER_SOCKET_H__