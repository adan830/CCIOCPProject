/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ����������GameServer������
**************************************************************************************/
#ifndef __CC_GAMESERVER_SERVER_SOCKET_H__
#define __CC_GAMESERVER_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer�����ĵ���GameServer�����Ӷ���
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
* DBServer��GameServer�ļ���������
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