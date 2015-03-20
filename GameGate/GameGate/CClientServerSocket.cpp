/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"

using namespace CC_UTILS;

CClientServerSocket* pG_ClientServerSocket;

/************************Start Of CClientConnector******************************************/

/************************End Of CClientConnector******************************************/


/************************Start Of CClientServerSocket******************************************/

const int DISCONNET_BUF_LEN = 512 * 1024;     //系统中本身有260k缓存是不会计算的

CClientServerSocket::CClientServerSocket()
{
	SetMaxBlockSize(DISCONNET_BUF_LEN);
	m_OnCreateClient = std::bind(&CClientServerSocket::OnCreateClientSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CClientServerSocket::OnClientError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CClientServerSocket::OnClientConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CClientServerSocket::OnClientDisconnect, this, std::placeholders::_1);
	m_OnListenReady = std::bind(&CClientServerSocket::OnListenReady, this, std::placeholders::_1);

	m_pDBServer = new CDBClientSocket();
	m_pGameServer = new CGSClientSocket();
	m_pIMServer = new CIMClientSocket();
	LoadConfig();
}

CClientServerSocket::~CClientServerSocket()
{
	Close();
	delete m_pIMServer;
	delete m_pGameServer;
	delete m_pDBServer;
}

void CClientServerSocket::SMServerConfig(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (sizeof(TServerAddress) == usBufLen)
	{
		G_PublicGateIdx = iParam;
		pG_MainThread->StartLogSocket(iParam);
		std::string sGSIP;
		PServerAddress pSA = (PServerAddress)pBuf;
		if (m_sGSIP != "")
			sGSIP = m_sGSIP;
		else
			sGSIP = pSA->IPAddress;
		Log("GameGate " + std::to_string(iParam) + " 启动.", lmtMessage);

		Log("Connect To GameServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		m_pGameServer->ConnectToServer(sGSIP, pSA->iPort);
		pSA->iPort += 10;
		Log("Connect To IMServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		m_pIMServer->ConnectToServer(sGSIP, pSA->iPort);
	}
}

void CClientServerSocket::ProcServerMessage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen)
{}

void CClientServerSocket::ClientManage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen, bool bInGame)
{}

void CClientServerSocket::GameServerShutDown()
{}

void CClientServerSocket::DoActive()
{}

void CClientServerSocket::LoadConfig()
{}

CClientConnector* CClientServerSocket::OnCreateClientSocket(const std::string &sIP)
{
	return new CPlayerClientConnector();
}

void CClientServerSocket::OnClientError(void* Sender, int& iErrorCode)
{}

void CClientServerSocket::OnListenReady(void* Sender)
{}

void CClientServerSocket::OnClientConnect(void* Sender)
{}

void CClientServerSocket::OnClientDisconnect(void* Sender)
{}

void CClientServerSocket::NotifyNotExistClient(unsigned short usHandle, int iReason)
{}

/************************End Of CClientServerSocket******************************************/