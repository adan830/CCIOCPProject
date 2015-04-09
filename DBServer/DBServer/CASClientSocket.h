/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接AuthenServer服务器的端口
**************************************************************************************/
#ifndef __CC_AUTHENSERVER_CLIENT_SOCKET_H__
#define __CC_AUTHENSERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer作为客户端方连接AuthenServer服务器的端口
*
*/

class CASClientSocket : public CIOCPClientSocketManager
{
public:
protected:
private:
private:
};

extern CASClientSocket* pG_AuthenServerSocket;

#endif //__CC_AUTHENSERVER_CLIENT_SOCKET_H__