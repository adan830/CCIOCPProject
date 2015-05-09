/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameServer的连接
**************************************************************************************/
#include "stdafx.h"
#include "CGSServerSocket.h"
#include "CGGServerSocket.h"
#include "CASClientSocket.h"
#include "CHumanDBManager.h"

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
	m_iMaxOnlineCount = pIniFileParser->getInteger("Setup", "MaxCount", 10000);
	m_sAllowIPs = pIniFileParser->getString("GameServer", "AllowIP", "127.0.0.1|");
	std::string sServer = pIniFileParser->getString("GameServer", "Address", "127.0.0.1");
	int iPort = 0;
	if (sServer != "")
	{
		iPort = DEFAULT_GameServer_PORT;

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
			iPort = StrToIntDef(sPort, DEFAULT_GameServer_PORT);
		}
		
		if (iPort > 0)
		{
			memcpy_s(m_ServerInfo.IPAddress, IP_ADDRESS_MAX_LEN + 1, sIP.c_str(), IP_ADDRESS_MAX_LEN + 1);
			m_ServerInfo.iPort = iPort;
		}
	}

	iPort = pIniFileParser->getInteger("GameServer", "ListenPort", DEFAULT_DBServer_GS_PORT);
	if (!IsActive())
	{
		m_sLocalIP = "0.0.0.0";
		m_iListenPort = iPort;
		Open();
		Log("GameServer Service Start.(Port: " + std::to_string(iPort) + ")");
	}
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
{
	std::string sRoleName("");
	std::string sCmdStr("");
	switch (pNode->usIdent)
	{
	case SM_SHUTDOWN:
		GameServerShutDown();
		break;
	case SM_PLAYER_DISCONNECT:
		//-----------------------
		//-----------------------
		//-----------------------
		//G_UserManage.RemoveUser(Param);
		break;
	case SM_PLAYER_DATA_READ:
		Msg_DataRead(pNode->iParam, pNode->pBuf, pNode->usBufLen);
		break;
	case SM_PLAYER_DATA_WRITE:
		Msg_DataWrite(pNode->iParam, pNode->pBuf, pNode->usBufLen);
		break;
	case SM_YB_CRUSH_RSP:
		if (pNode->usBufLen >= sizeof(TYBCrushInfoRsp))
		{
			PYBCrushInfoRsp pInfo = (PYBCrushInfoRsp)(pNode->pBuf);
			pG_AuthenServerSocket->OnYBCrushRsp(pInfo->szOrderID, pInfo->szRoleName, pInfo->iRetCode);
		}
		break;
	case SM_GIVE_ITEM_RSP:
		if (pNode->usBufLen >= sizeof(TYBCrushInfoRsp))
		{
			PYBCrushInfoRsp pInfo = (PYBCrushInfoRsp)(pNode->pBuf);
			pG_AuthenServerSocket->OnGiveItemRsp(pInfo->szOrderID, pInfo->szRoleName, pInfo->iRetCode);
		}
		break;
	case SM_PLAYER_DENYLOGIN:
		/*
		if wBufLen >= SizeOf(TDenyLoginRole) then
			G_DBSecurity.AddDenyRole(PDenyLoginRole(szBuf)^.RoleName, PDenyLoginRole(szBuf)^.KickTick);
		*/
		break;
	case SM_PLAYER_DATA_UNLOCK:
		if ((pNode->pBuf != nullptr) && (pNode->usBufLen > 0))
		{
			/*
			SetString(RoleName, szBuf, wBufLen);
			G_HumanDB.DBPlayer_UnLock(Param, RoleName);
			*/
		}
		break;
	case SM_PLAYER_ONLINE:
		if ((pNode->pBuf != nullptr) && (pNode->usBufLen > 0))
		{
			/*
			SetString(RoleName, szBuf, wBufLen);
			G_UserManage.PlayerOnLine(RoleName, Param);
			*/
		}
		break;
	case SM_PLAYER_OFFLINE:
		if ((pNode->pBuf != nullptr) && (pNode->usBufLen > 0))
		{
			/*
			SetString(RoleName, szBuf, wBufLen);
			G_UserManage.PlayerOffLine(RoleName);
			*/
		}
		break;
	case SM_SET_ONLINE_LIMIT:
		if ((pNode->pBuf != nullptr) && (sizeof(TRoleInfo) == pNode->usBufLen))
		{
			PRoleInfo pInfo = (PRoleInfo)(pNode->pBuf);
			m_iMaxOnlineCount = pInfo->iFlag;
			if (m_iMaxOnlineCount < 1)
				m_iMaxOnlineCount = 1;
			Log(CC_UTILS::FormatStr("%s 设置人数上限：%d", pInfo->szRoleName, m_iMaxOnlineCount), lmtWarning);
			//---------------------------
			//---------------------------
			//---------------------------
			//pInfo->iFlag = G_UserManage.QueueCount;
			SendToGameServer(pNode->iParam, SM_SET_ONLINE_LIMIT, pNode->pBuf, pNode->usBufLen);
		}
		break;
    case SM_DBSERVER_GMCMD:
		if ((pNode->pBuf != nullptr) && (sizeof(TRoleInfo) == pNode->usBufLen))
		{
			//-------------------------------
			//-------------------------------
			//-------------------------------
			//SetString(sCmdStr, szBuf, wBufLen);
			//sCmdStr: = PAnsiChar(sCmdStr);
			sCmdStr.assign(pNode->pBuf, pNode->usBufLen);
			ProcGMCmd(pNode->iParam, sCmdStr);
		}
		break;
    case SM_PLAYER_RENAME:   
		//---------------------------------
		//---------------------------------
		//G_UserManage.PlayerReName(szBuf, wBufLen, Param);
		break;
    case SM_GAME_ACT_CODE_REQ:   
		Msg_GameActCode(pNode->iParam, pNode->pBuf, pNode->usBufLen);
		break;
	default:
		break;
	}
}

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

bool CGSServerSocket::CanClosed()
{
	bool bRetFlag = false;
	if (m_bShutDown && ((m_uiShutDownTick > 0) && (_ExGetTickCount > m_uiShutDownTick + 1000)))
		bRetFlag = true;

	return bRetFlag;
}

int CGSServerSocket::GetHumanCount()
{
	int iHumCount = 0;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	CGSConnector* gs = nullptr;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gs = (CGSConnector*)*vIter;
		if (gs->IsEnable())
			iHumCount += gs->m_iOnlineCount;
	}
	return iHumCount;
}

CGSConnector* CGSServerSocket::GetActiveGameServer()
{
	CGSConnector* gs = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		gs = (CGSConnector*)*vIter;
		if (gs->IsEnable())
			break;
		else
			gs = nullptr;
	}
	return gs;
}

bool CGSServerSocket::RegisterGameServer(const std::string &sGSAddr, int iGSPort)
{
	return ((iGSPort == m_ServerInfo.iPort) && (sGSAddr.compare(m_ServerInfo.IPAddress) == 0));
}

void CGSServerSocket::Msg_DataRead(int iSessionID, char* pBuf, unsigned short usBufLen)
{
	if (sizeof(TRoleDetail) == usBufLen)
	{
		PRoleDetail pInfo = (PRoleDetail)pBuf;
		TDBDetailRec dbDetail;
		memset(&dbDetail, 0, sizeof(TDBDetailRec));
		//---------------------------------
		int iRef = 0;		
		//int iRef : = G_HumanDB.DBPlayer_Read(SessionID, P^.DBIndex, P^.bJob, @DBDetail);
		if (1 == iRef)
		{
			std::string sTempAccount(pInfo->szAccount);
			std::string sTempRoleName(pInfo->szRoleName);
			if ((pInfo->iAreaID == dbDetail.RoleInfo.iAreaID) && (sTempAccount.compare(dbDetail.RoleInfo.szAccount) == 0) &&
				(sTempRoleName.compare(dbDetail.RoleInfo.szRoleName) == 0))
			{
				memset(m_pSendCache, 0, sizeof(TSavePlayerRec));
				((PSavePlayerRec)m_pSendCache)->Detail = dbDetail.RoleInfo;
				int iSendLen = sizeof(TSavePlayerRec);
				((PSavePlayerRec)m_pSendCache)->usShareBlobLen = dbDetail.usShareBlobSize;
				if ((dbDetail.pShareBlobData != nullptr) && (dbDetail.usShareBlobSize > 0))
					memcpy_s(m_pSendCache+iSendLen, MAXWORD-iSendLen, dbDetail.pShareBlobData, dbDetail.usShareBlobSize);
				iSendLen += sizeof(dbDetail.usShareBlobSize);

				PJobDataRec pJobData = &(dbDetail.JobDataList[pInfo->ucJob]);
				if ((pJobData->pBlobData != nullptr) && (pJobData->usBlobSize > 0))
				{
					memcpy_s(m_pSendCache + iSendLen, MAXWORD - iSendLen, pJobData->pBlobData, pJobData->usBlobSize);
					((PSavePlayerRec)m_pSendCache)->usJobBlobLen = pJobData->usBlobSize;
					iSendLen += pJobData->usBlobSize;
				}
				SendToGameServer(iSessionID, SM_PLAYER_DATA_BACK, m_pSendCache, iSendLen);
				return;
			}
			else
			{
				Log(CC_UTILS::FormatStr("Msg_DataRead 数据不一致: %s/%s %s/%s %d/%d - %d",
				sTempAccount, pInfo->szAccount, sTempRoleName, pInfo->szRoleName, dbDetail.RoleInfo.iAreaID, pInfo->iAreaID, iRef), lmtError);
				iRef = 4;                 // 所指的DBIndex与角色名不匹配
			}
		}

		//处理读取错误
		TReadPlayerDataErr readErr;
		memset(&readErr, 0, sizeof(TReadPlayerDataErr));
		readErr.iErrCode = iRef;
		readErr.iDBIdx = pInfo->iDBIndex;
		memcpy_s(readErr.szRoleName, ACTOR_NAME_MAX_LEN, pInfo->szRoleName, ACTOR_NAME_MAX_LEN);
		SendToGameServer(iSessionID, SM_PLAYER_DATA_BACK, (char*)&readErr, sizeof(TReadPlayerDataErr));
		/*
		//--------------------------------------------
		//--------------------------------------------
		//--------------------------------------------
      User := G_UserManage.FindUser(SessionID);
      if Assigned(User) then
      begin
        case iRef of
          2: HintMsg := '您的角色被锁，请稍候再试';
          3: HintMsg := '您的角色数据读取错误，请稍候再试';
          4: HintMsg := '您所选区组不正确';
        end;
        User.OpenWindow(cwMessageBox, 0, HintMsg);
      end;
		*/
	}
}

void CGSServerSocket::Msg_DataWrite(int iSessionID, char* pBuf, unsigned short usBufLen)
{
	if (usBufLen > sizeof(TSavePlayerRec))
		;
	    //--------------------------------------
		//--------------------------------------
		//G_HumanDB.DBPlayer_Save(SessionID, buf, bufLen);
}

void CGSServerSocket::Msg_GameActCode(int iSessionID, char* pBuf, unsigned short usBufLen)
{
	if (usBufLen >= sizeof(TActCodeInfo))
	{
		PActCodeInfo pInfo = (PActCodeInfo)pBuf;
		Json::FastWriter writer;
		Json::Value root;
		root["UniqueID"] = pInfo->szAccount;
		root["RoleName"] = pInfo->szRoleName;
		root["Code"] = pInfo->szOrderID;
		root["ServerID"] = pInfo->uiAreaID;
		std::string sJsonStr = writer.write(root);
		pG_AuthenServerSocket->SendToServerPeer(SM_GAME_ACT_CODE_REQ, iSessionID, const_cast<char*>(sJsonStr.c_str()), sJsonStr.length());
	}
}

bool CGSServerSocket::OnCheckConnectIP(const std::string &sConnectIP)
{
	bool bRetFlag = ((sConnectIP.find(m_sAllowIPs) != std::string::npos) || (sConnectIP.compare(m_ServerInfo.IPAddress) == 0));
	if (!bRetFlag)
		Log(CC_UTILS::FormatStr("GameServer[%s] 连接被禁止！！", sConnectIP), lmtWarning);
	return bRetFlag;
}

void CGSServerSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log(CC_UTILS::FormatStr("CGSServerSocket Socket Error, Code = %d", iErrorCode), lmtWarning);
	iErrorCode = 0;
}

CClientConnector* CGSServerSocket::OnCreateGSSocket(const std::string &sIP)
{
	return new CGSConnector();
}

void CGSServerSocket::OnGSConnect(void* Sender)
{
	Log(CC_UTILS::FormatStr("GameServer %s Connected.", ((CGGConnector*)Sender)->GetRemoteAddress()), lmtWarning);
	m_bShutDown = false;
	m_uiShutDownTick = 0;
}

void CGSServerSocket::OnGSDisconnect(void* Sender)
{
	Log("GameServer Disconnected.", lmtWarning);
	((CGSConnector*)Sender)->m_bEnable = false;
}

void CGSServerSocket::ProcGMCmd(int iSessionID, std::string &sCmdStr)
{
	/*
var
  Par1, Par2, Par3, TempStr: AnsiString;
  User              : TUser;
  Hint              : AnsiString;
  nParam, nTimeOut  : Integer;
begin
  CmdStr := GetValidStr(CmdStr, Par1, ' ');
  CmdStr := GetValidStr(CmdStr, Par2, ' ');
  Par3 := CmdStr;
  if CompareText(Par1, 'MaxLimit') = 0 then
  begin
    FMaxOnline := StrToIntDef(Par2, MAX_QUEUE_COUNT);
    if FMaxOnline < 1 then
      FMaxOnline := 1;
    Hint := Format('设置人数上限：%d 当前排队：%d', [FMaxOnLine, G_UserManage.QueueCount]);
  end
  else if CompareText(Par1, 'DenyRecharge') = 0 then
  begin
    G_MainThread.DenyRecharge := (CompareText(Par2, 'true') = 0);
    if G_MainThread.DenyRecharge then
      Hint := '充值、送道具通道关闭'
    else
      Hint := '充值、送道具通道开启';
  end
  else if CompareText(Par1, 'BanIP') = 0 then
  begin
    if Par2 = '' then
      Exit;
    if CompareText(Par2, 'Look') = 0 then
    begin
      Hint := G_DBSecurity.GetBadIPInfo;
    end
    else
    begin
      nParam := StrToIntDef(Par3, KICKOUT_TIMEOUT);
      G_DBSecurity.AddBandIP(inet_addr(PAnsiChar(Par2)), nParam);
      Hint := Format('IP:%s %d秒内被禁止登陆', [Par2, nParam]);
    end;
  end
  else if CompareText(Par1, 'KickOut') = 0 then
  begin
    if Par2 = '' then
      Exit;
    if CompareText(Par2, 'Look') = 0 then
    begin
      Hint := G_DBSecurity.GetDenyRoleInfo;
    end
    else
    begin
      nParam := StrToIntDef(Par3, KICKOUT_TIMEOUT);
      G_DBSecurity.AddDenyRole(Par2, nParam);
      Hint := Format('%s %d秒内被禁止登陆', [Par2, nParam]);
    end;
  end
  else if CompareText(Par1, 'Login') = 0 then
  begin
    if (Par2 <> '') and (Par3 <> '') then
    begin
      TempStr := Trim(Par3);
      TempStr := Trim(GetValidStr(TempStr, Par3, ' '));
      nTimeOut := StrToIntDef(TempStr, TEMP_LOGIN_TIMEOUT);
      G_UserManage.AddTempLogin(Par3, MD5Print(MD5String(LowerCase(Par2)), True), nTimeOut);
      Hint := Format('成功添加临时登录账号%s，%d秒后过期', [Par3, nTimeOut]);
    end
    else
      Hint := '@DBServer Login 临时密码 角色名 [有效秒数]'
  end
  else if CompareText(Par1, 'rename') = 0 then
  begin
    if (Par2 <> '') and (Par3 <> '') then
    begin
      if CompareText(Par3, 'look') = 0 then
      begin
        if G_HumanDB.IsNeedReName(Par2, TempStr) then
          Hint := Par2 + '已申请更名，原角色名为：' + TempStr
        else
          Hint := Par2 + '未申请更名'
      end
      else if CompareText(Par3, 'add') = 0 then
      begin
        if G_HumanDB.IsNeedReName(Par2, TempStr) then
          Hint := Par2 + '已申请更名，原角色名为：' + TempStr
        else if G_HumanDB.AddReNameToList(Par2, Par2, G_ServerID) then
          Hint := Par2 + '申请更名成功'
        else
          Hint := Par2 + ' 申请更名失败';
      end
      else if CompareText(Par3, 'del') = 0 then
      begin
        if G_HumanDB.DelReNameFromList(Par2) then
          Hint := Format('成功取消%s更名申请', [Par2])
        else
          Hint := Format('取消%s更名申请失败', [Par2]);
      end;
    end
    else
      Hint := '@DBServer rename 角色名 add[del|look]'
  end
  else
    G_DispatchGate.ProcGMCmd(SessionID, Par1, Par2, Par3);
  if Hint <> '' then
  begin
    User := G_UserManage.FindUser(SessionID);
    if Assigned(User) then
      User.SendMsg(Hint);
  end;
end;
	*/
}


/************************End Of CGSServerSocket******************************************/