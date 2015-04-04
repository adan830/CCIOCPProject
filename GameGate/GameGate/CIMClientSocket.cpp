/**************************************************************************************
@author: 陈昌
@content: GameGate作为客户端方连接IMServer服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CIMClientSocket.h"
#include "CClientServerSocket.h"

using namespace CC_UTILS;

/************************Start Of CIMClientSocket******************************************/

CIMClientSocket::CIMClientSocket() : m_iPingCount(0), m_uiLastPingTick(_ExGetTickCount)
{
	m_OnConnect = std::bind(&CIMClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CIMClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CIMClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CIMClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
}

CIMClientSocket::~CIMClientSocket()
{}

bool CIMClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
{
	bool bRetFlag = false;
	if (IsConnected())
	{
		int iDataLen = sizeof(TServerSocketHeader)+usBufLen;
		char* pData = (char*)malloc(iDataLen);
		if (pData != nullptr)
		{
			try
			{
				((PServerSocketHeader)pData)->uiSign = SS_SEGMENTATION_SIGN;
				((PServerSocketHeader)pData)->usIdent = usIdent;
				((PServerSocketHeader)pData)->iParam = iParam;
				((PServerSocketHeader)pData)->usBehindLen = usBufLen;
				if (usBufLen > 0)
					memcpy(pData + sizeof(TServerSocketHeader), pBuf, usBufLen);

				bRetFlag = (SendBuf(pData, iDataLen) == iDataLen);
				free(pData);
			}
			catch (...)
			{
				free(pData);
			}
		}
	}
	return bRetFlag;
}

void CIMClientSocket::ConnectToServer(const std::string &sAddr, const int iPort)
{
	m_Address = sAddr;
	m_Port = iPort;
	SetReconnectInterval(10 * 1000);
	m_uiLastPingTick = _ExGetTickCount;
}

void CIMClientSocket::ClientDisconnect(unsigned short usHandle)
{
	SendToServerPeer(SM_PLAYER_DISCONNECT, usHandle, nullptr, 0);
}

void CIMClientSocket::DoHeartBeat()
{
	if (_ExGetTickCount - m_uiLastPingTick >= 10000)
	{
		m_uiLastPingTick = _ExGetTickCount;
		if (IsConnected())
		{
			if (m_iPingCount > 3)
			{
				m_iPingCount = 0;
				Close();
			}
			else
			{
				SendToServerPeer(SM_PING, 0, nullptr, 0);
			}
		}
	}
}

void CIMClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
		break;
	default:
		if (pG_ClientServerSocket != nullptr)
			pG_ClientServerSocket->ProcServerMessage(pHeader->usIdent, pHeader->iParam, pData, iDataLen);
		break;
	}
}

void CIMClientSocket::OnSocketConnect(void* Sender)
{
	Log("IMServer(" + m_Address + ":" + std::to_string(m_Port) + ") Connected.");
	SendRegisterServer();
}

void CIMClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("IMServer(" + m_Address + ":" + std::to_string(m_Port) + ") DisConnected.");
}

void CIMClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CGSClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CIMClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("IMServer Error, Code = " + std::to_string(iErrorCode), lmtWarning);
	iErrorCode = 0;
}

void CIMClientSocket::SendRegisterServer()
{
	SendToServerPeer(SM_REGISTER, G_PublicGateIdx, nullptr, 0);
}

/************************End Of CIMClientSocket********************************************/