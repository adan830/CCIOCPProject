/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ����������GameGate������
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer�����ĵ���GameGate�����Ӷ���
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
* DBServer��GameGate�ļ���������
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