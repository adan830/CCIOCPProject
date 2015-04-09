/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接DispatchGate服务器的端口
**************************************************************************************/
#ifndef __CC_DISPATCHGATE_CLIENT_SOCKET_H__
#define __CC_DISPATCHGATE_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer作为客户端方连接DispatchGate服务器的端口
*
*/

class CDGClientSocket : public CIOCPClientSocketManager
{
public:
protected:
private:
private:
};

extern CDGClientSocket* pG_DispatchGateSocket;

#endif //__CC_DISPATCHGATE_CLIENT_SOCKET_H__