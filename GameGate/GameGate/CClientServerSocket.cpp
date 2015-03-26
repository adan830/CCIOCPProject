/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"

using namespace CC_UTILS;

CClientServerSocket* pG_ClientServerSocket;

const
MAX_GPS_TIME = 60 * 1000;                            // 最长1分钟需要返回
MAX_GPS_PACKAGE_COUNT = 20;                               // 至多等20包需要返回
MAX_GPS_ACTION_COUNT = 100;                               // 最多100个动作就需要检测

MAX_DELAY_PACKAGE = 100;


SKILL_Z_TY = 1005;                                 //跳跃
SKILL_Z_YM = 1006;                                 //野蛮

//行会操作字协议
GOP_CREATE_TITLE = 6;                                    // 创建称号 strOpStr1为新称号名称
GOP_RENAME_TITLE = 7;                                    // 称号改名 strOpStr1为旧称号，strOpStr2为新称号
GOP_MODIFY_NOTICE = 12;                                   // 修改行会通告

// CD常量
CD_ATTACK_TIME = 760;                                  //物理攻击公共CD
CD_MAGIC_TIME = 1300;                                 //魔法攻击公共CD
CD_MOVE_TIME = 600;                                  //位移公共CD
CD_USEITEM_TIME = 200;
CD_CLICKNPC_TIME = 1000;
CD_GUILDOP_TIME = 500;
CD_SAY_TIME = 2000;
CD_RUSH_TIME = 540;
CD_EMAIL_TIME = 1000;

// 硬直时间常量
STIFF_MOVE = 600;
STIFF_ATTACK = 480;
STIFF_MAGIC1 = 820;
STIFF_MAGIC2 = 820;
STIFF_SHOOT1 = 820;
STIFF_SHOOT2 = 880;
STIFF_JUMP = 720;
STIFF_PUSH = 270;
STIFF_DEFAULT = 600;
STIFF_RUSH = 500;

//服务端与客户端容错时间
SEVER_CLIENT_DIFF_TIME = 20;

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
		CClientConnector* pClient = nullptr;
		switch (usIdent)
		{
		case SM_PLAYER_MSG:
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				pClient = (CClientConnector*)ValueOf(usHandle);
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
						pClient = (CClientConnector*)ValueOf(*pHandle);
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
						pClient = (CClientConnector*)*vIter;
						if ((pClient != nullptr) && (pClient->m_ObjectID != 0))
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