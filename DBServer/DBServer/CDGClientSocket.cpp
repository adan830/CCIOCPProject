/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接DispatchGate服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CDGClientSocket.h"
#include "CGSServerSocket.h"

using namespace CC_UTILS;

CDGClientSocket* pG_DispatchGate;

#ifdef TEST
const int MAX_SESSION_TIMEOUT = 30 * 1000;                          // 30秒SessionID有效时间
#else
const int MAX_SESSION_TIMEOUT = 10 * 1000;                          // 10秒SessionID有效时间
#endif

/************************Start Of CDGClientSocket******************************************/

CDGClientSocket::CDGClientSocket() :m_uiCheckTick(0), m_iPingCount(0), m_bDenyAll(false), m_iConfigFileAge(0), m_SessionHash(1023), m_TraceList(255), m_iWorkIndex(0)
{
	m_OnConnect = std::bind(&CDGClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CDGClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CDGClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CDGClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	m_SessionHash.Remove = std::bind(&CDGClientSocket::OnRemoveSession, this, std::placeholders::_1, std::placeholders::_2);
}

CDGClientSocket::~CDGClientSocket()
{
	m_SessionHash.Clear();
	m_TraceList.clear();
}

void CDGClientSocket::DoHeartbeat()
{
	unsigned int uiInterval = 3000;
	if (IsConnected())
		uiInterval = 10000;

	unsigned int uiTick = _ExGetTickCount;
	if (uiTick - m_uiCheckTick >= uiInterval)
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
				SendHeartbeat();
		}
		else
			Reconnect();
	}
}

void CDGClientSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{
	std::string sServer;
	int iPort = 0;
	int iPos = 0;
	for (int i = 0; i < MAX_DISPATCHGATE_NUM; i++)
	{
		sServer = pIniFileParser->getString("DispatchGate", "Server" + std::to_string(i + 1), "");
		if ("" == sServer)
			break;

		iPort = DEFAULT_DispatchGate_DB_PORT;
		std::vector<std::string> strVec;
		SplitStr(sServer, ":", &strVec);
		std::string sIP;
		std::string sPort;
		if (1 == strVec.size())
			sIP = sServer;
		else
		{
			sIP = strVec[0];
			sPort = strVec[1];
			iPort = StrToIntDef(sPort, DEFAULT_DispatchGate_DB_PORT);
		}
		//--------------------------------
		//--------------------------------
		//--------------------------------??????????????
		memcpy_s(m_ServerArray[i].IPAddress, IP_ADDRESS_MAX_LEN + 1, sIP.c_str(), IP_ADDRESS_MAX_LEN + 1);
		m_ServerArray[i].iPort = iPort;
	}
	m_iWorkIndex = 0;
}

bool CDGClientSocket::Closed()
{
	Close();
	return !IsConnected();
}

bool CDGClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
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
		return bRetFlag;
	}
}

void CDGClientSocket::ProcDispatchMessage(PInnerMsgNode pNode)
{
	switch (pNode->usIdent)
	{
	case SM_PIG_MSG:
		//-------------------------------
		//-------------------------------
		//-------------------------------
		//G_GSSocket.SendToGameServer(0, SM_PIG_MSG, nNode^.szBuf, nNode^.wBufLen);
		break;
	default:
		break;
	}
}

bool CDGClientSocket::IsTraceRole(const std::string &sRoleName)
{
	return false;
	if (m_TraceList.size() > 0)
	{
		std::string sLowerRoleName = sRoleName;
		std::transform(sLowerRoleName.begin(), sLowerRoleName.end(), sLowerRoleName.begin(), tolower);
		return (std::find(m_TraceList.begin(), m_TraceList.end(), sLowerRoleName) != m_TraceList.end());
	}
}

bool CDGClientSocket::GetSession(int iSessionID, PSessionInfo pResult)
{
	bool bRetFlag = false;	
	{
		std::lock_guard<std::mutex> guard(m_SessionCS);
		PSessionInfo pInfo = (PSessionInfo)m_SessionHash.ValueOf(iSessionID);
		if (pInfo != nullptr)
		{
			if (_ExGetTickCount - pInfo->uiCreateTick <= MAX_SESSION_TIMEOUT)
			{
				*pResult = *pInfo;
				bRetFlag = true;
			}
			m_SessionHash.Remove(iSessionID);
		}		
	}
	return bRetFlag;
}

bool CDGClientSocket::ProcGMCmd(int iSessionID, const std::string &sParam1, const std::string &sParam2, const std::string &sParam3)
{
	bool bRetFlag = false;
	std::string sHint("");
	if (sParam1.compare("Trace") == 0)
	{
		std::string sLowerParam2 = sParam2;
		std::transform(sLowerParam2.begin(), sLowerParam2.end(), sLowerParam2.begin(), tolower);
		if (sParam2.compare("Clear") == 0)
		{
			m_TraceList.clear();
			sHint = "跟踪列表清理完毕";
		}
		else if (sParam2.compare("Look") == 0)
		{
			sHint = FTraceList.Text;
		}
		else if (sParam3.compare("Del") == 0)
		{
			std::vector<std::string>::iterator vIter = std::find(m_TraceList.begin(), m_TraceList.end(), sLowerParam2);
			if (vIter != m_TraceList.end())
			{
				m_TraceList.erase(vIter);
				sHint = sParam2 + " 删除成功";
			}
			else
				sHint = sParam2 + " 已经存在";
		}
		else if (std::find(m_TraceList.begin(), m_TraceList.end(), sLowerParam2) == m_TraceList.end())
		{
			m_TraceList.push_back(sLowerParam2);
			sHint = sParam2 + " 添加成功";
		}

		bRetFlag = true;
	}
	else if ((sParam1.compare("Deny") == 0) || (sParam1.compare("Allow") == 0))
	{
		if (sParam2.compare("Look") == 0)
			sHint = GetConfigInfo(sParam1);
		else if (SetConfig(sParam1, sParam2, (sParam3.compare("Del") == 0)))
			sHint = CC_UTILS::FormatStr("设置成功,可使用@DBServer %s look 查看", sParam1);

		if (m_bDenyAll && (sParam1.compare("Deny") == 0))
			sHint = "当前禁止所有外部IP登陆!";
	}

	if ("" == sHint)
	{
		/*
		//-------------------------------
		//-------------------------------
		//-------------------------------
		User: = G_UserManage.FindUser(SessionID);
			if Assigned(User) then
				User.SendMsg(Hint);
		*/
	}

	return bRetFlag;
}

void CDGClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	PInnerMsgNode pNode;
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
		break;
	case SM_SELECT_SERVER:
		MsgSelectServer(pNode->iParam, pNode->pBuf, pNode->usBufLen);  //iParam是客户端在DispatchGate的SocketHandle 		
		break;
	default:
		pNode = new TInnerMsgNode();
		pNode->MsgFrom = fromDispatchGate;
		pNode->iIdx = 0;
		pNode->iParam = pHeader->iParam;
		pNode->usIdent = pHeader->usIdent;
		if ((pData != nullptr) && (iDataLen > 0))
		{
			pNode->usBufLen = iDataLen;
			pNode->pBuf = (char*)malloc(iDataLen);
			memcpy(pNode->pBuf, pData, iDataLen);
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

void CDGClientSocket::OnSocketConnect(void* Sender)
{
	Log("DispatchGate(" + m_Address + ") connected.");
	SendRegisterServer();
	LoadIpConfigFile();
}

void CDGClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("DispatchGate(" + m_Address + ") disconnected.");
	m_iConfigFileAge = 0;
}

void CDGClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	if (G_BoClose)
		return;
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CDGClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CDGClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("CDGClientSocket Socket Error, Code = " + std::to_string(iErrorCode), lmtError);
	iErrorCode = 0;
}

void CDGClientSocket::Reconnect()
{
	if (!IsActive())
	{
		m_Address = m_ServerArray[m_iWorkIndex].IPAddress;
		m_Port = m_ServerArray[m_iWorkIndex].iPort;
		if (m_Port > 0)
		{
			Log("Connect to Dispatch Gate(" + m_Address + ").");
			Open();
		}
	}
	// 切换Dispatch Gate
	++m_iWorkIndex;
	if (m_iWorkIndex >= MAX_DISPATCHGATE_NUM)
		m_iWorkIndex = 0;
}

void CDGClientSocket::SendHeartbeat()
{
	int iCount = 0;
	if (pG_GameServerSocket != nullptr)
		iCount = pG_GameServerSocket->HumanCount;
	SendToServerPeer(SM_PING, iCount, nullptr, 0);
}

void CDGClientSocket::OnRemoveSession(void* pValue, int iKey)
{
	delete ((PSessionInfo)pValue);
}

void CDGClientSocket::LoadIpConfigFile()
{
	/*
var
  i, iage           : integer;
  tmpList           : TStringList;
  key, value, FileName: ansistring;
begin
  FBoDenyAll := False;
  FileName := ExtractFilePath(ParamStr(0)) + 'ipaddress.txt';
  if FileExists(FileName) then
  begin
    iage := FileAge(FileName);
    if iage <> FConfigFileAge then
    begin
      if FConfigFileAge > 0 then
        Log('Reload ipaddress.txt...', lmtMessage);
      SendToServer(SM_SERVER_CONFIG, 2 or $8000, nil, 0);
      SendToServer(SM_SERVER_CONFIG, 3 or $8000, nil, 0);
      FConfigFileAge := iage;
      tmpList := TStringList.Create;
      tmpList.Delimiter := '=';
      tmpList.LoadFromFile(FileName);
      for i := 0 to tmpList.Count - 1 do
      begin
        key := Trim(tmpList.Names[i]);
        value := Trim(tmpList.ValueFromIndex[i]);
        SendConfig(key, value);
      end;
      tmpList.Free;
    end;
  end;
end;
	*/
}

void CDGClientSocket::SendConfig(const std::string &sKey, std::string &sValue)
{
	/*
  if CompareText(key, 'DenyHint') = 0 then
  begin
    if value <> '' then
    begin
      value := StringReplace(value, '#13#10', #13#10, [rfReplaceAll]);
      value := StringReplace(value, '</br>', #13#10, [rfReplaceAll]);
      SendToServer(SM_SERVER_CONFIG, 1, PAnsiChar(value), Length(Value));
    end;
  end
  else if CompareText(key, 'Deny') = 0 then
  begin
    if Value = '' then
      SendToServer(SM_SERVER_CONFIG, 2 or $8000, nil, 0)
    else
    begin
      SendToServer(SM_SERVER_CONFIG, 2, PAnsiChar(value), Length(Value));
      if CompareText(value, 'all') = 0 then
        FBoDenyAll := True;
    end;
  end
  else if CompareText(key, 'Allow') = 0 then
  begin
    if value = '' then
      SendToServer(SM_SERVER_CONFIG, 3 or $8000, nil, 0)
    else
      SendToServer(SM_SERVER_CONFIG, 3, PAnsiChar(value), Length(Value));
  end
  else if CompareText(key, 'Trace') = 0 then
  begin
    if (Value <> '') and (FTraceList.IndexOf(LowerCase(value)) < 0) then
      FTraceList.Add(LowerCase(value));
  end;
	*/
}

bool CDGClientSocket::SetConfig(const std::string &sKey, const std::string &sValue, bool bDel)
{
	/*
var
  i                 : Integer;
  tmpList           : TStringList;
  FileName, sKey, sValue: AnsiString;
  BoFound           : Boolean;
begin
  Result := False;
  tmpList := TStringList.Create;
  try
    tmpList.Delimiter := '=';
    FileName := ExtractFilePath(ParamStr(0)) + 'ipaddress.txt';
    if FileExists(FileName) then
      tmpList.LoadFromFile(FileName);
    BoFound := False;
    for i := tmpList.Count - 1 downto 0 do
    begin
      sKey := Trim(tmpList.Names[i]);
      sValue := Trim(tmpList.ValueFromIndex[i]);
      if CompareText(sKey, Key) = 0 then
      begin
        if CompareText('DenyHint', sKey) = 0 then
        begin
          tmpList.Strings[i] := sKey + '=' + Value;
          BoFound := True;
          Result := True;
          Break;
        end
        else if (CompareText(sValue, Value) = 0) then
        begin
          Result := True;
          if bDel then
            tmpList.Delete(i)
          else
            Exit;
        end;
      end;
    end;
    if not bDel and not BoFound then
      tmpList.Add(Key + '=' + Value);
    tmpList.SaveToFile(FileName);
  finally
    tmpList.Free;
  end;
end;
	*/
}

std::string CDGClientSocket::GetConfigInfo(const std::string &sKey)
{
	/*
var
  i                 : integer;
  tmpList           : TStringList;
  FileName, TempStr : AnsiString;
begin
  Result := '';
  FileName := ExtractFilePath(ParamStr(0)) + 'ipaddress.txt';
  if FileExists(FileName) then
  begin
    tmpList := TStringList.Create;
    try
      tmpList.LoadFromFile(FileName);
      for i := 0 to tmpList.Count - 1 do
      begin
        TempStr := tmpList.Strings[i];
        if (TempStr = '') or (TempStr[1] = '#') or (TempStr[1] = ';') then
          Continue;
        if Pos(Key, TempStr) > 0 then
          Result := Result + TempStr + #13#10;
      end;
    finally
      tmpList.Free;
    end;
  end;
end;
	*/
}

void CDGClientSocket::SendRegisterServer()
{
	SendToServerPeer(SM_REGISTER, G_ServerID, nullptr, 0);
}

void CDGClientSocket::MsgSelectServer(int iParam, char* pBuf, unsigned short usBufLen)
{
	PClientSelectServerInfo pInfo = (PClientSelectServerInfo)pBuf;
	if (sizeof(TClientSelectServerInfo) == usBufLen)
	{
		PSessionInfo pSession = new TSessionInfo();
		memset(pSession, 0, sizeof(TSessionInfo));
		pSession->iEncodeIdx = pInfo->iEnCodeIdx;
		pSession->iAreaID = pInfo->iSelectServerID;
		pSession->iClientType = pInfo->iClientType;
		pSession->uiCreateTick = _ExGetTickCount;
		pSession->bMasterIP = pInfo->bMasterIP;
		pSession->ucNetType = pInfo->ucNetType;
		{
			std::lock_guard<std::mutex> guard(m_SessionCS);
			m_SessionHash.Remove(pInfo->iSessionID);
			m_SessionHash.Add(pInfo->iSessionID, pSession);
		}

		TNextGateInfo ngInfo;
		int iAddr = 0;
		//--------------------
		//--------------------
		//--------------------
		//G_GGSocket.GetComfyGate(iAddr, GatePort, PInfo^.NetType); // 取可连接的Gate
		ngInfo.iSessionID = pInfo->iSessionID;
		ngInfo.iGateAddr = iAddr;
		SendToServerPeer(SM_SELECT_SERVER, iParam, (char*)&ngInfo, sizeof(TNextGateInfo));
	}
}

/************************End Of CDGClientSocket********************************************/