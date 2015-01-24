/**************************************************************************************
@author: �²�
@content: AuthenServer��DB���������ӵļ���socket����
**************************************************************************************/
#include "stdafx.h"
#include "CDBServerSocket.h"

using namespace CC_UTILS;

CDBServerSocket* pG_DBSocket;

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
	//�ڻ������������ݰ���������ProcessReceiveMsg����߼���Ϣ����
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("TDBServer Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CDBConnector::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{
	//����
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
		//---------����ת���Ƿ���ȷ
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
					  Msg := MSG_LOGIN_SECURE_FAILED;           // 1Сʱ�ڵ�ʧ�ܴ�������
					  G_AuthFailLog.WriteLog(Format('%s,%s,%s,%s,%s,%s,%d'#13#10, [FormatDateTime('yyyy-mm-dd hh:nn:ss', Now()), AreaID, Auth_ID, Auth_IP, Mac, Pwd, iRetCode]));
					end
					else
					begin
					  if G_SQLInterFace.AddJob(SM_USER_AUTHEN_REQ, SocketHandle, Param, Str) then
					  begin
						iRetCode := 1;
						Exit;                                   // ������гɹ�����ʱ�޳�����
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
				sMsg = "��֤���ʹ���:" + std::to_string(iAuthType);
				break;
			}
		}
		else
		{
			Log("����ʶ�����֤��Ϣ��" + str);
			iRetCode = 12;
			sMsg = MSG_AUTHEN_ERROR;
		}
	}
	catch (...)
	{
		//�����쳣������
	}

	if (iRetCode != 1)
		OnAuthenFail(iParam, iRetCode, sMsg, iAuthType, iAuthenApp);
}

void CDBConnector::Msg_NewAccountRequest(int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------����ת���Ƿ���ȷ
	std::string str(pBuf, usBufLen);
	Json::Reader reader;
	Json::FastWriter writer;
	Json::Value root;
	if (reader.parse(str, root))
	{
		std::string sIP = root.get("ClientIP", "").asString();
		/*
		if G_AuthenSecure.LimitOfRegister(IP) then            // �ж�ͬIPע����������
		begin
			js.Add('Result', 8);                                // ע��ʧ�ܣ��Ժ�����
			js.Add('Message', MSG_REGISTER_SECURE_FAILED);
			ResultStr := TlkJSON.GenerateText(js);
			SQLWorkCallBack(SM_USER_REGIST_RES, iParam, ResultStr);
		end
		else if not G_SQLInterFace.AddJob(SM_USER_REGIST_REQ, SocketHandle, Param, Str) then
		begin
			js.Add('Result', 9);                                // ��������ʧ�ܷ���
			js.Add('Message', MSG_SERVER_BUSY);
			ResultStr := TlkJSON.GenerateText(js);
			SQLWorkCallBack(SM_USER_REGIST_RES, iParam, ResultStr);
		end;
		*/
	}
	else
	{
		Log("����ʶ���ע����Ϣ��" + str);
	}	
}

void CDBConnector::Msg_DBResponse(int iIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------����ת���Ƿ���ȷ
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
		Log("Msg_DBResponse δ֪Э�飺" + std::to_string(iIdent), lmtWarning);
		break;
	}
}

void CDBConnector::Msg_SafeCardAuthen(int iParam, char* pBuf, unsigned short usBufLen)
{
	//------------------------
	//------------------------
	//---------����ת���Ƿ���ȷ
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
		  js.Add('Result', 7);                                  // 1Сʱ�ڵ�ʧ�ܴ�������
		  js.Add('Message', MSG_SAFECARD_SECURE_FAILED);
		end
		else
		begin
		  if G_SQLInterFace.AddJob(SM_SAFECARD_AUTHEN_REQ, SocketHandle, Param, Buf) then
			Exit;
		  js.Add('Result', 9);                                  // ��������ʧ�ܷ���
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
	//----�����json����Ƿ�ok��
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

CDBServerSocket::CDBServerSocket(const std::string &sServerName)
{

}

CDBServerSocket::~CDBServerSocket()
{

}

void CDBServerSocket::SQLJobResponse(int iCmd, int iHandle, int iParam, int iRes, const std::string &str)
{}

void CDBServerSocket::InCreditNow()
{}

void CDBServerSocket::InSendItemNow()
{}

void CDBServerSocket::BroadCastKickOutNow(const std::string &sAccount, int iParam)
{}

void CDBServerSocket::DoActive()
{}

bool CDBServerSocket::OnChildNotify(int iServerID, PGameChildInfo p)
{}

void CDBServerSocket::ProcResponseMsg()
{}

void CDBServerSocket::Clear()
{}

void CDBServerSocket::LoadServerConfig()
{}

void CDBServerSocket::OnSetListView(void* Sender)
{}

void CDBServerSocket::OnLogSocketDisConnect(void* Sender)
{}

void CDBServerSocket::AddRechargeQueryJob(int iServerID, int iSocketHandle)
{}

void CDBServerSocket::AddQueryGiveItemJob(int iServerID, int iSocketHandle)
{}

void CDBServerSocket::RemoveServerConfig(void* pValue, const std::string &sKey)
{}

bool CDBServerSocket::CheckConnectIP(const std::string &sConnectIP)
{}

void CDBServerSocket::LoadConfig()
{}

void CDBServerSocket::SocketError(void* Sender, int iErrorCode)
{}

CClientConnector CDBServerSocket::CreateCustomSocket(const std::string &sIP)
{}

void CDBServerSocket::DBConnect(void* Sender)
{}

void CDBServerSocket::DBDisConnect(void* Sender)
{}

bool CDBServerSocket::RegisterDBServer(CDBConnector* Socket, int iServerID)
{}

std::string CDBServerSocket::OnLineDBServer(int iServerID)
{}

void CDBServerSocket::ShowDBMsg(int iServerID, int iCol, const std::string &sMsg)
{}

void CDBServerSocket::RechargeFail(const std::string &sOrderID)
{}

/************************End Of CDBServerSocket******************************************/