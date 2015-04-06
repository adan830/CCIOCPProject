/**************************************************************************************
@author: 陈昌
@content: GameServer对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"

using namespace CC_UTILS;

const int DELAY_DISCONNECT_TIME = 3000;                   // 延时断开时间

CClientServerSocket* pG_GameSocket;

/************************Start Of CRPSClient********************************************************/
CRPSClient::CRPSClient() : m_uiForceCloseTick(0), m_iCurrentRound(0), m_iTotalWins(0), m_iTotalLosses(0), m_iTotalTies(0)
{

}

CRPSClient::~CRPSClient()
{}

void CRPSClient::ForceClose()
{
	m_uiForceCloseTick = GetTickCount();
}

void CRPSClient::Execute(unsigned int uiTick)
{
	CClientConnector::Execute(uiTick);
	if ((m_uiForceCloseTick > 0) && (uiTick > m_uiForceCloseTick) && (uiTick - m_uiForceCloseTick > DELAY_DISCONNECT_TIME))
	{
		Close();
	}
}

void CRPSClient::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
{
	switch (((PServerSocketHeader)pHeader)->usIdent)
	{
	case CM_PING:
		SendToClientPeer(SCM_PING, 0, nullptr, 0);
		break;
	case CM_PLAY_REQ:
		CMPlayReq(((PServerSocketHeader)pHeader)->iParam);
		break;
	case CM_QUIT:
		ForceClose();
		break;
	default:
		Log("received unknown client msg，IP=" + GetRemoteAddress() + " Ident=" + std::to_string(((PServerSocketHeader)pHeader)->usIdent), lmtWarning);
		break;
	}
}

void CRPSClient::SocketRead(const char* pBuf, int iCount)
{
	if (m_uiForceCloseTick > 0)
		return;
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CRPSClient Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CRPSClient::CMPlayReq(const int iClientChoose)
{
	++m_iCurrentRound;
	int iServerChoose = rand() % 3;
	int iConclusion = CheckGamePlayConclusion(iClientChoose, iServerChoose);
	//this string connect is not good
	Log("player [" + std::to_string((int)this) + "]: Round " + std::to_string(m_iCurrentRound) + " -> client [" + G_RPS_STRING[iClientChoose] + 
		"] server [" + G_RPS_STRING[iServerChoose] + "] " + G_RPS_CONCLUSION[iConclusion]);
	switch (iConclusion)
	{
	case 0:
		++m_iTotalTies;
		break;
	case 1:
		++m_iTotalWins;
		break;
	case 2:
		++m_iTotalLosses;
		break;
	default:
		break;
	}

	TGamePlayAckPkg pkg;
	pkg.iRoundCount = m_iCurrentRound;
	pkg.iClientChoose = iClientChoose;
	pkg.iServerChoose = iServerChoose;
	pkg.iConclusion = iConclusion;
	pkg.iTotalLooses = m_iTotalLosses;
	pkg.iTotalTies = m_iTotalTies;
	pkg.iTotalWins = m_iTotalWins;
	SendToClientPeer(SCM_PLAY_ACK, 0, &pkg, sizeof(TGamePlayAckPkg));
}

//return 0 is tie, 1 client win, 2 client loose
int CRPSClient::CheckGamePlayConclusion(const int iClientChoose, const int iServerChoose)
{
	int iRetConclusion = 0;
	if (iClientChoose != iServerChoose)
	{
		switch (iClientChoose)
		{
		case rps_rock:
			if (iServerChoose == rps_paper)
				iRetConclusion = 2;
			else
				iRetConclusion = 1;
			break;
		case rps_paper:
			if (iServerChoose == rps_scissors)
				iRetConclusion = 2;
			else
				iRetConclusion = 1;
			break;
		case rps_scissors:
			if (iServerChoose == rps_rock)
				iRetConclusion = 2;
			else
				iRetConclusion = 1;
			break;
		default:
			break;
		}
	}
	return iRetConclusion;
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
	m_OnCreateClient = std::bind(&CClientServerSocket::OnCreateClientSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CClientServerSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CClientServerSocket::OnClientConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CClientServerSocket::OnClientDisconnect, this, std::placeholders::_1);

	srand(time(0));
}

CClientServerSocket::~CClientServerSocket()
{
}

void CClientServerSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{
	/*
	if (pIniFileParser != nullptr)
	{
		int iPort = pIniFileParser->getInteger("Setup", "ListenPort", DEFAULT_LISTENING_PORT);

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
		m_iListenPort = DEFAULT_LISTENING_PORT;
		Log("Server Listening Port = " + std::to_string(DEFAULT_LISTENING_PORT), lmtMessage);
		Open();
	}
}

CClientConnector* CClientServerSocket::OnCreateClientSocket(const std::string& sIP)
{
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

