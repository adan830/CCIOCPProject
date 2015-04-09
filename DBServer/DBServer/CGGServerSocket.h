/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameGate的连接
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer监听的单个GameGate的连接对象
*
*/
class CGGConnector : public CClientConnector
{
public:
protected:
private:
private:
};

/**
*
* DBServer对GameGate的监听管理器
*
*/
class CGGServerSocket : public CIOCPServerSocketManager
{
public:
protected:
private:
private:
};

extern CGGServerSocket* pG_GameGateSocket;

#endif //__CC_GAMEGATE_SERVER_SOCKET_H__