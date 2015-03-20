/**************************************************************************************
@author: 陈昌
@content: GameGate作为客户端方连接GameServer服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CGSClientSocket.h"

using namespace CC_UTILS;

/************************Start Of CGSClientSocket******************************************/

CGSClientSocket::CGSClientSocket() : m_bShutDown(false), m_iPingCount(0), m_uiLastPingTick(0)
{
	m_OnConnect = std::bind(&CGSClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CGSClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CGSClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CGSClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
}

CGSClientSocket::~CGSClientSocket()
{}

bool CGSClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
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

void CGSClientSocket::ConnectToServer(const std::string &sAddr, const int iPort)
{
	m_Address = sAddr;
	m_Port = iPort;
	SetReconnectInterval(10 * 1000);
}

void CGSClientSocket::ClientDisconnect(unsigned short usHandle)
{
	SendToServerPeer(SM_PLAYER_DISCONNECT, usHandle, nullptr, 0);
}

void CGSClientSocket::DoHeartBeat()
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

void CGSClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
	case SM_PLAYER_CONNECT:
	case SM_PLAYER_DISCONNECT:
		//----------------------------
		//----------------------------
		//----------------------------
		//G_ServerSocket.ClientManage(Ident, nParam, Buf, BufLen, True);
		break;
	case SM_SHUTDOWN:
		//----------------------------
		//----------------------------
		//----------------------------
		//G_ServerSocket.GameServerShutDown;
		m_bShutDown = true;
		break;
	default:
		//----------------------------
		//----------------------------
		//----------------------------
		//G_ServerSocket.ProcServerMessage(Ident, nParam, Buf, BufLen);
		break;
	}
}

void CGSClientSocket::OnSocketConnect(void* Sender)
{
	Log("GameServer(" + m_Address + ":" + std::to_string(m_Port) + ") Connected.");
	m_bShutDown = false;
	SendRegisterServer();
	/*
	//-------------------------
	//-------------------------
	//-------------------------
  if Assigned(G_ServerSocket) and Assigned(G_ServerSocket.DBSocket) then
    G_ServerSocket.DBSocket.BoEnable := True;
	*/
}

void CGSClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("GameServer(" + m_Address + ":" + std::to_string(m_Port) + ") DisConnected.");
}

void CGSClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CGSClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CGSClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("GSSocket Error, Code = " + std::to_string(iErrorCode), lmtWarning);
	iErrorCode = 0;
}

void CGSClientSocket::SendRegisterServer()
{
	SendToServerPeer(SM_REGISTER, G_PublicGateIdx, nullptr, 0);
}

/************************End Of CGSClientSocket********************************************/