/**************************************************************************************
@author: 陈昌
@content: AuthenServer对DB服务器连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CDBServerSocket.h"

using namespace CC_UTILS;

CDBServerSocket* pG_DBSocket;

const int DEFAULT_QUERY_DELAY = 20 * 1000;                          // 10 * 60 * 1000;
const int QUICK_QUERY_DELAY = 5 * 1000;

/************************Start Of CDBConnector******************************************/
CDBConnector::CDBConnector() :m_iServerID(0), m_iHumanCount(0), m_bCheckCredit(false), m_bCheckItem(false), m_EnCodeFunc(nullptr), m_DeCodeFunc(nullptr)
{}

CDBConnector::~CDBConnector()
{}

int CDBConnector::GetServerID()
{
	return m_iServerID;
}

int CDBConnector::GetHumanCount()
{
	return m_iHumanCount;
}

void CDBConnector::SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	int iDataLen = sizeof(TServerSocketHeader) + usBufLen;
	char* pData = (char*)malloc(iDataLen);
	if (pData != nullptr)
	{
		try
		{
			((PServerSocketHeader)pData)->ulSign = SS_SEGMENTATION_SIGN;
			((PServerSocketHeader)pData)->usIdent = usIdent;
			((PServerSocketHeader)pData)->iParam = iParam;
			((PServerSocketHeader)pData)->usBehindLen = usBufLen;
			if (usBufLen > 0)
			{
				memcpy(pData + sizeof(TServerSocketHeader), pBuf, usBufLen);
				if (m_EnCodeFunc != nullptr)
					m_EnCodeFunc((CC_UTILS::PBYTE)(&pData[sizeof(PServerSocketHeader)]), usBufLen);
			}

			SendBuf(pData, iDataLen);
			free(pData);
		}
		catch (...)
		{
			free(pData);
		}
	}
}

void CDBConnector::SendToClientPeer(unsigned short usIdent, int iParam, const std::string &str)
{
	if (str.length() > 0)
		SendToClientPeer(usIdent, iParam, const_cast<char*>(str.c_str()), str.length() + 1);
	else
		SendToClientPeer(usIdent, iParam, nullptr, 0);
}

void CDBConnector::Execute(unsigned long ulTick)
{
	if (m_bCheckCredit)
	{
		m_bCheckCredit = false;
		//G_ServerSocket.AddRechargeQueryJob(FServerIdx, SocketHandle);
	}
	if (m_bCheckItem)
	{
		m_bCheckItem = false;
		//G_ServerSocket.AddQueryGiveItemJob(FServerIdx, SocketHandle);
	}
}

void CDBConnector::SocketRead(const char* pBuf, int iCount)
{
	//在基类解析外层数据包，并调用ProcessReceiveMsg完成逻辑消息处理
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("TDBServer Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CDBConnector::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	//解密
	if ((m_DeCodeFunc != nullptr) && (iDataLen > 0))
		m_DeCodeFunc((CC_UTILS::PBYTE)pData, iDataLen);

	switch (pHeader->usIdent)
	{
	case SM_PING: 
		Msg_Ping(pHeader->iParam);
		break;
    case SM_REGISTER: 
		Msg_RegisterServer(pHeader->iParam);
		break;
	case SM_USER_AUTHEN_REQ:
		if (m_iServerID > 0)
			Msg_UserAuthenRequest(pHeader->iParam, pData, iDataLen);
		else
			OnAuthenFail(pHeader->iParam, 13, MSG_AUTHEN_ERROR, 0, 0);
		break;
	case SM_SAFECARD_AUTHEN_REQ:
		if (m_iServerID > 0)
			Msg_SafeCardAuthen(pHeader->iParam, pData, iDataLen);
		break;
	case SM_USER_REGIST_REQ:
		if (m_iServerID > 0)
			Msg_NewAccountRequest(pHeader->iParam, pData, iDataLen);
		break;
	case SM_RECHARGE_DB_ACK:
	case SM_GIVEITEM_DB_ACK:
		if (m_iServerID > 0)
			Msg_DBResponse(pHeader->usIdent, pHeader->iParam, pData, iDataLen);
		break;
	case SM_CHILD_LOGON:
		if (sizeof(TGameChildLogin) == iDataLen)
		{
			PGameChildLogin pLogin = (PGameChildLogin)pData;
			//--------------------------------------------
			//--------------------------------------------
			//--------------------------------------------
			//--------------------------------------------
			//G_ChildManager.Logon(pLogin->szCard_ID, pLogin->szRoleName, m_iServerID);
		}
		break;      
	case SM_CHILD_LOGOUT:
		if (sizeof(TGameChildLogin) == iDataLen)
		{
			PGameChildLogin pLogin = (PGameChildLogin)pData;
			//--------------------------------------------
			//--------------------------------------------
			//--------------------------------------------
			//--------------------------------------------
			//G_ChildManager.Logout(pLogin->szCard_ID, pLogin->szRoleName, m_iServerID);
		}
		break;
	case SM_REFRESH_RECHARGE:
		m_bCheckCredit = true;
		m_bCheckItem = true;
		break;
	default:
		break;
	}
}

void CDBConnector::InitDynCode()
{
	if ((m_EnCodeFunc != nullptr) || (m_DeCodeFunc != nullptr))
		return;

	PEnDeRecord p = CC_UTILS::GetCode();
	if (p != nullptr)
	{
		SendToClientPeer(SM_ENCODE_BUFFER, 0, p->pEnBuffer, p->usEnBufferLen);
		SendToClientPeer(SM_DECODE_BUFFER, 0, p->pDeBuffer, p->usDeBufferLen);
		m_EnCodeFunc = (CC_UTILS::PCodingFunc)p->pEnBuffer;
		m_DeCodeFunc = (CC_UTILS::PCodingFunc)p->pDeBuffer;
	}	
}

void CDBConnector::Msg_Ping(int iCount)
{
	m_iHumanCount = iCount;
	pG_DBSocket->ShowDBMsg(m_iServerID, 4, to_string(iCount));
	SendToClientPeer(SM_PING, m_iServerID, nullptr, 0);
}

void CDBConnector::Msg_RegisterServer(int iServerID)
{
	if (pG_DBSocket->RegisterDBServer(this, iServerID))
	{
		Log("DBServer " + to_string(iServerID) + " Enabled.");
		pG_DBSocket->ShowDBMsg(iServerID, 3, GetRemoteAddress());
		pG_DBSocket->ShowDBMsg(iServerID, 3, to_string(m_iHumanCount));
		m_iServerID = iServerID;
		InitDynCode();
	}
	else
	{
		Close();
	}
}

void CDBConnector::Msg_UserAuthenRequest(int iParam, char* pBuf, unsigned short usBufLen)
{
	int iAuthType = 0;
	int iAuthenApp = 0;
	int iRetCode = 0;
	std::string sMsg = MSG_AUTHEN_ERROR;

	Json::Reader reader;
	Json::Value root;
	try
	{
		//------------------------
		//------------------------
		//---------这样转换是否正确
		std::string str(pBuf, usBufLen);
		if (reader.parse(str, root))
		{
			iAuthType = root.get("AuthType", 0).asInt();
			iAuthenApp = root.get("AuthenApp", 0).asInt();
			std::string sAuthID;
			switch (iAuthType)
			{
			case 0:
				sAuthID = root.get("AuthenID", "").asString();
				if (VerifyPassport(sAuthID))
				{
					std::string sAuthIP = root.get("ClientIP", "").asString();
					std::string sMac = root.get("Mac", "").asString();
					std::string sAreaID = root.get("AreaID", "").asString();
					std::string sPwd = root.get("Pwd", "").asString();
					/*
					if G_AuthenSecure.LimitOfLogin(Auth_ID, Auth_IP, Mac) then
					begin
					  iRetCode := 7;
					  Msg := MSG_LOGIN_SECURE_FAILED;           // 1小时内的失败次数过多
					  G_AuthFailLog.WriteLog(Format('%s,%s,%s,%s,%s,%s,%d'#13#10, [FormatDateTime('yyyy-mm-dd hh:nn:ss', Now()), AreaID, Auth_ID, Auth_IP, Mac, Pwd, iRetCode]));
					end
					else
					begin
					  if G_SQLInterFace.AddJob(SM_USER_AUTHEN_REQ, SocketHandle, Param, Str) then
					  begin
						iRetCode := 1;
						Exit;                                   // 加入队列成功，暂时无出错返回
					  end
					  else
					  begin
						iRetCode := 9;
						Msg := MSG_SERVER_BUSY;
					  end;
					end;
					*/
				}
				else
				{
					iRetCode = 9;
					sMsg = MSG_AUTHEN_ERROR;
				}
				break;
			default:
				iRetCode = 8;
				sMsg = "认证类型错误:" + std::to_string(iAuthType);
				break;
			}
		}
		else
		{
			Log("不能识别的认证信息：" + str);
			iRetCode = 12;
			sMsg = MSG_AUTHEN_ERROR;
		}
	}
	catch (...)
	{
		//捕获异常不处理
	}

	if (iRetCode != 1)
		OnAuthenFail(iParam, iRetCode, sMsg, iAuthType, iAuthenApp);
}

void CDBConnector::Msg_NewAccountRequest(int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------这样转换是否正确
	std::string str(pBuf, usBufLen);
	Json::Reader reader;
	Json::FastWriter writer;
	Json::Value root;
	if (reader.parse(str, root))
	{
		std::string sIP = root.get("ClientIP", "").asString();
		/*
		if G_AuthenSecure.LimitOfRegister(IP) then            // 判断同IP注册数量限制
		begin
			js.Add('Result', 8);                                // 注册失败，稍候再试
			js.Add('Message', MSG_REGISTER_SECURE_FAILED);
			ResultStr := TlkJSON.GenerateText(js);
			SQLWorkCallBack(SM_USER_REGIST_RES, iParam, ResultStr);
		end
		else if not G_SQLInterFace.AddJob(SM_USER_REGIST_REQ, SocketHandle, Param, Str) then
		begin
			js.Add('Result', 9);                                // 队列满，失败返回
			js.Add('Message', MSG_SERVER_BUSY);
			ResultStr := TlkJSON.GenerateText(js);
			SQLWorkCallBack(SM_USER_REGIST_RES, iParam, ResultStr);
		end;
		*/
	}
	else
	{
		Log("不能识别的注册信息：" + str);
	}	
}

void CDBConnector::Msg_DBResponse(int iIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------这样转换是否正确
	std::string str(pBuf, usBufLen);
	switch (iIdent)
	{
	case SM_RECHARGE_DB_ACK:
		//G_RechargeManager.AddJob(Ident, SocketHandle, Param, str, True);
		break;
	case SM_GIVEITEM_DB_ACK:
		//G_GiveItemManager.AddJob(Ident, SocketHandle, Param, str, True);
		break;
	default:
		Log("Msg_DBResponse 未知协议：" + std::to_string(iIdent), lmtWarning);
		break;
	}
}

void CDBConnector::Msg_SafeCardAuthen(int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------这样转换是否正确
	std::string jsonStr(pBuf, usBufLen);
	Json::Reader reader;
	Json::FastWriter writer;
	Json::Value root;
	if (reader.parse(jsonStr, root))
	{
		std::string sCardNo = root.get("SafeCardNo", "").asString();
		std::string sIP = root.get("ClientIP", "").asString();
		/*
		if G_AuthenSecure.LimitOfSafeCard(CardNo, IP) then
		begin
		  js.Add('Result', 7);                                  // 1小时内的失败次数过多
		  js.Add('Message', MSG_SAFECARD_SECURE_FAILED);
		end
		else
		begin
		  if G_SQLInterFace.AddJob(SM_SAFECARD_AUTHEN_REQ, SocketHandle, Param, Buf) then
			Exit;
		  js.Add('Result', 9);                                  // 队列满，失败返回
		  js.Add('Message', MSG_SERVER_BUSY);
		end;
		*/
		std::string str = writer.write(root);
		SQLWorkCallBack(SM_SAFECARD_AUTHEN_RES, iParam, str);
	}
}

void CDBConnector::SQLWorkCallBack(int iCmd, int iParam, const std::string &str)
{
	SendToClientPeer(iCmd, iParam, str);
}

void CDBConnector::OnAuthenFail(int iSessionID, int iRetCode, const std::string &sMsg, int iAuthType, int iAuthenApp)
{
	//--------------------------
	//--------------------------
	//----这里的json组包是否ok？
	//--------------------------
	//--------------------------
	Json::Value root;
	root["Result"] = iRetCode;
	root["Message"] = sMsg;
	root["AuthType"] = iAuthType;
	root["AuthenApp"] = iAuthenApp;

	Json::FastWriter writer;
	std::string str = writer.write(root);
	SQLWorkCallBack(SM_USER_AUTHEN_RES, iSessionID, str);
}

/************************End Of CDBConnector******************************************/



/************************Start Of CDBServerSocket******************************************/

CDBServerSocket::CDBServerSocket(const std::string &sServerName) :m_sServerName("#" + sServerName + "#"), m_ServerHash(511), m_ulLastQueryRechargeTick(0),
m_ulQueryRechargeInterval(0), m_ulLastQueryItemTick(0), m_ulQueryItemInterval(0), m_ulLastCheckTick(0), m_iConfigFileAge(0), m_iGameID(0), m_sAllowDBServerIPs(""),
m_pLogSocket(nullptr), m_FirstNode(nullptr), m_LastNode(nullptr)
{
	SetMaxCorpseTime(60 * 1000);
	m_ServerHash.m_RemoveEvent = std::bind(&CDBServerSocket::RemoveServerConfig, this, std::placeholders::_1, std::placeholders::_2);

	m_OnCreateClient = std::bind(&CDBServerSocket::OnCreateDBSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CDBServerSocket::OnSocketError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CDBServerSocket::OnDBConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CDBServerSocket::OnDBDisconnect, this, std::placeholders::_1);
	m_OnCheckAddress = std::bind(&CDBServerSocket::OnCheckIPAddress, this, std::placeholders::_1);
	LoadConfig();
}

CDBServerSocket::~CDBServerSocket()
{
	m_ServerHash.m_RemoveEvent = nullptr;
	if (m_pLogSocket != nullptr)
		delete m_pLogSocket;
	Clear();
}

void CDBServerSocket::SQLJobResponse(int iCmd, int iHandle, int iParam, int iRes, const std::string &str)
{
	PJsonJobNode pNode = new TJsonJobNode;
	pNode->iCmd = iCmd;
	pNode->iHandle = iHandle;
	pNode->iParam = iParam;
	pNode->iRes = iRes;
	pNode->sJsonText = str;
	pNode->pNext = nullptr;
	{
		std::lock_guard<std::mutex> guard(m_LockJsonNodeCS);
		if (m_LastNode != nullptr)
			m_LastNode->pNext = pNode;
		else
			m_FirstNode = pNode;
		m_LastNode = pNode;
	}	
}

void CDBServerSocket::InCreditNow()
{
	m_ulQueryRechargeInterval = QUICK_QUERY_DELAY;
}

void CDBServerSocket::InSendItemNow()
{
	m_ulQueryItemInterval = QUICK_QUERY_DELAY;
}

void CDBServerSocket::BroadCastKickOutNow(const std::string &sAccount, int iParam)
{
	if (!IsTerminated())
	{
		std::string sSendAccount(sAccount, 0, 32);
		{
			std::lock_guard<std::mutex> guard(m_LockCS);
			std::list<void*>::iterator vIter;
			for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
				((CDBConnector*)(*vIter))->SendToClientPeer(SM_KICKOUT_ACCOUNT, 0, sSendAccount);				
		}
		//----------------------------------
		//----------------------------------
		//----------------------------------
		//G_IWebSocket.SendToServer(SM_KICKOUT_NOW, nParam, '1', 1);
	}
}

void CDBServerSocket::DoActive()
{
	LoadServerConfig();
	ProcResponseMsg();
	unsigned long tick = GetTickCount();
	//-----------------------------
	//-----------------------------
	//-----------------------------
	//if Assigned(G_RechargeManager) and G_RechargeManager.Enabled then
	if (true)
	{
		if (tick - m_ulLastQueryRechargeTick >= m_ulQueryRechargeInterval)
		{
			m_ulLastQueryRechargeTick = tick;
			m_ulQueryRechargeInterval = DEFAULT_QUERY_DELAY;
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				std::list<void*>::iterator vIter;
				for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
					((CDBConnector*)(*vIter))->m_bCheckCredit = true;
			}
		}
	}
	//------------------------------------
	//------------------------------------
	//------------------------------------
	//if Assigned(G_GiveItemManager) and G_GiveItemManager.Enabled then
	if (true)
	{
		if (tick - m_ulLastQueryItemTick >= m_ulQueryItemInterval)
		{
			m_ulLastQueryItemTick = tick;
			m_ulQueryItemInterval = DEFAULT_QUERY_DELAY;
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				std::list<void*>::iterator vIter;
				for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
					((CDBConnector*)(*vIter))->m_bCheckItem = true;
			}
		}
	}
}

bool CDBServerSocket::OnChildNotify(int iServerID, PGameChildInfo p)
{
	if (!IsTerminated())
	{
		std::lock_guard<std::mutex> guard(m_LockCS);
		std::list<void*>::iterator vIter;
		CDBConnector* pDBSock = nullptr;
		for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
		{
			pDBSock = (CDBConnector*)*vIter;
			if ((pDBSock != nullptr) && (pDBSock->m_iServerID == iServerID))
			{
				pDBSock->SendToClientPeer(SM_CHILD_ONLINE_TIME, 0, (char*)p, sizeof(TGameChildInfo));
				break;
			}
		}
	}
}

void CDBServerSocket::ProcResponseMsg()
{

}

void CDBServerSocket::Clear()
{
	PJsonJobNode pNode = nullptr;
	std::lock_guard<std::mutex> guard(m_LockJsonNodeCS);
	while (m_FirstNode != nullptr)
	{
		pNode = m_FirstNode;
		m_FirstNode = pNode->pNext;
		delete(pNode);		 
	}
	m_FirstNode = nullptr;
	m_LastNode = nullptr;
}

void CDBServerSocket::LoadServerConfig()
{
	unsigned long ulTick = GetTickCount();
	if ((0 == m_ulLastCheckTick) || (ulTick - m_ulLastCheckTick >= 5000))
	{
		m_ulLastCheckTick = ulTick;
		std::string sFileName(G_CurrentExeDir + "AreaConfig.json");
		int iAge = GetFileAge(sFileName);
		if ((iAge != -1) && (iAge != m_iConfigFileAge))
		{
			if (m_iConfigFileAge > 0)
				Log("AreaConfig.json is Reloaded.", lmtMessage);
			m_iConfigFileAge = iAge;

			//c++解析utf-8的json文件乱码，还是需要ascii
			ifstream configFile;
			std::string sJsonStr;
			std::string sTemp;
			configFile.open(sFileName);
			while (!configFile.eof())
			{
				configFile >> sTemp;
				sJsonStr.append(sTemp);
			}
			configFile.close();

			//sJsonStr = "[{\"AreaID\":1,\"AreaName\":\"神技测试服\",\"ServerID\":1,\"ServerIP\":\"192.168.1.2\"}]";
			Json::Reader reader;
			Json::Value root;
			if (reader.parse(sJsonStr, root))
			{
				if (root.isArray())
				{
					m_ServerHash.Clear();
					{
						std::string sAreaName;
						PServerConfigInfo pInfo;
						Json::Value item;
						std::lock_guard<std::mutex> guard(m_LockCS);
						for (int i = 0; i < root.size(); i++)
						{
							item = root.get(i, 0);
							sAreaName = item.get("AreaName", "").asString();
							if (m_ServerHash.ValueOf(sAreaName) == nullptr)
							{
								pInfo = new TServerConfigInfo;
								pInfo->sServerName = sAreaName;
								pInfo->iMaskServerID = item.get("AreaID", 0).asInt();
								pInfo->iRealServerID = item.get("ServerID", 0).asInt();
								pInfo->sServerIP = item.get("ServerIP", "").asString();
								pInfo->bDenyRecharge = (item.get("DenyRecharge", "").asString().compare("true") == 0);
								pInfo->bDenyGiveItem = (item.get("DenyGiveItem", "").asString().compare("true") == 0);								
								m_ServerHash.Add(sAreaName, pInfo);
								m_sAllowDBServerIPs.append(pInfo->sServerIP + "|");
								if (m_pLogSocket != nullptr)
								{
									std::string sStatus;
									if (pInfo->bDenyRecharge)
										sStatus = "充值关";
									else
										sStatus = "充值开";
									if (pInfo->bDenyGiveItem)
										sStatus = sTemp + "|道具关";
									else
										sStatus = sTemp + "|道具开";

									ShowDBMsg(pInfo->iRealServerID, 5, sStatus);
								}
							}
							else
								Log("区名重复: " + sAreaName, lmtError);
						}
					}
					if (nullptr == m_pLogSocket)
					{
						m_pLogSocket = new CC_UTILS::CLogSocket(m_sServerName, true);
						m_pLogSocket->m_OnConnectEvent = std::bind(&CDBServerSocket::OnSetListView, this, std::placeholders::_1);
						m_pLogSocket->m_OnDisConnectEvent = std::bind(&CDBServerSocket::OnLogSocketDisConnect, this, std::placeholders::_1);
						m_pLogSocket->InitialWorkThread();
					}
				}
			}
		}
	}
}

void CDBServerSocket::OnSetListView(void* Sender)
{
	Log("与 MonitorServer 连接成功");
	PServerConfigInfo pInfo = nullptr;
	std::string sTemp;
	TListViewInfo lvInfo;
	memset(&lvInfo, 0, sizeof(TListViewInfo));
	{
		std::lock_guard<std::mutex> guard(m_LockCS);
		m_ServerHash.First();
		while (!m_ServerHash.Eof())
		{
			pInfo = (PServerConfigInfo)m_ServerHash.GetNextNode();
			sTemp = std::to_string(pInfo->iMaskServerID);
			memcpy_s(lvInfo[0], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
			sTemp = pInfo->sServerName;
			memcpy_s(lvInfo[1], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
			sTemp = std::to_string(pInfo->iRealServerID);
			memcpy_s(lvInfo[2], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
			sTemp = OnLineDBServer(pInfo->iRealServerID);
			memcpy_s(lvInfo[3], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
			sTemp = "--";
			memcpy_s(lvInfo[4], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);

			std::string sStatus;
			if (pInfo->bDenyRecharge)
				sTemp = "充值关";
			else
				sTemp = "充值开";
			if (pInfo->bDenyGiveItem)
				sTemp = sTemp + "|道具关";
			else
				sTemp = sTemp + "|道具开";
			memcpy_s(lvInfo[5], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);

			if (m_pLogSocket != nullptr)
				m_pLogSocket->AddListView(&lvInfo);
		}
	}
	sTemp = "区号";
	memcpy_s(lvInfo[0], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	sTemp = "     区名     ";
	memcpy_s(lvInfo[1], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	sTemp = "服务器编号";
	memcpy_s(lvInfo[2], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	sTemp = "    连接地址    ";
	memcpy_s(lvInfo[3], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	sTemp = "  人数  ";
	memcpy_s(lvInfo[4], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	sTemp = "  状态  ";
	memcpy_s(lvInfo[5], sizeof(TShortValue), sTemp.c_str(), sTemp.length() + 1);
	if (m_pLogSocket != nullptr)
		m_pLogSocket->SetListViewColumns(&lvInfo);
}

void CDBServerSocket::OnLogSocketDisConnect(void* Sender)
{
	Log("与MonitoServer断开连接");
}

void CDBServerSocket::AddRechargeQueryJob(int iServerID, int iSocketHandle)
{
	PServerConfigInfo pInfo = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	m_ServerHash.First();
	while (!m_ServerHash.Eof())
	{
		pInfo = (PServerConfigInfo)m_ServerHash.GetNextNode();
		if ((pInfo->iRealServerID == iServerID) && (!pInfo->bDenyRecharge))
		{
			//--------------------------
			//--------------------------
			//----这里的json组包是否ok？
			//--------------------------
			//--------------------------
			Json::Value root;
			root["GameId"] = m_iGameID;
			root["AreaId"] = pInfo->iMaskServerID;
			Json::FastWriter writer;
			std::string str = writer.write(root);
			//------------------------------------
			//------------------------------------
			//------------------------------------
			//------------------------------------
			//G_RechargeManager.AddJob(SM_RECHARGE_AREA_QUERY, SocketHandle, 0, s);
		}
	}
}

void CDBServerSocket::AddQueryGiveItemJob(int iServerID, int iSocketHandle)
{
	PServerConfigInfo pInfo = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	m_ServerHash.First();
	while (!m_ServerHash.Eof())
	{
		pInfo = (PServerConfigInfo)m_ServerHash.GetNextNode();
		if ((pInfo->iRealServerID == iServerID) && (!pInfo->bDenyGiveItem))
		{
			//--------------------------
			//--------------------------
			//----这里的json组包是否ok？
			//--------------------------
			//--------------------------
			Json::Value root;
			root["GameId"] = m_iGameID;
			root["AreaId"] = pInfo->iMaskServerID;
			Json::FastWriter writer;
			std::string str = writer.write(root);
			//------------------------------------
			//------------------------------------
			//------------------------------------
			//------------------------------------
			//G_GiveItemManager.AddJob(SM_GIVEITEM_QUERY, SocketHandle, 0, s);
		}
	}
}

void CDBServerSocket::RemoveServerConfig(void* pValue, const std::string &sKey)
{
	delete (PServerConfigInfo)pValue;
}

bool CDBServerSocket::OnCheckIPAddress(const std::string &sConnectIP)
{
	bool bRetFlag = (m_sAllowDBServerIPs.find(sConnectIP) != string::npos);
	if (!bRetFlag)
		Log(sConnectIP + " 连接被禁止！");
	return bRetFlag;
}

void CDBServerSocket::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	int iPort = pIniFileParser->getInteger("Setup", "Port", DEFAULT_AuthenServer_PORT);
	m_iGameID = pIniFileParser->getInteger("Setup", "GameId", G_GAME_ID);
	delete pIniFileParser;

	if (!IsActive())
	{
		m_sLocalIP = "0.0.0.0";
		m_iListenPort = iPort;
		//在CMainThread::DoExecute里面启动再调用Open()
	}
}

void CDBServerSocket::OnSocketError(void* Sender, int& iErrorCode)
{
	Log("Server Socket Error, Code = " + to_string(iErrorCode), lmtError);
	iErrorCode = 0;
}

CClientConnector* CDBServerSocket::OnCreateDBSocket(const std::string &sIP)
{
	return new CDBConnector;
}

void CDBServerSocket::OnDBConnect(void* Sender)
{
	CDBConnector* pDBConnector = (CDBConnector*)Sender;
	Log("DBServer " + pDBConnector->GetRemoteAddress() + " Connected.");
	InCreditNow();
	InSendItemNow();
}

void CDBServerSocket::OnDBDisconnect(void* Sender)
{
	CDBConnector* pDBConnector = (CDBConnector*)Sender;
	int iServerID = pDBConnector->GetServerID();
	if (iServerID > 0)
	{
		Log("DBServer(" + std::to_string(pDBConnector->m_iServerID) + ")(" + pDBConnector->GetRemoteAddress() + ") DisConnected.");
		ShowDBMsg(iServerID, 3, "--");
		ShowDBMsg(iServerID, 4, "--");
	}
}

bool CDBServerSocket::RegisterDBServer(CDBConnector* Socket, int iServerID)
{
	bool bRetFlag = false;
	PServerConfigInfo pInfo = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	m_ServerHash.First();
	while (!m_ServerHash.Eof())
	{
		pInfo = (PServerConfigInfo)m_ServerHash.GetNextNode();
		if (pInfo->iRealServerID == iServerID)
		{
			if (Socket->GetRemoteAddress().find(pInfo->sServerIP) == string::npos)
				Log("DBServer IP地址不正确: " + std::to_string(pInfo->iRealServerID) + "(" + pInfo->sServerName + ")(" + Socket->GetRemoteAddress() + ")");
			bRetFlag = true;
			break;
		}
	}
	return bRetFlag;
}

std::string CDBServerSocket::OnLineDBServer(int iServerID)
{
	std::string sRetStr("--");
	CDBConnector* pDBConnector = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		pDBConnector = (CDBConnector*)*vIter;
		if ((pDBConnector != nullptr) && (iServerID == pDBConnector->GetServerID()))
		{
			sRetStr = pDBConnector->GetRemoteAddress();
			break;
		}
	}
	return sRetStr;
}

void CDBServerSocket::ShowDBMsg(int iServerID, int iCol, const std::string &sMsg)
{
	int iRow = 1;
	PServerConfigInfo pInfo = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	m_ServerHash.First();
	while (!m_ServerHash.Eof())
	{
		pInfo = (PServerConfigInfo)m_ServerHash.GetNextNode();
		if (pInfo->iRealServerID == iServerID)
		{
			if (m_pLogSocket != nullptr)
				m_pLogSocket->UpdateListView(sMsg, iRow, iCol);
			break;
		}
		++iRow;
	}
}

void CDBServerSocket::RechargeFail(const std::string &sOrderID)
{
	//--------------------------
	//--------------------------
	//----这里的json组包是否ok？
	//--------------------------
	//--------------------------
	Json::Value root;
	root["Orderid"] = sOrderID;
	root["IP"] = "127.0.0.1";
	root["State"] = -1;
	Json::FastWriter writer;
	std::string str = writer.write(root);
	//----------------------------------
	//----------------------------------
	//----------------------------------
	//G_RechargeManager.AddJob(SM_RECHARGE_DB_ACK, 0, 0, s, True);
}

/************************End Of CDBServerSocket******************************************/