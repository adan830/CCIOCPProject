/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameGate的连接
**************************************************************************************/
#include "stdafx.h"
#include "CGGServerSocket.h"
#include "CGSServerSocket.h"
#include "CDGClientSocket.h"
using namespace CC_UTILS;

CGGServerSocket* pG_GameGateSocket;

/************************Start Of CGGConnector******************************************/

CGGConnector::CGGConnector() :m_sNetType(""), m_iServerIdx(0), m_iOnlineCount(0), m_bEnable(false), m_GamePlayerHash(1023)
{
}

CGGConnector::~CGGConnector()
{
	m_GamePlayerHash.Clear();
}

void CGGConnector::SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
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

void CGGConnector::SocketRead(const char* pBuf, int iCount)
{
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CGGConnector Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CGGConnector::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
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
		pNode->MsgFrom = fromGameGate;
		pNode->iIdx = m_iServerIdx;
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

void CGGConnector::Msg_Register(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (sizeof(TServerAddress) == usBufLen)
	{
		PServerAddress pAddr = (PServerAddress)pBuf;
		if (pG_GameGateSocket->RegisterGameGate(this, pAddr->IPAddress, pAddr->iPort))
		{
			if (m_sNetType != "")
				Log(CC_UTILS::FormatStr("GameGate %d %s:%d Enabled. NetType=%s", m_iServerIdx, pAddr->IPAddress, pAddr->iPort, m_sNetType), lmtMessage);
			else
				Log(CC_UTILS::FormatStr("GameGate %d %s:%d Enabled.", m_iServerIdx, pAddr->IPAddress, pAddr->iPort), lmtMessage);

			pAddr = pG_GameServerSocket->GetGameServerInfo();
			SendToClientPeer(SM_SERVER_CONFIG, m_iServerIdx, (char*)pAddr, sizeof(TServerAddress));
			//--------------------------------------------------
			//--------------------------------------------------
			//--------------------------------------------------
			//--------------------------------------------------
			//需要带对象一起
			pG_MainThread->SendFilterWords(SendToClientPeer);
		}
		else
			Log(CC_UTILS::FormatStr("Invalid GameGate %s:%d", pAddr->IPAddress, pAddr->iPort), lmtError);
	}
	else
		Log(CC_UTILS::FormatStr("Invalid GameGate(%s) Data. BufLen = %d", GetRemoteAddress(), usBufLen), lmtError);
}

void CGGConnector::Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (iParam >= 0)
	{
		m_bEnable = true;
		m_iOnlineCount = iParam;
	}
	SendToClientPeer(SM_PING, 0, nullptr, 0);
}

/************************End Of CGGConnector******************************************/


/************************Start Of CGGServerSocket******************************************/

CGGServerSocket::CGGServerSocket() :m_sAllowIPs("")
{
  m_OnCreateClient = std::bind(&CGGServerSocket::OnCreateGGSocket, this, std::placeholders::_1);
  m_OnClientError = std::bind(&CGGServerSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
  m_OnConnect = std::bind(&CGGServerSocket::OnGGConnect, this, std::placeholders::_1);
  m_OnDisConnect = std::bind(&CGGServerSocket::OnGGDisconnect, this, std::placeholders::_1);
  m_OnCheckAddress = std::bind(&CGGServerSocket::OnCheckConnectIP, this, std::placeholders::_1);
}

CGGServerSocket::~CGGServerSocket()
{
	m_OnCheckAddress = nullptr;
}

void CGGServerSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{
	std::string sOldAllIPs = m_sAllowIPs;
	m_sAllowIPs = pIniFileParser->getString("GameGate", "AllowIP", "127.0.0.1|");
	std::string sServer, sIP, sPort, sNetType;
	int iPos;
	int iPort = 0;
	for (int i = 0; i < MAX_GAMEGATE_COUNT; i++)
	{
		sServer = pIniFileParser->getString("GameGate", "Server" + std::to_string(i), "");
		if ("" == sServer)
			break;

		if (sServer.find(":") != std::string::npos)
		{
			std::vector<std::string> strVec;
			SplitStr(sServer, ":", &strVec);
			if (strVec.size() >= 3)
			{
				sIP = strVec[0];
				sPort = strVec[1];
				sNetType = strVec[2];
				iPort = StrToIntDef(sPort, 0);
				if (0 == iPort)
					break;
			/*
			with FServerArray[i].Addr do
			begin
			StrPlCopy(IPAddress, sIP, 15);
			nPort := iPort;
			end;
			if (FServerArray[i].NetType <> sNetType) then
			SetGameGateNet(i, sNetType);
			FServerArray[i].NetType := sNetType;
			*/
			}
		}
		else
			break;
	}

	iPort = pIniFileParser->getInteger("GameGate", "ListenPort", DEFAULT_DBServer_GG_PORT);
	if (!IsActive())
	{
		m_sLocalIP = "0.0.0.0";
		m_iListenPort = iPort;
		Open();
		Log("GameGate Service Start.(Port: " + std::to_string(iPort) + ")");
	}
	if ((sOldAllIPs != "") && (sOldAllIPs.compare(m_sAllowIPs) != 0))
		pG_GameServerSocket->SendToGameServer(SM_SERVER_CONFIG, 0, const_cast<char*>(m_sAllowIPs.c_str()), m_sAllowIPs.length());

	/*
var
  sServer, sIP, sPort, sNetType: ansistring;
  i, iPos, iPort    : integer;
  OldAllIPs         : AnsiString;
begin
  OldAllIPs := FAllowIPs;
  FAllowIPs := IniFile.ReadString('GameGate', 'AllowIP', '127.0.0.1|');
  for i := Low(FServerArray) to High(FServerArray) do
  begin
    sServer := IniFile.ReadString('GameGate', 'Server' + inttostr(i), '');
    if sServer = '' then
      Break;

  end;
  iPort := IniFile.ReadInteger('GameGate', 'ListenPort', DEFAULT_DBServer_GG_PORT); // 侦听端口
  if not Active then
  begin
    Address := '0.0.0.0';
    Port := iPort;
    Open;
    Log('GameGate Service Start.(Port: ' + IntToStr(iPort) + ')');
  end;
  if (OldAllIPs <> '') and (CompareText(FAllowIPs, OldAllIPs) <> 0) then
  begin
    G_GSSocket.BroadCastToServer(SM_SERVER_CONFIG, 0, PAnsiChar(FAllowIPs), Length(FAllowIPs));
	//G_GSSocket.SendToGameServer(SM_SERVER_CONFIG, 0, PAnsiChar(FAllowIPs), Length(FAllowIPs));
  end;
end;
	*/
}

void CGGServerSocket::GetComfyGate(int &iAddr, int &iPort, unsigned char ucNetType)
{
	iAddr = 0;
	iPort = 0;
	if (!pG_GameServerSocket->IsGameServerOK())
	{
		Log("服务器维护中,GS未连接DBServer.", lmtWarning);
		return;
	}
	int idx = 0;
	int iMinCount = MAXINT;
	{
		CGGConnector* gg = nullptr;
		std::lock_guard<std::mutex> guard(m_LockCS);
		if (ucNetType > 0)
		{
			std::string sNetType = std::to_string(ucNetType);
			std::list<void*>::iterator vIter;
			for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
			{
				gg = (CGGConnector*)*vIter;
				/*
				if (GG.FServerIdx > 0) and GG.FBoEnable and (Pos(sNetType, GG.FNetType) > 0) and (MinCount > GG.FOnLineCount) then
				begin
				  Idx := GG.FServerIdx;
				  MinCount := GG.OnLineCount;
				end;
				*/
			}

			// 找不到指定网络类型的GG
			if (0 == idx)
			{
				for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
				{
					gg = (CGGConnector*)*vIter;
					if ((gg->m_iServerIdx > 0) && gg->IsEnable() && ("" == gg->m_sNetType) && (iMinCount > gg->GetOnlineCount()))
					{
						idx = gg->m_iServerIdx;
						iMinCount = gg->GetOnlineCount();
					}
				}
			}
		}
	}

	if ((idx >= 0) && (idx < MAX_GAMEGATE_COUNT))
	{
		iAddr = inet_addr(m_ServerArray[idx].Addr.IPAddress);
		iPort = m_ServerArray[idx].Addr.iPort;
		AddOnlineCount(idx, 1);
	}
}

void CGGServerSocket::ProcGameGateMessage(PInnerMsgNode pNode)
{
	switch (pNode->usIdent)
	{
	case SM_PLAYER_CONNECT:
		SMPlayerConnect(pNode);
		break;
	case SM_PLAYER_DISCONNECT:
		//------------------------
		//------------------------
		//------------------------
		//G_UserManage.RemoveUser(Idx, Param);
		break;
	case SM_PLAYER_MSG:
		//------------------------
		//------------------------
		//------------------------
		//G_UserManage.ProcClientmsg(idx, Param, szBuf, wBufLen);
		break;
	default:
		break;
	}
}

void CGGServerSocket::KickOutClient(unsigned char ucIdx, unsigned short usHandle, int iReason)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGGConnector* gg = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gg = (CGGConnector*)*vIter;
		if (gg->m_iServerIdx == ucIdx)
		{
			gg->SendToClientPeer(SM_PLAYER_DISCONNECT, usHandle, (char*)&iReason, sizeof(int));
			break;
		}
	}
}

void CGGServerSocket::AddOnlineCount(unsigned char ucGGIdx, int iCount = 1)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGGConnector* gg = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gg = (CGGConnector*)*vIter;
		if (gg->m_iServerIdx == ucGGIdx)
		{
			gg->m_iOnlineCount += iCount;
			break;
		}
	}
}

void CGGServerSocket::SendToGameGate(unsigned char ucGGIdx, unsigned short usHandle, char* pBuf, unsigned short usBufLen)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGGConnector* gg = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gg = (CGGConnector*)*vIter;
		if (gg->m_iServerIdx == ucGGIdx)
		{
			gg->SendToClientPeer(SM_PLAYER_MSG, usHandle, pBuf, usBufLen);
			break;
		}
	}
}

void CGGServerSocket::SetGameGateNet(unsigned char ucGGIdx, const std::string &sNetType)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGGConnector* gg = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gg = (CGGConnector*)*vIter;
		if (gg->m_iServerIdx == ucGGIdx)
		{
			gg->m_sNetType = sNetType;
			Log(CC_UTILS::FormatStr("GameGate %d NetType=%s", ucGGIdx, sNetType), lmtMessage);			
			break;
		}
	}
}

void CGGServerSocket::ResendFilterWords()
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGGConnector* gg = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gg = (CGGConnector*)*vIter;
		//---------------------------
		//---------------------------
		//---------------------------
		//这里的绑定是有问题的？？？？？？？？？？？？？？？？
		if (gg->m_iServerIdx > 0)
			pG_MainThread->SendFilterWords(gg->SendToClientPeer);
	}
}

std::string CGGServerSocket::GetAllowIPs()
{
	std::string sRetStr = m_sAllowIPs;
	if ("" == sRetStr)
	{
		std::string sIP;
		for (int i = 0; i < MAX_GAMEGATE_COUNT; i++)
		{
			sIP = m_ServerArray[i].Addr.IPAddress;
			sRetStr = sRetStr + sIP + "|";
		}
	}
	return sRetStr;
}

bool CGGServerSocket::OnCheckConnectIP(const std::string &sConnectIP)
{
	bool bRetFlag = false;
	if (!G_BoClose)
	{
		bRetFlag = sConnectIP.find(m_sAllowIPs) != std::string::npos;
		if (!bRetFlag)
		{
			for (int i = 0; i < MAX_GAMEGATE_COUNT; i++)
			{
				if (sConnectIP.compare(m_ServerArray[i].Addr.IPAddress) == 0)
				{
					bRetFlag = true;
					break;
				}
			}
		}
		if (!bRetFlag)
			Log(CC_UTILS::FormatStr("GameGate[%s] 连接被禁止！！", sConnectIP), lmtWarning);
	}
	return bRetFlag;
}

void CGGServerSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log(CC_UTILS::FormatStr("CGGServerSocket Socket Error, Code = %d", iErrorCode), lmtWarning);
	iErrorCode = 0;
}

CClientConnector* CGGServerSocket::OnCreateGGSocket(const std::string &sIP)
{
	return new CGGConnector();
}

void CGGServerSocket::OnGGConnect(void* Sender)
{
	Log(CC_UTILS::FormatStr("GameGate %s Connected.", ((CGGConnector*)Sender)->GetRemoteAddress()), lmtWarning);
}

void CGGServerSocket::OnGGDisconnect(void* Sender)
{
	Log(CC_UTILS::FormatStr("GameGate %d Disconnected.", ((CGGConnector*)Sender)->m_iServerIdx), lmtWarning);
	((CGGConnector*)Sender)->m_iServerIdx = 0;
}

bool CGGServerSocket::RegisterGameGate(CGGConnector* Sender, const std::string &sAddr, int iPort)
{
	bool bRetFlag = false;
	if (iPort > 0)
	{
		for (int i = 0; i < MAX_GAMEGATE_COUNT; i++)
		{
			if ((iPort == m_ServerArray[i].Addr.iPort) && (sAddr.compare(m_ServerArray[i].Addr.IPAddress) == 0))
			{
				Sender->m_iServerIdx = i;
				Sender->m_sNetType = m_ServerArray[i].sNetType;
				bRetFlag = true;
				break;
			}
		}
	}
	if (!bRetFlag)
		Log(CC_UTILS::FormatStr("GameGate Register Error : %s : %d", sAddr, iPort), lmtError);

	return bRetFlag;
}

void CGGServerSocket::SMPlayerConnect(PInnerMsgNode pNode)
{
	bool bSuccess = false;
	if (sizeof(TPlayerConnectRec) == pNode->usBufLen)
	{
		PPlayerConnectRec pRec = (PPlayerConnectRec)(pNode->pBuf);
		TSessionInfo info;
		if (pG_DispatchGateSocket->GetSession(pRec->iSessionID, &info))
		{
			int iEncodeIdx = info.iEncodeIdx;
			int iClientType = info.iClientType;
			int iAreaID = info.iAreaID;
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				std::list<void*>::iterator vIter;
				CGGConnector* gg = nullptr;
				for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
				{
					gg = (CGGConnector*)*vIter;
					if (gg->m_iServerIdx == pNode->iIdx)
					{
						gg->SendToClientPeer(SM_PLAYER_CONNECT, pNode->iParam, (char*)&iEncodeIdx, sizeof(int));
						bSuccess = true;
						break;
					}
				}
			}
			if (bSuccess)
			{
				/*
				with nNode^ do
				  G_UserManage.AddUser(idx, Param, ClientType, AreaId, EnCodeIdx, P^.IntAddr, Info.BoMasterIP);
				*/
			}
		}
		else
		{
			SOCKADDR_IN Addr;
			Addr.sin_addr.s_addr = pRec->iIntAddr;
			//----------------------------------------
			//----------------------------------------
			//----------------------------------------
			//----------------------------------------
			//Log(CC_UTILS::FormatStr("%s 连接被强行关闭，ErrCode=%d", inet_ntoa(Addr.sin_addr), DISCONNECT_SESSION_TIMEOUT), lmtWarning);
			//----------------------------------------
			//----------------------------------------
			//----------------------------------------
			//KickOutClient(pNode->iIdx, pNode->iParam, DISCONNECT_SESSION_TIMEOUT);
		}
	}
}

/************************End Of CGGServerSocket******************************************/