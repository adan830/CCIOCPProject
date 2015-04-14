/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接AuthenServer服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CASClientSocket.h"
#include "CGSServerSocket.h"

using namespace CC_UTILS;

CASClientSocket* pG_AuthenServer;

/************************Start Of CASClientSocket******************************************/

CASClientSocket::CASClientSocket() : m_uiCheckTick(0), m_iPingCount(0), m_EnCodeFunc(nullptr), m_DeCodeFunc(nullptr),
m_pEnBuf(nullptr), m_pDeBuf(nullptr), m_usEnBufLen(0), m_usDeBufLen(0), m_iWorkIndex(0)
{
	m_OnConnect = std::bind(&CASClientSocket::OnSocketConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CASClientSocket::OnSocketDisconnect, this, std::placeholders::_1);
	m_OnRead = std::bind(&CASClientSocket::OnSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_OnError = std::bind(&CASClientSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
}

CASClientSocket::~CASClientSocket()
{
	ClearEncodeBuf();
	ClearDecodeBuf();
}

void CASClientSocket::DoHeartbeat()
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

void CASClientSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{
	//------------------------
	//------------------------
	//------------------------
}

bool CASClientSocket::Closed()
{
	Close();
	return !IsConnected();
}

bool CASClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	bool bRetFlag = false;
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
			{
				memcpy(pData + sizeof(TServerSocketHeader), pBuf, usBufLen);
				if (m_EnCodeFunc != nullptr)
					m_EnCodeFunc((CC_UTILS::PBYTE)(&pData[sizeof(PServerSocketHeader)]), usBufLen);
			}
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

void CASClientSocket::ProcAuthenServerMessage(PInnerMsgNode pNode)
{
	switch (pNode->usIdent)
	{
	case SM_USER_AUTHEN_RES:
		//G_UserManage.AuthenResponse(Param, szBuf, wBufLen);
		break;
	case SM_USER_REGIST_RES:
		//G_UserManage.RegistResponse(Param, szBuf, wBufLen);
		break;
	case SM_SAFECARD_AUTHEN_RES:
		//G_UserManage.SafeCardResponse(Param, szBuf, wBufLen);
		break;
	case SM_KICKOUT_ACCOUNT:
		//G_UserManage.KickOut(szBuf);
		break;
	case SM_RECHARGE_DB_REQ:
		//YBRechargeResponse(Param, szBuf, wBufLen);		
		break;
	case SM_CHILD_ONLINE_TIME:
		//G_UserManage.UpdateChildTime(szBuf, wBufLen);
		break;
	case SM_GIVEITEM_DB_REQ:
		//GiveItemResponse(szBuf, wBufLen);
		break;
	case SM_GAME_ACT_CODE_REQ:
		//GameActCodeResponse(szBuf, wBufLen);
		break;
	default:
		Log("TAuthenSocket unknown Ident: " + std::to_string(pNode->usIdent), lmtWarning);
		break;
	}
}

void CASClientSocket::OnYBCrushRsp(const std::string &sOrderID, const std::string &sRoleName, int iRetCode)
{
	/*
var
  jsAck             : TlkJSONobject;
  Msg               : AnsiString;
begin
  jsAck := TlkJSONobject.Create;
  jsAck.Add('OrderId', sOrderID);
  jsAck.Add('RoleName', sRoleName);
  jsAck.Add('IP', GetInternetIP);
  jsAck.Add('State', RetCode);
  Msg := TlkJSON.GenerateText(jsAck);
  jsAck.Free;
  G_AuthenSocket.SendToServer(SM_RECHARGE_DB_ACK, 0, PAnsiChar(Msg), Length(Msg));
end;
	*/
}

void CASClientSocket::OnGiveItemRsp(const std::string &sOrderID, const std::string &sRoleName, int iRetCode)
{
	/*
var
  jsAck             : TlkJSONobject;
  Msg               : AnsiString;
begin
  jsAck := TlkJSONobject.Create;
  jsAck.Add('OrderId', sOrderID);
  jsAck.Add('RoleName', sRoleName);
  jsAck.Add('IP', GetInternetIP);
  jsAck.Add('State', RetCode);
  Msg := TlkJSON.GenerateText(jsAck);
  jsAck.Free;
  G_AuthenSocket.SendToServer(SM_GIVEITEM_DB_ACK, 0, PAnsiChar(Msg), Length(Msg));
end;
	*/
}

void CASClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	if ((m_DeCodeFunc != nullptr) && (iDataLen > 0))
		m_DeCodeFunc((CC_UTILS::PBYTE)pData, iDataLen);

	PInnerMsgNode pNode;
	switch (pHeader->usIdent)
	{
	case SM_PING:
		m_iPingCount = 0;
		if (0 == pHeader->iParam)
			SendRegisterServer();
		break;
	case SM_ENCODE_BUFFER:
		ClearEncodeBuf();
		if (iDataLen > 0)
		{
			m_usEnBufLen = iDataLen;
			m_pEnBuf = (char*)VirtualAlloc(nullptr, m_usEnBufLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			memcpy(m_pEnBuf, pData, iDataLen);
			m_EnCodeFunc = (PCodingFunc)m_pEnBuf;
		}
		break;
	case SM_DECODE_BUFFER:
		ClearDecodeBuf();
		if (iDataLen > 0)
		{
			m_usDeBufLen = iDataLen;
			m_pDeBuf = (char*)VirtualAlloc(nullptr, m_usDeBufLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			memcpy(m_pDeBuf, pData, iDataLen);
			m_DeCodeFunc = (PCodingFunc)m_pDeBuf;
		}
		break;
	default:
		pNode = new TInnerMsgNode();
		pNode->MsgFrom = fromAuthenServer;
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

void CASClientSocket::OnSocketConnect(void* Sender)
{
	ClearEncodeBuf();
	ClearDecodeBuf();
	Log("AuthenServer(" + m_Address + ") Connected.");
	SendRegisterServer();
}

void CASClientSocket::OnSocketDisconnect(void* Sender)
{
	Log("AuthenServer(" + m_Address + ") Disconnected.");
}

void CASClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{
	if (G_BoClose)
		return;
	//收到消息,ping计数重置
	m_iPingCount = 0;
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CASClientSocket Socket Read Error, Code = " + to_string(iErrorCode), lmtError);
}

void CASClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("CASClientSocket Socket Error, Code = " + std::to_string(iErrorCode), lmtError);
	iErrorCode = 0;
}

void CASClientSocket::Reconnect()
{
	if (!IsActive())
	{
		m_Address = m_ServerArray[m_iWorkIndex].IPAddress;
		m_Port = m_ServerArray[m_iWorkIndex].iPort;
		if (m_Port > 0)
		{
			Log("Connect to AuthenServer(" + m_Address + ").");
			Open();
		}
	}
	// 切换AuthenServer
	++m_iWorkIndex;
	if (m_iWorkIndex >= MAX_AUTHENSERVER_NUM)
		m_iWorkIndex = 0;
}

void CASClientSocket::SendHeartbeat()
{
	int iCount = 0;
	if (pG_GameServerSocket != nullptr)
		iCount = pG_GameServerSocket->HumanCount;
	SendToServerPeer(SM_PING, iCount, nullptr, 0);
}

void CASClientSocket::SendRegisterServer()
{
	SendToServerPeer(SM_REGISTER, G_ServerID, nullptr, 0);
}

void CASClientSocket::YBRechargeResponse(int iParam, char* pBuf, unsigned short usBufLen)
{
	/*
var
  js                : TlkJSONobject;
  s                 : AnsiString;
  CrushInfo         : TYBCrushInfo;
begin
  SetString(s, Buf, BufLen);
  js := TlkJSON.ParseText(s) as TlkJSONobject;
  if Assigned(js) then
  begin
    FillChar(CrushInfo, SizeOf(TYBCrushInfo), 0);
    with CrushInfo, js do
    begin
      s := clkJSON.GetStringValue(Field['ToAccount']);      // Int64 最大长度(10进制) = 20位
      StrPLCopy(szAccount, s, ACCOUNT_MAX_LEN - 1);
      s := clkJSON.GetStringValue(Field['OrderId']);
      StrPLCopy(szOrderID, s, ORDER_ID_MAX_LEN - 1);
      dwAreaId := getIntFromName('AreaId');
      nAmount := getIntFromName('ChargeAmount') * YB_RATION; // 充值元宝转成分宝数
      nBindAmount := getIntFromName('BindAmount') * YB_RATION; // 充值绑定元宝转成分宝数;
    end;
    js.Free;
    if G_HumanDB.ProcYBRecharge(@CrushInfo) then
      G_GSSocket.SendToGameServer(0, SM_YB_CRUSH, @CrushInfo, sizeof(TYBCrushInfo))
    else
    begin
      with CrushInfo do
        OnCrushRsp(szOrderID, '', ORDER_RSP_SERVER_CLOSE);  // 游戏服务器未开启
    end;
  end;
end;
	*/
}

void CASClientSocket::GiveItemResponse(char* pBuf, unsigned short usBufLen)
{
	/*
var
  js                : TlkJSONobject;
  s, Desc           : AnsiString;
  SendBuf           : PAnsiChar;
  P                 : PGiveItemInfo;
  SendLen, DescLen  : Word;
begin
  SetString(s, Buf, BufLen);
  js := TlkJSON.ParseText(s) as TlkJSONobject;
  if Assigned(js) then
  begin
    Desc := clkJSON.GetStringValue(js.Field['Description']);
    DescLen := Length(Desc);
    if DescLen > 0 then
      inc(DescLen);
    SendLen := SizeOf(TGiveItemInfo) + DescLen;
    SendBuf := AllocMem(SendLen);
    try
      P := PGiveItemInfo(SendBuf);
      with P^, js do
      begin
        StrPLCopy(szAccount, clkJSON.GetStringValue(Field['ToAccount']), ACCOUNT_MAX_LEN - 1);
        StrPLCopy(szOrderID, clkJSON.GetStringValue(Field['OrderId']), ORDER_ID_MAX_LEN - 1);
        dwAreaId := StrToIntDef(clkJSON.GetStringValue(Field['AreaId']), 0);
        nCount := StrToIntDef(clkJSON.GetStringValue(Field['ItemCount']), 0);
        StrPLCopy(szItemName, clkJSON.GetStringValue(Field['ItemName']), ITEM_NAME_MAX_LEN - 1);
        bBindFlag := StrToIntDef(clkJSON.GetStringValue(Field['BindFlag']), 0);
        bDescLen := DescLen;
        if bDescLen > 0 then
          StrPLCopy(@SendBuf[SizeOf(TGiveItemInfo)], Desc, bDescLen - 1);
      end;
      js.Free;
      if G_HumanDB.ProcGiveItem(P) then
        G_GSSocket.SendToGameServer(0, SM_GIVE_ITEM, SendBuf, SendLen)
      else
        OnGiveItemRsp(P^.szOrderID, P^.szRoleName, ORDER_RSP_SERVER_CLOSE);
    finally
      FreeMem(SendBuf);
    end;
  end;
end;
	*/
}

void CASClientSocket::GameActCodeResponse(char* pBuf, unsigned short usBufLen)
{
	/*
var
  s                 : AnsiString;
  js                : TlkJSONobject;
  Info              : TActCodeInfo;
begin
  SetString(s, Buf, BufLen);
  {$IFDEF TEST2}
  Log('返回GS激活码信息: '+s);
  {$ENDIF}
  js := TlkJSON.ParseText(s) as TlkJSONobject;
  if Assigned(js) then
  begin
    FillChar(Info, sizeof(TActCodeInfo), 0);
    with Info, js do
    begin
      StrPlCopy(szAccount, clkJSON.GetStringValue(Field['Account']), ACCOUNT_MAX_LEN - 1);
      StrPlCopy(szRoleName, clkJSON.GetStringValue(Field['RoleName']), ACTOR_NAME_MAX_LEN - 1);
      StrPlCopy(szRetInfo, clkJSON.GetStringValue(Field['retinfo']), 100 - 1);
      iRetCode := getIntFromName('retcode');
    end;
    G_GSSocket.SendToGameServer(0, SM_GAME_ACT_CODE_RES, @info, sizeof(TActCodeInfo));
  end;
end;
	*/
}

void CASClientSocket::ClearEncodeBuf()
{
	if (m_pEnBuf != nullptr)
	{
		if (m_pEnBuf != nullptr)
			VirtualFree(m_pEnBuf, m_usEnBufLen, MEM_DECOMMIT);
		m_pEnBuf = nullptr;
		m_usEnBufLen = 0;
	}
	m_EnCodeFunc = nullptr;
}

void CASClientSocket::ClearDecodeBuf()
{
	if (m_pDeBuf != nullptr)
	{
		if (m_pDeBuf != nullptr)
			VirtualFree(m_pDeBuf, m_usDeBufLen, MEM_DECOMMIT);
		m_pDeBuf = nullptr;
		m_usDeBufLen = 0;
	}
	m_DeCodeFunc = nullptr;
}

/************************End Of CASClientSocket********************************************/