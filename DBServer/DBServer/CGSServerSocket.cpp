/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameServer的连接
**************************************************************************************/
#include "stdafx.h"
#include "CGSServerSocket.h"
#include "CGGServerSocket.h"

using namespace CC_UTILS;

CGSServerSocket* pG_GameServerSocket;

/************************Start Of CGSConnector******************************************/

CGSConnector::CGSConnector() :m_iOnlineCount(0), m_bEnable(false)
{}

CGSConnector::~CGSConnector()
{}

void CGSConnector::SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
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

void CGSConnector::SocketRead(const char* pBuf, int iCount)
{
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CGSConnector Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CGSConnector::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
{
	PInnerMsgNode pNode;
	PServerSocketHeader pH = (PServerSocketHeader)pHeader;
	switch (pH->usIdent)
	{
	case SM_REGISTER:
		Msg_Register(pH->iParam, pData, iDataLen);
		break;
	case SM_PING:
		Msg_Ping(pH->iParam, pData, iDataLen);
		break;
	default:
		//其它消息都重新回到主线程的队列中处理，保证线程安全
		pNode = new TInnerMsgNode();
		pNode->MsgFrom = fromGameServer;
		pNode->iIdx = pH->usIdent;
		pNode->iParam = pH->iParam;
		pNode->usIdent = pH->usIdent;
		if ((pData != nullptr) && (iDataLen > 0))
		{
			pNode->usBufLen = iDataLen;
			pNode->pBuf = (char*)malloc(iDataLen);
			memcpy_s(pNode->pBuf, iDataLen, pData, iDataLen);
		}
		else
		{
			pNode->pBuf = nullptr;
			pNode->usBufLen = 0;
		}
		pG_MainThread->ReceiveMessage(pNode);
		break;
	}
}

void CGSConnector::Msg_Register(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (pG_GameServerSocket->RegisterGameServer(GetRemoteAddress(), iParam))
	{
		m_bEnable = true;
		std::string sAddr = pG_GameGateSocket->GetAllowIPs();
		SendToClientPeer(SM_SERVER_CONFIG, G_DataBaseID, const_cast<char*>(sAddr.c_str()), sAddr.length());
		Log(CC_UTILS::FormatStr("Invalid GameServer %s:%d", GetRemoteAddress(), iParam), lmtError);
	}
	else
		Log(CC_UTILS::FormatStr("Invalid GameServer %s:%d", GetRemoteAddress(), iParam), lmtError);
}

void CGSConnector::Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen)
{
	m_iOnlineCount = iParam;
	SendToClientPeer(SM_PING, 0, nullptr, 0);
}

/************************End Of CGSConnector******************************************/



/************************Start Of CGSServerSocket******************************************/

CGSServerSocket::CGSServerSocket() :m_iMaxOnlineCount(0), m_sAllowIPs(""), m_bShutDown(false), m_uiShutDownTick(0)
{
	m_OnCreateClient = std::bind(&CGSServerSocket::OnCreateGSSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CGSServerSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CGSServerSocket::OnGSConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CGSServerSocket::OnGSDisconnect, this, std::placeholders::_1);
	m_OnCheckAddress = std::bind(&CGSServerSocket::OnCheckConnectIP, this, std::placeholders::_1);

	m_pSendCache = (char*)malloc(MAXWORD);
	memset(&m_ServerInfo, 0, sizeof(TServerAddress));
}

CGSServerSocket::~CGSServerSocket()
{
	free(m_pSendCache);
	m_OnCheckAddress = nullptr;
}

void CGSServerSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{

	/*
var
  sServer, sIP, sPort: ansistring;
  iPos, iPort       : integer;
begin
  FMaxOnline := IniFile.ReadInteger('Setup', 'MaxCount', 10000);
  FAllowIPs := IniFile.ReadString('GameServer', 'AllowIP', '127.0.0.1|');
  sServer := IniFile.ReadString('GameServer', 'Address', '127.0.0.1');
  if sServer <> '' then
  begin
    iPort := DEFAULT_GameServer_PORT;
    iPos := Pos(':', sServer);
    if iPos > 0 then
    begin
      sIP := Copy(sServer, 1, iPos - 1);
      sPort := Copy(sServer, iPos + 1, 4);
      iPort := StrToIntDef(sPort, DEFAULT_GameServer_PORT);
    end
    else
      sIP := sServer;
    if iPort > 0 then
    begin
      with FServerInfo do
      begin
        StrPlCopy(IPAddress, sIP, 15);
        nPort := iPort;
      end;
    end;
  end;
  iPort := IniFile.ReadInteger('GameServer', 'ListenPort', DEFAULT_DBServer_GS_PORT); // 侦听端口
  if not Active then
  begin
    Address := '0.0.0.0';
    Port := iPort;
    Open;
    Log('GameServer Service Start.(Port: ' + IntToStr(iPort) + ')');
  end;
end;
	*/

}

PServerAddress CGSServerSocket::GetGameServerInfo()
{
	return &m_ServerInfo;
}

bool CGSServerSocket::IsGameServerOK()
{
	bool bRetFlag = false;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGSConnector* gs = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gs = (CGSConnector*)*vIter;
		if (gs->IsEnable())
		{
			bRetFlag = true;
			break;
		}
	}
	return bRetFlag;
}

void CGSServerSocket::GameServerShutDown()
{
	m_uiShutDownTick = _ExGetTickCount;
	m_bShutDown = true;
}

void CGSServerSocket::ProcGameServerMessage(PInnerMsgNode pNode)
{}

bool CGSServerSocket::SendToGameServer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	bool bRetFlag = false;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGSConnector* gs = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gs = (CGSConnector*)*vIter;
		if (gs->IsEnable())
		{
			bRetFlag = true;
			gs->SendToClientPeer(usIdent, iParam, pBuf, usBufLen);
		}
	}
	return bRetFlag;
}

void CGSServerSocket::BroadcastToGS(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{}

bool CGSServerSocket::CanClosed()
{}

int CGSServerSocket::GetHumanCount()
{}

CGSConnector* CGSServerSocket::GetActiveGameServer()
{}

bool CGSServerSocket::RegisterGameServer(const std::string &sGSAddr, int iGSPort)
{}

void CGSServerSocket::Msg_DataRead(int iSessionID, char* pBuf, unsigned short usBufLen)
{}

void CGSServerSocket::Msg_DataWrite(int iSessionID, char* pBuf, unsigned short usBufLen)
{}

void CGSServerSocket::Msg_GameActCode(int iSessionID, char* pBuf, unsigned short usBufLen)
{}

bool CGSServerSocket::OnCheckConnectIP(const std::string &sConnectIP)
{}

void CGSServerSocket::OnSocketError(void* Sender, int& iErrorCode)
{}

CClientConnector* CGSServerSocket::OnCreateGSSocket(const std::string &sIP)
{}

void CGSServerSocket::OnGSConnect(void* Sender)
{}

void CGSServerSocket::OnGSDisconnect(void* Sender)
{}

void CGSServerSocket::ProcGMCmd(int iSessionID, std::string &sCmdStr)
{}


/************************End Of CGSServerSocket******************************************/