/**************************************************************************************
@author: 陈昌
@content: GameServer对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"

using namespace CC_UTILS;

const int MAX_CONNECT_TIMEOUT = 30 * 1000;                // 最长的连接时间
const int DELAY_DISCONNECT_TIME = 3000;                   // 延时断开时间

CClientServerSocket* pG_GameSocket;

/************************Start Of CRPSClient********************************************************/
CRPSClient::CRPSClient() :m_uiLastConnectTick(GetTickCount()), m_uiForceCloseTick(0)
{
	SendDebugString("CRPSClient Create");
}

CRPSClient::~CRPSClient()
{
	SendDebugString("CRPSClient Destroy");
}

void CRPSClient::ForceClose()
{
	m_uiForceCloseTick = GetTickCount();
}

void CRPSClient::Execute(unsigned int uiTick)
{
	CClientConnector::Execute(uiTick);
	if (((uiTick > m_uiLastConnectTick) && (uiTick - m_uiLastConnectTick > MAX_CONNECT_TIMEOUT)) ||
		((m_uiForceCloseTick > 0) && (uiTick > m_uiForceCloseTick) && (uiTick - m_uiForceCloseTick > DELAY_DISCONNECT_TIME)))
	{
		Close();
	}

	if (uiTick < m_uiLastConnectTick)
		m_uiLastConnectTick = uiTick;
}

void CRPSClient::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
{
	switch (((PServerSocketHeader)pHeader)->usIdent)
	{
		/*
	case CM_PING:
		SendToClientPeer(CM_PING, 0, nullptr, 0);
		break;
	case CM_QUIT:
		ForceClose();
		break;
		*/
	default:
		Log("received unknown client msg，IP=" + GetRemoteAddress() + " Ident=" + std::to_string(((PServerSocketHeader)pHeader)->usIdent), lmtWarning);
		break;
	}
}

void CRPSClient::SocketRead(const char* pBuf, int iCount)
{
	if (m_uiForceCloseTick > 0)
		return;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CRPSClient Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CRPSClient::CMSelectServer(char* pBuf, unsigned short usBufLen)
{

}

void CRPSClient::CMCloseWindow(char* pBuf, unsigned short usBufLen)
{

}

void CRPSClient::SendToClientPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
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
			free(pData);
		}
	}
}

/************************End Of CRPSClient***********************************************************/


/************************Start Of CClientServerSocket************************************************/
CClientServerSocket::CClientServerSocket() :m_uiLastCheckTick(0), m_sWarWarning("")				 
{
	SendDebugString("CClientServerSocket Create");
	m_OnCreateClient = std::bind(&CClientServerSocket::OnCreateClientSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CClientServerSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CClientServerSocket::OnClientConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CClientServerSocket::OnClientDisconnect, this, std::placeholders::_1);
}

CClientServerSocket::~CClientServerSocket()
{
	SendDebugString("CClientServerSocket Destroy");
}

void CClientServerSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{
	/*
	if (pIniFileParser != nullptr)
	{
		int iPort = pIniFileParser->getInteger("Setup", "GatePort", DEFAULT_GameGate_PORT);

		if (!IsActive())
		{
			m_sLocalIP = "0.0.0.0";
			m_iListenPort = iPort;
			Log("Server Listening Port = " + std::to_string(iPort), lmtMessage);
			Open();
		}
	}
	*/
	if (!IsActive())
	{
		m_sLocalIP = "0.0.0.0";
		m_iListenPort = DEFAULT_GameGate_PORT;
		Log("Server Listening Port = " + std::to_string(DEFAULT_GameGate_PORT), lmtMessage);
		Open();
	}
}

CClientConnector* CClientServerSocket::OnCreateClientSocket(const std::string& sIP)
{
	SendDebugString("OnCreateClientSocket");
	return new CRPSClient;
}

void CClientServerSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	if (iErrorCode != 10054)
		Log("Server Socket Error, Code = " + std::to_string(iErrorCode), lmtError);
	iErrorCode = 0;
}

void CClientServerSocket::OnClientConnect(void* Sender)
{
	CRPSClient* client = (CRPSClient*)Sender;
	Log("game client connect, remote ip is:" + client->GetRemoteAddress(), lmtError);
}

void CClientServerSocket::OnClientDisconnect(void* Sender)
{
	CRPSClient* client = (CRPSClient*)Sender;
	Log("game client disconnect, remote ip is:" + client->GetRemoteAddress(), lmtError);
}


/************************End Of CClientServerSocket****************************************************/

