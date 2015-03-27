/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"
#include "CGGGlobal.h"

using namespace CC_UTILS;

CClientServerSocket* pG_ClientServerSocket;

const int MAX_GPS_TIME = 60 * 1000;									// 最长1分钟需要返回
const int MAX_GPS_PACKAGE_COUNT = 20;                               // 至多等20包需要返回
const int MAX_GPS_ACTION_COUNT = 100;                               // 最多100个动作就需要检测
const int MAX_DELAY_PACKAGE = 100;
const int SKILL_Z_TY = 1005;										//跳跃
const int SKILL_Z_YM = 1006;										//野蛮

//行会操作字协议
const int GOP_CREATE_TITLE = 6;										// 创建称号 strOpStr1为新称号名称
const int GOP_RENAME_TITLE = 7;										// 称号改名 strOpStr1为旧称号，strOpStr2为新称号
const int GOP_MODIFY_NOTICE = 12;                                   // 修改行会通告

// CD常量
const int CD_ATTACK_TIME = 760;										//物理攻击公共CD
const int CD_MAGIC_TIME = 1300;										//魔法攻击公共CD
const int CD_MOVE_TIME = 600;										//位移公共CD
const int CD_USEITEM_TIME = 200;
const int CD_CLICKNPC_TIME = 1000;
const int CD_GUILDOP_TIME = 500;
const int CD_SAY_TIME = 2000;
const int CD_RUSH_TIME = 540;
const int CD_EMAIL_TIME = 1000;

// 硬直时间常量
const int STIFF_MOVE = 600;
const int STIFF_ATTACK = 480;
const int STIFF_MAGIC1 = 820;
const int STIFF_MAGIC2 = 820;
const int STIFF_SHOOT1 = 820;
const int STIFF_SHOOT2 = 880;
const int STIFF_JUMP = 720;
const int STIFF_PUSH = 270;
const int STIFF_DEFAULT = 600;
const int STIFF_RUSH = 500;

//服务端与客户端容错时间
const int SEVER_CLIENT_DIFF_TIME = 20;

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
{
	try
	{
		CPlayerClientConnector* pClient = nullptr;
		switch (usIdent)
		{
		case SM_PLAYER_MSG:
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				pClient = (CPlayerClientConnector*)ValueOf(usHandle);
				if (pClient != nullptr)
					pClient->ReceiveServerMsg(pBuf, usBufLen);
				else
					NotifyNotExistClient(usHandle, SM_PLAYER_MSG);
			}
			break;
		case SM_MULPLAYER_MSG:
			{
				int iHandlePos = sizeof(unsigned short) * usHandle;
				if (iHandlePos > usBufLen)
					return;	

				unsigned short usDataLen = usBufLen - iHandlePos;
				char* pData = (char*)malloc(usDataLen);
				try
				{
					unsigned short* pHandle = (unsigned short*)pBuf;
					std::lock_guard<std::mutex> guard(m_LockCS);
					for (int i = 0; i < usHandle; i++)
					{
						pClient = (CPlayerClientConnector*)ValueOf(*pHandle);
						if (pClient != nullptr)
						{
							memcpy(pData, pBuf + iHandlePos, usDataLen);
							pClient->ReceiveServerMsg(pData, usDataLen);
						}
						else
							NotifyNotExistClient(*pHandle, SM_MULPLAYER_MSG);
						++pHandle;
					}
					free(pData);
				}
				catch (...)
				{
					free(pData);
				}
			}
			break;
		case SM_BROADCAST_MSG:
			{
				char* pData = (char*)malloc(usBufLen);
				try
				{
					std::lock_guard<std::mutex> guard(m_LockCS);
					std::list<void*>::iterator vIter;
					for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
					{
						pClient = (CPlayerClientConnector*)*vIter;
						if ((pClient != nullptr) && (pClient->m_iObjectID != 0))
						{
							memcpy(pData, pBuf, usBufLen);
							pClient->ReceiveServerMsg(pData, usBufLen);
						}
					}
					free(pData);
				}
				catch (...)
				{
					free(pData);
				}
			}
			break;
		default:
			break;
		}
	}
	catch (...)
	{
		Log("ProcServerMessage: " + std::to_string(usIdent), lmtException);
	}
}

void CClientServerSocket::ClientManage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen, bool bInGame)
{}

void CClientServerSocket::GameServerShutDown()
{}

void CClientServerSocket::DoActive()
{
	++m_iLoopCount;
	if (_ExGetTickCount - m_uiSlowerTick >= 1000)
	{
		m_uiSlowerTick = _ExGetTickCount;
		if ()


		m_pDBServer->DoHeartBeat();
		m_pGameServer->DoHeartBeat();
		m_pIMServer->DoHeartBeat();
	}
	/*
  inc(m_LoopCount);
  if _GetTickCount - m_SlowerTick >= 1000 then              // 慢速执行
  begin
    m_SlowerTick := _GetTickCount;
    if G_BoardStatus[bsMinRun] < m_LoopCount then
      G_BoardStatus[bsMinRun] := m_LoopCount;
    G_BoardStatus[bsRun] := m_LoopCount;
    m_LoopCount := 0;

    UpdateLabel(Format('Run:%d/%d', [G_BoardStatus[bsRun], G_BoardStatus[bsMinRun]]), LABEL_RUN_ID);
    if not m_ListenOk then
      Exit;
    m_DBSocket.DoHeartbest;
    m_GameServer.DoHeartbeat;
    m_IMServer.DoHeartbest;
  end;
	*/
}

void CClientServerSocket::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	try
	{
		m_sDBAddr = pIniFileParser->getString("DBServer", "IP", "127.0.0.1");					// DB地址
		m_iDBPort = pIniFileParser->getInteger("DBServer", "Port", DEFAULT_DBServer_GG_PORT);	// DB端口
		m_sGSIP = pIniFileParser->getString("GameServer", "IP", "");							// Gameserver地址
		int iPort = pIniFileParser->getInteger("Setup", "Port", DEFAULT_GameGate_PORT);			// 侦听端口
		m_sInternetIP = GetInternetIP(pIniFileParser->getString("Setup", "Bind", ""));
		G_BoCheckGPS = pIniFileParser->getBoolean("Setup", "CheckGPS", false);
		iPort += G_LocalGateIdx;
		if (!IsActive())
		{
			m_sLocalIP = "0.0.0.0";
			m_iListenPort = iPort;
			Open();
		}
		delete pIniFileParser;
	}
	catch (...)
	{
		delete pIniFileParser;
	}
}

CClientConnector* CClientServerSocket::OnCreateClientSocket(const std::string &sIP)
{
	return new CPlayerClientConnector();
}

void CClientServerSocket::OnClientError(void* Sender, int& iErrorCode)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	std::string sIP;
	if (pConnector != nullptr)
		sIP = pConnector->GetRemoteAddress();
	Log(sIP + " ListenSocket Error, Code = " + std::to_string(iErrorCode), lmtMessage);
	iErrorCode = 0;
}

void CClientServerSocket::OnListenReady(void* Sender)
{
	m_bListenOK = true;
	Log("Connect To DBServer(" + m_sDBAddr + ":" + std::to_string(m_iDBPort) + ")");
	m_pDBServer->ConnectToServer(m_sDBAddr, m_iDBPort);
	UpdateLabel("Port:" + std::to_string(m_iListenPort), LABEL_PORT_ID);
}

void CClientServerSocket::OnClientConnect(void* Sender)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	if (pConnector != nullptr)
	{
		pConnector->m_OnSendToServer = std::bind(&CDBClientSocket::SendToServerPeer, m_pDBServer, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		pConnector->m_bDisconnected = false;
		UpdateLabel("Connect: " + std::to_string(m_ActiveConnects.size()) + "/" + std::to_string(m_iDelayFreeHandleCount), LABEL_CONNECT_ID);
	}
}

void CClientServerSocket::OnClientDisconnect(void* Sender)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	if (pConnector != nullptr)
	{
		UpdateLabel("Connect: " + std::to_string(m_ActiveConnects.size()) + "/" + std::to_string(m_iDelayFreeHandleCount), LABEL_CONNECT_ID);
		m_pDBServer->ClientDisconnect(pConnector->GetSocketHandle());
		m_pGameServer->ClientDisconnect(pConnector->GetSocketHandle());
		m_pIMServer->ClientDisconnect(pConnector->GetSocketHandle());;
		pConnector->OnDisconnect();
	}
}

void CClientServerSocket::NotifyNotExistClient(unsigned short usHandle, int iReason)
{
	m_pDBServer->ClientDisconnect(usHandle);
	m_pGameServer->ClientDisconnect(usHandle);
	m_pIMServer->ClientDisconnect(usHandle);	
#ifdef TEST
	Log("Not Found Client " + std::to_string(usHandle) + ", Reason=" + std::to_string(iReason), lmtError);
#endif
}

/************************End Of CClientServerSocket******************************************/