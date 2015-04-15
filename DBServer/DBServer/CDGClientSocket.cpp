/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ�ͻ��˷�����DispatchGate�������Ķ˿�
**************************************************************************************/
#include "stdafx.h"
#include "CDGClientSocket.h"
#include "CGSServerSocket.h"

using namespace CC_UTILS;

CDGClientSocket* pG_DispatchGate;

#ifdef TEST
const int MAX_SESSION_TIMEOUT = 30 * 1000;                          // 30��SessionID��Чʱ��
#else
const int MAX_SESSION_TIMEOUT = 10 * 1000;                          // 10��SessionID��Чʱ��
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
	/*
  if FTraceList.Count > 0 then
    Result := (FTraceList.IndexOf(LowerCase(RoleName)) > -1)
  else
    Result := False;
	*/
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
		MsgSelectServer(pNode->iParam, pNode->pBuf, pNode->usBufLen);  //iParam�ǿͻ�����DispatchGate��SocketHandle 		
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
	//�յ���Ϣ,ping��������
	m_iPingCount = 0;
	//�ڻ������������ݰ���������ProcessReceiveMsg����߼���Ϣ����
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
	// �л�Dispatch Gate
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

}

void CDGClientSocket::SendConfig(const std::string &sKey, std::string &sValue)
{

}

bool CDGClientSocket::SetConfig(const std::string &sKey, std::string &sValue, bool bDel)
{

}

std::string CDGClientSocket::GetConfigInfo(const std::string &sKey)
{

}

void CDGClientSocket::SendRegisterServer()
{
	SendToServerPeer(SM_REGISTER, G_ServerID, nullptr, 0);
}

void CDGClientSocket::MsgSelectServer(int iParam, char* pBuf, unsigned short usBufLen)
{
	/*
var
  PSI               : PSessionInfo;
  PInfo             : PClientInfo absolute Buf;
  NextGateInfo      : TNextGateInfo;
  iAddr             : integer;
begin
  if BufLen = sizeof(TClientInfo) then
  begin
    with PInfo^ do
    begin
      New(PSI);
      FillChar(PSI^, sizeof(TSessionInfo), 0);
      PSI^.EnCodeIdx := iEnCodeIdx;                         // �ӽ���Key
      PSI^.AreaID := iAreaID;                               // ѡ��������
      PSI^.ClientType := iClientType;                       // �ͻ�������
      PSI^.dwCreateTick := GetTickCount;                    // ������ʱ��
      PSI^.BoMasterIP := BoMasterIP;
      PSI^.bNetType := NetType;
      FSessionList.Lock;
      try
        FSessionList.Remove(iSessionID);
        FSessionList.Add(iSessionID, PSI);
      finally
        FSessionList.UnLock;
      end;
    end;
    with NextGateInfo do
    begin
      SessionID := PInfo^.iSessionID;
      G_GGSocket.GetComfyGate(iAddr, GatePort, PInfo^.NetType); // ȡ�����ӵ�Gate
      GateAddr := iAddr;
    end;
    SendToServer(SM_SELECT_SERVER, Param, @NextGateInfo, sizeof(TNextGateInfo));
  end;
end;
	*/
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
		//G_GGSocket.GetComfyGate(iAddr, GatePort, PInfo^.NetType); // ȡ�����ӵ�Gate
		ngInfo.iSessionID = pInfo->iSessionID;
		ngInfo.iGateAddr = iAddr;
		SendToServerPeer(SM_SELECT_SERVER, iParam, &ngInfo, sizeof(TNextGateInfo));
	}
}

/************************End Of CDGClientSocket********************************************/