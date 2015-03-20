/**************************************************************************************
@author: 陈昌
@content: GameGate作为客户端方连接DBServer服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CDBClientSocket.h"

using namespace CC_UTILS;

CDBClientSocket* pG_DBSocket;

/************************Start Of CDBClientSocket******************************************/

CDBClientSocket::CDBClientSocket() : m_iPingCount(0), m_uiLastPingTick(0), m_bEnable(false), m_ForbiddenWords(300)
{
	m_OnConnect = std::bind(&CDBClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CDBClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CDBClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CDBClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
}

CDBClientSocket::~CDBClientSocket()
{}

bool CDBClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, void* pBuf, unsigned short usBufLen)
{
	bool bRetFlag = false;
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

				bRetFlag = (SendBuf(pData, iDataLen) == iDataLen);
				free(pData);
			}
			catch (...)
			{
				free(pData);
			}
		}
	}
	return bRetFlag;
}

void CDBClientSocket::ConnectToServer(const std::string &sAddr, const int iPort)
{
	m_Address = sAddr;
	m_Port = iPort;
	SetReconnectInterval(10 * 1000);
}

void CDBClientSocket::ClientDisconnect(unsigned short usHandle)
{
	SendToServerPeer(SM_PLAYER_DISCONNECT, usHandle, nullptr, 0);
}

void CDBClientSocket::DoHeartBeat()
{
	if (_ExGetTickCount - m_uiLastPingTick >= 10000)
	{
		m_uiLastPingTick = _ExGetTickCount;
		if (IsConnected())
		{
			if (m_iPingCount > 3)
			{
				m_iPingCount = 0;
				Close();
			}
			else
			{
				//--------------------------------
				//--------------------------------
				//--------------------------------
				//SendHeartBeat(G_ServerSocket.Count);
			}
		}
	}
}

void CDBClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
		break;
	case SM_SERVER_CONFIG:
		//---------------------------
		//---------------------------
		//---------------------------
		//G_ServerSocket.smServerConfig(nParam, buf, Buflen);
		break;
	case SM_FILTER_WORD:
		AddForbiddenWord(pData, iDataLen, pHeader->iParam);
		break;
	case SM_PLAYER_CONNECT:
	case SM_PLAYER_DISCONNECT:
		//---------------------------
		//---------------------------
		//---------------------------
		//if Assigned(G_ServerSocket) then
		//	G_ServerSocket.ClientManage(Ident, nParam, Buf, BufLen, False);
		break;
	default:
		//----------------------------
		//----------------------------
		//----------------------------
		//if Assigned(G_ServerSocket) then
		//	G_ServerSocket.ProcServerMessage(Ident, nParam, Buf, BufLen);
		break;
	}
}

void CDBClientSocket::OnSocketConnect(void* Sender)
{
	Log("DBServer(" + m_Address + ":" + std::to_string(m_Port) + ") Connected.");
	SendRegisterServer();
}

void CDBClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("DBServer(" + m_Address + ":" + std::to_string(m_Port) + ") DisConnected.");
}

void CDBClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CDBClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CDBClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("DBServer Error, Code = " + std::to_string(iErrorCode), lmtWarning);
	iErrorCode = 0;
}

void CDBClientSocket::SendRegisterServer()
{
	std::string sAddr = ""; 
	//----------------------------
	//----------------------------
	//----------------------------
	//G_ServerSocket.InternetIP;
	TServerAddress info;
	memcpy_s(info.IPAddress, sizeof(info.IPAddress), sAddr.c_str(), sAddr.length() + 1);
	info.iPort = 0;
	//----------------------------
	//----------------------------
	//----------------------------
	//G_ServerSocket.Port;
	SendToServerPeer(SM_REGISTER, 0, &info, sizeof(TServerAddress));
}

void CDBClientSocket::SendHeartBeat(int iConnectCount)
{
	if (m_bEnable)
		SendToServerPeer(SM_PING, iConnectCount, nullptr, 0);
	else
		SendToServerPeer(SM_PING, -1, nullptr, 0);
}

void CDBClientSocket::SetEnable(bool bFlag)
{
	m_bEnable = bFlag;
	//-----------------------------
	//-----------------------------
	//-----------------------------
	//SendHeartBeat(G_ServerSocket.Count)
}

char* CDBClientSocket::IsIncludeForbiddenWord(const char* pMsg)
{
	/*
var
  i                 : Integer;
  s                 : ansistring;
  theWord           : PAnsiChar;
begin
  Result := nil;
  for i := 0 to m_ForbiddenWords.Count - 1 do
  begin
    s := m_ForbiddenWords[i];
    theWord := PAnsiChar(s);
    Result := AnsiStrPos(theWord, pmsg);
    if Assigned(Result) then
      Break;
    Result := AnsiStrPos(pmsg, theWord);
    if Assigned(Result) then
    begin
      Result := theWord;
      Break;
    end;
  end;
end;
	*/
	return nullptr;
}

void CDBClientSocket::AddForbiddenWord(char* pBuf, unsigned short usBufLen, int iCount)
{
	/*
var
  tmpList           : TStringList;
  s                 : ansistring;
begin
  if BufLen <= 0 then
  begin
    m_ForbiddenWords.Clear;
    Exit;
  end;
  SetString(s, Buf, BufLen);
  tmpList := TStringList.Create;
  try
    tmpList.Text := s;
    m_ForbiddenWords.AddStrings(tmpList);
    Log(Format('收到过滤字(%d Bytes)：%d/%d 条', [BufLen, m_ForbiddenWords.Count, Count]), lmtMessage);
  finally
    tmpList.Free;
  end;
end;
	*/
}

/************************End Of CDBClientSocket********************************************/