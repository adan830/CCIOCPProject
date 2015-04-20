/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameGate的连接
**************************************************************************************/
#include "stdafx.h"
#include "CGGServerSocket.h"
#include "CGSServerSocket.h"
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
	/*
var
  P                 : PServerAddress;
begin
  if BufLen = sizeof(TServerAddress) then
  begin
    P := PServerAddress(Buf);
    if G_GGSocket.RegisterGameGate(Self, P^.IPAddress, P^.nPort) then
    begin
      if FNetType <> '' then
        Log(Format('GameGate %d %s:%d Enabled. NetType=%s', [FServerIdx, p^.IPAddress, p^.nPort, FNetType]), lmtMessage)
      else
        Log(Format('GameGate %d %s:%d Enabled.', [FServerIdx, p^.IPAddress, p^.nPort]), lmtMessage);
      P := G_GSSocket.GetGameServerInfo;
      SendBuffer(SM_SERVER_CONFIG, FServerIdx, PAnsiChar(P), sizeof(TServerAddress));
      G_MainThread.SendFilterWords(SendBuffer);
    end
    else
      Log(Format('Invalid GameGate %s:%d', [P^.IPAddress, P^.nPort]), lmtError);
  end
  else
    Log('Invalid GameGate(' + RemoteAddress + ') Data. BufLen = ' + IntToStr(BufLen), lmtError);
end;
	*/
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

}

void CGGServerSocket::KickOutClient(unsigned char ucIdx, unsigned short usHandle, int iReason)
{

}

void CGGServerSocketAddOnlineCount(unsigned char ucGGIdx, int iCount = 1)
{

}

void CGGServerSocket::SendToClientPeer(unsigned char ucGGIdx, unsigned short usHandle, char* pBuf, unsigned short usBufLen)
{

}

void CGGServerSocket::SetGameGateNet(unsigned char ucGGIdx, const std::string &sNetType)
{

}

void CGGServerSocket::ResendFilterWords()
{

}

std::string CGGServerSocket::GetAllowIPs()
{

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
		----
	}
	/*
var
  Success           : Boolean;
  i, EnCodeIdx, ClientType, AreaId: integer;
  Info              : TSessionInfo;
  GG                : TGameGate;
  P                 : PPlayerConnectRec;
  Addr              : In_Addr;
begin
  Success := False;
  if nNode^.wBufLen = sizeof(TPlayerConnectRec) then
  begin
    P := PPlayerConnectRec(nNode^.szBuf);
    if G_DispatchGate.GetSession(P^.SessionID, @Info) then
    begin
      EnCodeIdx := Info.EncodeIdx;
      ClientType := Info.ClientType;
      AreaId := Info.AreaID;
      Lock;
      try
        for i := 0 to ActiveConnects.Count - 1 do
        begin
          GG := TGameGate(ActiveConnects.Items[i]);
          if GG.FServerIdx = nNode^.idx then
          begin
            GG.SendBuffer(SM_PLAYER_CONNECT, nNode^.Param, @EnCodeIdx, sizeof(integer));
            Success := True;
            Break;
          end;
        end;
      finally
        UnLock;
      end;
      if Success then
      begin
        with nNode^ do
          G_UserManage.AddUser(idx, Param, ClientType, AreaId, EnCodeIdx, P^.IntAddr, Info.BoMasterIP);
      end;
    end
    else
    begin
      Addr.S_addr := P^.IntAddr;
      Log(Format('%s 连接被强行关闭，ErrCode=%d', [inet_ntoa(Addr), DISCONNECT_SESSION_TIMEOUT]), lmtWarning);
      KickOutClient(nNode^.idx, nNode^.Param, DISCONNECT_SESSION_TIMEOUT);
    end;
  end;
end;
	*/
}

/************************End Of CGGServerSocket******************************************/