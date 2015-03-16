/**************************************************************************************
@author: 陈昌
@content: AuthenServer作为客户端方连接WebInterfaceServer服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CIWebClientSocket.h"
#include "CSQLDBManager.h"

using namespace CC_UTILS;

CIWebClientSocket* pG_IWebSocket;

/************************Start Of CIWebClientSocket******************************************/

CIWebClientSocket::CIWebClientSocket() :m_uiCheckTick(0), m_iPingCount(0)
{
	m_OnConnect = std::bind(&CIWebClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CIWebClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CIWebClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CIWebClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	LoadConfig();
}

CIWebClientSocket::~CIWebClientSocket()
{}

void CIWebClientSocket::DoHeartBeat()
{
	unsigned int uiTick = GetTickCount();
	if (uiTick - m_uiCheckTick >= 10000)
	{
		m_uiCheckTick = uiTick;
		if (IsConnected())
		{
			if (m_iPingCount > 3)
			{
				m_iPingCount = 0;
				Close();
			}
			else
				SendHeartBeat();
		}
	}
}

bool CIWebClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
{
	bool retFlag = false;
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

				retFlag = (SendBuf(pData, iDataLen) == iDataLen);
				free(pData);
			}
			catch (...)
			{
				free(pData);
			}
		}
	}
	return retFlag;
}

bool CIWebClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, const std::string &str)
{
	return SendToServerPeer(usIdent, iParam, (void*)str.c_str(), str.length());
}

void CIWebClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, const char* pData, int iDataLen)
{
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
		break;
	default:
		//------------------------
		//------------------------
		//---------这样转换是否正确
		std::string str(pData, iDataLen);
		if (!pG_SQLDBManager->AddWorkJob(pHeader->usIdent, 0, pHeader->iParam, str))
		{
			str = "Error";
			SendToServerPeer(pHeader->usIdent, pHeader->iParam, (void*)str.c_str(), str.length());
		}
		break;
	}
}

void CIWebClientSocket::OnSocketConnect(void* Sender)
{
	Log("WebInterface(" + m_Address + ") Connected.");
}

void CIWebClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("WebInterface(" + m_Address + ") Disconnected.");
}

void CIWebClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CIWebClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CIWebClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	//------------------------------
	//------------------------------
	//------------------------------
	//SysErrorMessage(ErrorCode)
	if (iErrorCode != 10061)
		Log("Socket Error: SysErrorMessage(ErrorCode) (" + std::to_string(iErrorCode) + ")", lmtError);
	iErrorCode = 0;
}

void CIWebClientSocket::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
#ifdef TEST
	std::string sIPPort = "127.0.0.1:8011";
#else
	std::string sIPPort = pIniFileParser->getString("Setup", "WebInterface", DEFAULT_MQ_SERVER_IP + ':' + std::to_string(DEFAULT_WEBSERVER_PORT));
#endif
	delete pIniFileParser;

	std::vector<std::string> vec;
	SplitStr(sIPPort, ":", &vec);  //vec[0]是ip vec[1]是port
	if (vec.size() > 1)
	{
		m_Address = vec[0];
		m_Port = StrToIntDef(vec[1], DEFAULT_WEBSERVER_PORT);
		SetReconnectInterval(10000);
	}
}

void CIWebClientSocket::SendHeartBeat()
{
	SendToServerPeer(SM_PING, 0, nullptr, 0);
	++m_iPingCount;
}

/************************End Of CIWebClientSocket********************************************/