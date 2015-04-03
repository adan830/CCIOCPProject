/**************************************************************************************
@author: 陈昌
@content: 客户端连接服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CGameClientSocket.h"

using namespace CC_UTILS;

CGameClientSocket* pG_ClientSocket;

/************************Start Of CGameClientSocket******************************************/

CGameClientSocket::CGameClientSocket() : m_iPingCount(0)
{
	SendDebugString("CGameClientSocket Create");
	SetReconnectInterval(10 * 1000);
	m_Address = "127.0.0.1";
	m_Port = 8300;

	m_OnConnect = std::bind(&CGameClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CGameClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CGameClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CGameClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
}

CGameClientSocket::~CGameClientSocket()
{
	SendDebugString("CGameClientSocket Destroy");
}

void CGameClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
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

			SendBuf(pData, iDataLen);
			free(pData);
		}
		catch (...)
		{
			SendDebugString("GetLastError " + std::to_string(GetLastError()));
			free(pData);
		}
	}
}

void CGameClientSocket::DoHeartBeat()
{
	unsigned int uiInterval = 10 * 1000;
	if (!IsConnected())
		uiInterval = 3 * 1000;

	unsigned int uiTick = GetTickCount();
	if (uiTick - m_uiCheckTick >= uiInterval)
	{
		m_uiCheckTick = uiTick;
		if (IsConnected())
		{   //连接状态进行心跳检测
			if (m_iPingCount >= 3)
			{
				m_iPingCount = 0;
				Close();
			}
			else
			{
				SendToServerPeer(CM_PING, 0, nullptr, 0);
				m_iPingCount += 1;
			}
		}
		else
		{	//断开状态进行连接
			Log("Connected to RPSGameServer : " + m_Address + ":" + std::to_string(m_Port));
			Open();
		}
	}
}

void CGameClientSocket::PlayGame()
{
	int iRandChoose = rand() % 3;
	SendToServerPeer(CM_PLAY_REQ, iRandChoose, nullptr, 0);
}

void CGameClientSocket::OnSocketConnect(void* Sender)
{	
	Log("Is Connected with RPSGameServer : " + m_Address);
}

void CGameClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("Is DisConnected with RPSGameServer : " + m_Address);
}

void CGameClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	//收到消息,ping计数重置
	m_iPingCount = 0;     
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("RPSGameServer Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CGameClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	switch (pHeader->usIdent)
	{
	case SCM_PING:
		break;
	case SCM_PLAY_ACK:
		ProcessPlayAckMsg(pData, iDataLen);
		break;
	default:
		break;
	}
}

void CGameClientSocket::ProcessPlayAckMsg(void* pBuf, int iBufLen)
{
	if (iBufLen >= sizeof(TGamePlayAckPkg))
	{
		PGamePlayerAckPkg p = (PGamePlayerAckPkg)pBuf;
		Log("Round " + std::to_string(p->iRoundCount) + " : client [" + G_RPS_STRING[p->iClientChoose] + "] -> server [" + G_RPS_STRING[p->iServerChoose] + "] " + G_RPS_CONCLUSION[p->iConclusion]);
		Log("Statistics : client win[" + std::to_string(p->iTotalWins) + "] loose[" + std::to_string(p->iTotalLooses) + "] tie[" + std::to_string(p->iTotalTies) + "]");
	}
}

void CGameClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("RPSGameServer Socket Error, Code = " + std::to_string(iErrorCode), lmtError);
	iErrorCode = 0;
}

/************************End Of CGameClientSocket********************************************/