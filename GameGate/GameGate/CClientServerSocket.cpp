/**************************************************************************************
@author: �²�
@content: GameGate�Կͻ������ӵļ���socket����
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"
#include "CGGGlobal.h"

using namespace CC_UTILS;

CClientServerSocket* pG_ClientServerSocket;

enum TBoardStatus { bsRun=0, bsMinRun };						    // GG��״̬��Ϣ
int G_BoardStatus[2];

const int MAX_GPS_TIME = 60 * 1000;									// �1������Ҫ����
const int MAX_GPS_PACKAGE_COUNT = 20;                               // �����20����Ҫ����
const int MAX_GPS_ACTION_COUNT = 100;                               // ���100����������Ҫ���
const int MAX_DELAY_PACKAGE = 100;
const int SKILL_Z_TY = 1005;										//��Ծ
const int SKILL_Z_YM = 1006;										//Ұ��

//�л������Э��
const int GOP_CREATE_TITLE = 6;										// �����ƺ� strOpStr1Ϊ�³ƺ�����
const int GOP_RENAME_TITLE = 7;										// �ƺŸ��� strOpStr1Ϊ�ɳƺţ�strOpStr2Ϊ�³ƺ�
const int GOP_MODIFY_NOTICE = 12;                                   // �޸��л�ͨ��

// CD����
const int CD_ATTACK_TIME = 760;										//������������CD
const int CD_MAGIC_TIME = 1300;										//ħ����������CD
const int CD_MOVE_TIME = 600;										//λ�ƹ���CD
const int CD_USEITEM_TIME = 200;
const int CD_CLICKNPC_TIME = 1000;
const int CD_GUILDOP_TIME = 500;
const int CD_SAY_TIME = 2000;
const int CD_RUSH_TIME = 540;
const int CD_EMAIL_TIME = 1000;

// Ӳֱʱ�䳣��
const int STIFF_MOVE = 600;
const int STIFF_ATTACK = 480;
const int STIFF_MAGIC1 = 820;
const int STIFF_MAGIC2 = 820;
const int STIFF_SHOOT1 = 820;
const int STIFF_SHOOT2 = 880;
const int STIFF_JUMP = 720;
const int STIFF_PUSH = 270;
const int STIFF_DEFAULT = 600;
const int STIFF_RUSH = 500;

//�������ͻ����ݴ�ʱ��
const int SEVER_CLIENT_DIFF_TIME = 20;

/************************Start Of CClientConnector******************************************/

CPlayerClientConnector::CPlayerClientConnector() : m_OnSendToServer(nullptr), m_iObjectID(0), m_pFirst(nullptr), m_pLast(nullptr),
	m_EnCodeFunc(nullptr), m_DeCodeFunc(nullptr), m_bDisconnected(false), m_bDeath(false), m_bNormalClose(false)
{
}

CPlayerClientConnector::~CPlayerClientConnector()
{
	PSkillCoolDelay pSkillCD = nullptr;
	std::list<PSkillCoolDelay>::iterator vIter;
	for (vIter = m_SkillCDTable.begin(); vIter != m_SkillCDTable.end(); ++vIter)
	{
		pSkillCD = (PSkillCoolDelay)*vIter;
		if (pSkillCD != nullptr)
		{
			delete pSkillCD;
			pSkillCD = nullptr;
		}
	}
}

void CPlayerClientConnector::SendToClientPeer(unsigned short usIdent, unsigned int uiIdx, void* pBuf, unsigned short usBufLen)
{
	int iDataLen = sizeof(TClientSocketHead)+usBufLen;
	char* pData = (char*)malloc(iDataLen);
	if (pData != nullptr)
	{
		try
		{
			((PClientSocketHead)pData)->uiSign = CS_SEGMENTATION_CLIENTSIGN;
			((PClientSocketHead)pData)->usPackageLen = iDataLen;
			((PClientSocketHead)pData)->usIdent = usIdent;
			((PClientSocketHead)pData)->uiIdx = uiIdx;			
			if (usBufLen > 0)
			{
				memcpy(pData + sizeof(TClientSocketHead), pBuf, usBufLen);
				if (m_EnCodeFunc != nullptr)
					m_EnCodeFunc((CC_UTILS::PBYTE)(&pData[ENCODE_START_LEN]), iDataLen - ENCODE_START_LEN);
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

void CPlayerClientConnector::DelayClose(int iReason)
{
	m_uiCloseTick = _ExGetTickCount;
	if ((iReason != 0) && (!m_bNormalClose) && (!m_bDisconnected) && (!m_bDeath))
		Log(m_sRoleName + " " + GetRemoteAddress() + " ���������Ͽ�,Reason=" + std::to_string(iReason), lmtWarning);
	m_bNormalClose = true;
}

void CPlayerClientConnector::OnDisconnect()
{
	m_bDisconnected = true;
#ifdef TEST
	if ((!m_bNormalClose) && (m_iObjectID != 0) && (m_sRoleName != ""))
		Log(CC_UTILS::FormatStr("%s[%s]�쳣�Ͽ�,Map=%d Cmd=%d/%d NoData=%d��", m_sRoleName, 
		GetRemoteAddress(), m_iMapID, m_usLastCMCmd, m_usLastSCMCmd, (_ExGetTickCount - m_uiLastPackageTick) / 1000), lmtWarning);
#endif
}

void CPlayerClientConnector::SendActGood(unsigned short usAct, unsigned short usActParam)
{
	TPkgActGood actGood;
	actGood.iObjID = m_iObjectID;
	actGood.usAct = usAct;
	actGood.usParam = usActParam;
	SendToClientPeer(SCM_ACTGOOD, 0, &actGood, sizeof(TPkgActGood));
}

void CPlayerClientConnector::UpdateCDTime(unsigned char bCDType, unsigned int uiCDTime)
{
	TPkgCDTimeChg pkg;
	pkg.ucCDType = bCDType;
	pkg.uiCDTime = uiCDTime;
	SendToClientPeer(SCM_CDTIME_UPDATE, _ExGetTickCount, &pkg, sizeof(TPkgCDTimeChg));
}

void CPlayerClientConnector::Execute(unsigned int uiTick)
{
	if (m_uiCloseTick > 0)
	{
		if (_ExGetTickCount - m_uiCloseTick > 3000)
		{
			m_bNormalClose = true;
			Close();
		}
	}
	else
		ProcDelayQueue();	
}

void CPlayerClientConnector::SocketRead(const char* pBuf, int iCount)
{
	if (m_uiCloseTick > 0)
		return;
	m_uiLastPackageTick = _ExGetTickCount;

	int iErrorCode = 0;
	m_pReceiveBuffer->Write(pBuf, iCount);
	char* pTempBuf = (char*)m_pReceiveBuffer->GetMemPoint();
	int iTempBufLen = m_pReceiveBuffer->GetPosition();
	int iOffset = 0;
	int iPackageLen = 0;
	PClientSocketHead pHeader = nullptr;
	while (iTempBufLen - iOffset >= sizeof(TClientSocketHead))
	{
		pHeader = (PClientSocketHead)pTempBuf;
		if (CS_SEGMENTATION_CLIENTSIGN == pHeader->uiSign)
		{
			iPackageLen = pHeader->usPackageLen;
			//�������ݰ���������С�������������ӵ�
			if ((iPackageLen >= MAXWORD) || (iPackageLen < sizeof(TClientSocketHead)))
			{
				iOffset = iTempBufLen;
				iErrorCode = 1;
				break;
			}
			//����m_pReceiveBuffer����ʱ���������µİ�����iPackageLen�ڵ�ǰλ��iOffset�ϳ���iTempBufLen
			if (iOffset + iPackageLen > iTempBufLen)
				break;

			if (m_DeCodeFunc != nullptr)
				m_DeCodeFunc((CC_UTILS::PBYTE)(&pBuf[ENCODE_START_LEN]), iPackageLen - ENCODE_START_LEN);

			if (0 == m_uiPackageIdx)
				m_uiPackageIdx = pHeader->uiIdx;
			else
			{
				++m_uiPackageIdx;
				if (m_uiPackageIdx != pHeader->uiIdx)
				{
					Log(CC_UTILS::FormatStr("%s[%s] Package Index Error.", m_sRoleName, GetRemoteAddress()), lmtWarning);
					break;
				}
			}

			//�����յ������ݰ�������ʵ��
			ProcessReceiveMsg((char*)pHeader, pTempBuf + sizeof(TClientSocketHead), pHeader->usPackageLen - sizeof(TClientSocketHead));
			//�ƶ�ָ�룬��������socket���������
			iOffset += iPackageLen;
			pTempBuf += iPackageLen;
		}
		else
		{	//����Ѱ�Ұ�ͷ
			iErrorCode = 2;
			iOffset += 1;
			pTempBuf += 1;
		}
	}
	m_pReceiveBuffer->Reset(iOffset);
	if (iErrorCode > 0)
	{
		Log(CC_UTILS::FormatStr("CPlayerClientConnector Socket Read Error, Code =%d  %s %s", 
			iErrorCode, m_sRoleName, GetRemoteAddress()), lmtWarning);
	}
}

bool CPlayerClientConnector::CheckGuildWords(char* pBuf, unsigned short usBufLen)
{
	bool retFlag = false;
	if ((pBuf != nullptr) && (usBufLen >= sizeof(TPkgGuildOpRec)))
	{
		char* ps = nullptr;
		char* pf = nullptr;
		PPkgGuildOpRec pPkg = (PPkgGuildOpRec)pBuf;
		switch (pPkg->ucOpID)
		{
		case GOP_CREATE_TITLE:
			pPkg->strOpStr1[GUILD_OP_MAX_LEN - 1] = '\0';
			ps = pPkg->strOpStr1;
			break;
		case GOP_RENAME_TITLE:
		case GOP_MODIFY_NOTICE:
			pBuf += sizeof(TPkgGuildOpRec);
			usBufLen -= sizeof(TPkgGuildOpRec);
			if (usBufLen > 0)
				ps = pBuf;
			break;
		default:
			break;
		}
		if (ps != nullptr)
		{
			pf = pG_ClientServerSocket->m_pDBServer->IsIncludeForbiddenWord(ps);
			if (pf != nullptr)
			{
				SendMsg(CC_UTILS::FormatStr("����Υ����[%s],����ʧ�ܣ�", pf));
				retFlag = true;
			}
		}
	}
	return retFlag;
}

bool CPlayerClientConnector::CheckInputWords(char* pBuf, unsigned short usBufLen)
{
	/*
var
  pf                : PAnsichar;
  sMsg              : AnsiString;
begin
  Result := False;
  if Assigned(Buf) and (BufLen > 0) then
  begin
    SetString(sMsg, Buf, BufLen);
    sMsg := AnsiString(PAnsiChar(sMsg));
    if Length(sMsg) < 1 then
      Exit;
    if IsNumber(sMsg) then
      Exit;
    pf := G_ServerSocket.m_DBSocket.IncludeForbiddenWord(PAnsiChar(sMsg));
    if assigned(pf) then
    begin
      SendMsg(Format('����Υ����[%s]���������', [pf]));
      Result := True;
    end
    else
    begin
      if IsEmptyChinese(sMsg) then
      begin
        SendMsg('���зǷ��ַ����������');
        Result := True;
      end;
    end;
  end;
end;
	*/
}

void CPlayerClientConnector::ProcessReceiveMsg(char* pHeader, char* pBuf, int iBufLen)
{

}

void CPlayerClientConnector::ReceiveServerMsg(char* pBuf, unsigned short usBufLen)
{
	if (usBufLen >= sizeof(TClientSocketHead))
	{
		PClientSocketHead pHead = (PClientSocketHead)pBuf;
		char* pData = pBuf + sizeof(TClientSocketHead);
		int iDataLen = pHead->usPackageLen - sizeof(TClientSocketHead);
		if (CheckServerPkg(pHead->usIdent, pData, iDataLen))
		{
			m_usLastSCMCmd = pHead->usIdent;
			if (m_EnCodeFunc != nullptr)
				m_EnCodeFunc((CC_UTILS::PBYTE)(&pBuf[ENCODE_START_LEN]), usBufLen - ENCODE_START_LEN);
			SendBuf(pBuf, usBufLen);
		}
	}
	else
		Log("Error BufLen = " + std::to_string(usBufLen));
}

bool CPlayerClientConnector::CheckServerPkg(unsigned short usIdent, char* pBuf, unsigned short usBufLen)
{
	char* pTraceBuf = nullptr;
	//��GM�Ÿ���
	if (m_bTrace && (!m_bGM) && (m_sRoleName != ""))
	{
		pTraceBuf = (char*)malloc(usBufLen + sizeof(unsigned short));
		*((unsigned short*)pTraceBuf) = usIdent;
		memcpy(pTraceBuf + sizeof(unsigned short), pBuf, usBufLen);
		TracertPackage(m_sRoleName, pTraceBuf, sizeof(unsigned short)+usBufLen);
	}

	bool bRetFlag = true;

	switch (usIdent)
	{
    case SCM_QUIT:
		m_bNormalClose = true;
		break;
    case SCM_DIE:
		if (usBufLen >= sizeof(TPkgAction))
		{
			if (m_iObjectID == ((PPkgAction)pBuf)->iObjID)
				m_bDeath = true;
		}
		break;
    case SCM_MAPINFO:
		if (usBufLen >= sizeof(TPkgMapInfo))
			m_iMapID = (PPkgMapInfo(pBuf))->iMapID;
		break;
    case SCM_RELIVE:
		if ((usBufLen >= sizeof(int)) && (*((int*)pBuf) == m_iObjectID))
			m_bDeath = false;
		break;
    case SCM_SKILL_LIST:
		SCMSkillList(pBuf, usBufLen);
		break;
    case SCM_SKILL_ADD:
		SCMAddSkill(pBuf, usBufLen);
		break;
    case SCM_LOGON:
		m_iObjectID = *((int*)pBuf);
		break;
    case SCM_AUTHEN_RESULT:
		if (usBufLen >= sizeof(TAuthenResponse))
			m_sAccount = ((PAuthenResponse)pBuf)->szUniqueID;
		break;
    case SCM_ROLE_INFO:
		if (usBufLen >= sizeof(TRoleInfo))
		{
			m_sRoleName = ((PRoleInfo)pBuf)->szRoleName;
			m_bTrace = (((PRoleInfo)pBuf)->iFlag & 0x1) != 0;
#ifdef TEST
			m_bGM = (((PRoleInfo)pBuf)->iFlag & 0x2) != 0;
#endif
			if (m_bGM)
				m_bTrace = true;
		}
		bRetFlag = false;
		break;
	case SCM_BACKSTEP:
	case SCM_L_RUSH:
	case SCM_R_RUSH:
		if (*((int*)pBuf) == m_iObjectID)
			IncStiffen(STIFF_PUSH);
		break;
    case SCM_JUMP:
		if (*((int*)pBuf) == m_iObjectID)
			IncStiffen(STIFF_JUMP);
		break;
    case SCM_CDTIME_UPDATE:
		SCMUpdateCDTime(pBuf, usBufLen);
		break;
	default:
		break;
	}
	return bRetFlag;
}

void CPlayerClientConnector::SendMsg(const std::string &sMsg, TMesssageType msgType, unsigned char ucColor, unsigned char ucBackColor)
{

}

void CPlayerClientConnector::OpenWindow(TClientWindowType wtype, int iParam, const std::string &sMsg)
{
	TClientWindowRec winInfo;
	if ("" == sMsg)
	{
		winInfo.WinType = wtype;
		winInfo.Param = iParam;
		SendToClientPeer(SCM_OPEN_WINDOW, 0, &winInfo, sizeof(TClientWindowRec));
	}
	else
	{
		int iMsgLen = sMsg.length();
		if (iMsgLen > 0)
			++iMsgLen;
		int iBufLen = sizeof(TClientWindowRec)+iMsgLen;
		char* pBuf = (char*)malloc(iBufLen);
		try
		{
			((PClientWindowRec)pBuf)->WinType = wtype;
			((PClientWindowRec)pBuf)->Param = iParam;
			((PClientWindowRec)pBuf)->TransID = 0;
			if (iMsgLen > 0)
			{
				memcpy(pBuf+sizeof(TClientWindowRec), sMsg.c_str(), iMsgLen-1);
				pBuf[iBufLen - 1] = '\0';
			}
			SendToClientPeer(SCM_OPEN_WINDOW, 0, pBuf, iBufLen);
			free(pBuf);
		}
		catch (...)
		{
			free(pBuf);
		}
	}
}

bool CPlayerClientConnector::NeedQueueCount(unsigned char ucCDType)
{
	bool bRetFlag = true;
	switch (ucCDType)
	{
	case CD_NOT_DELAY:
	case CD_SAY:
	case CD_CLICK_NPC:
	case CD_USEITEM:
	case CD_RELATION_OP:
		bRetFlag = false;
		break;
	default:
		break;
	}
	return bRetFlag;
}

void CPlayerClientConnector::InitDynCode(unsigned char ucEdIdx)
{
	PEnDeRecord p = GetCode();
	SendToClientPeer(SCM_ENCODE, _ExGetTickCount, p->pEnBuffer, p->usEnBufferLen);
	SendToClientPeer(SCM_DECODE, _ExGetTickCount, p->pDeBuffer, p->usDeBufferLen);
	m_EnCodeFunc = PCodingFunc(p->pEnBuffer);
	m_DeCodeFunc = PCodingFunc(p->pDeBuffer);
}

TActionType CPlayerClientConnector::AnalyseIdent(char* pBuf, unsigned short usBufLen, unsigned char &ucCDType, unsigned int &uiCDTime)
{

}

bool CPlayerClientConnector::AcceptNextAction()
{
	return _ExGetTickCount - m_uiLastActionTick >= m_usStiffTime;
}

void CPlayerClientConnector::Stiffen(TActionType aType)
{

}

void CPlayerClientConnector::IncStiffen(unsigned int uiIncValue)
{

}

void CPlayerClientConnector::AddToDelayQueue(PClientActionNode pNode)
{
	std::lock_guard<std::mutex> guard(m_LockQueueCS);
	pNode->pNextNode = nullptr;
	pNode->pPrevNode = m_pLast;
	if (pNode->pPrevNode != nullptr)
		pNode->pPrevNode->pNextNode = pNode;
	else
		m_pFirst = pNode;
	m_pLast = pNode;

	if (NeedQueueCount(pNode->ucCDType))
		++m_iQueueCount;
}

void CPlayerClientConnector::ProcDelayQueue()
{
	std::lock_guard<std::mutex> guard(m_LockQueueCS);
	if (m_iQueueCount > MAX_DELAY_PACKAGE)
	{
		Log(m_sRoleName + " ��ǿ�жϿ�", lmtMessage);
		m_bNormalClose = true;
		Close();
		return;
	}

	PClientActionNode pNode = m_pFirst;
	PClientActionNode pNext = nullptr;
	while (pNode != nullptr)
	{
		pNext = pNode->pNextNode;
		if (IsCoolDelayPass(pNode))
		{
			if (pNode->pPrevNode != nullptr)
				pNode->pPrevNode->pNextNode = pNode->pNextNode;
			else
				m_pFirst = pNode->pNextNode;
			if (pNode->pNextNode != nullptr)
				pNode->pNextNode->pPrevNode = pNode->pPrevNode;
			else
				m_pLast = pNode->pPrevNode;

			if (NeedQueueCount(pNode->ucCDType))
				--m_iQueueCount;

			free(pNode->szBuf);
			delete(pNode);
		}
		pNode = pNext;
	}
}

bool CPlayerClientConnector::IsCoolDelayPass(PClientActionNode pNode)
{

}

void CPlayerClientConnector::SCMSkillList(char* pBuf, unsigned short usBufLen)
{

}

void CPlayerClientConnector::SCMAddSkill(char* pBuf, unsigned short usBufLen)
{

}

void CPlayerClientConnector::SCMUpdateCDTime(char* pBuf, unsigned short usBufLen)
{

}

/************************End Of CClientConnector******************************************/


/************************Start Of CClientServerSocket******************************************/

const int DISCONNET_BUF_LEN = 512 * 1024;     //ϵͳ�б�����260k�����ǲ�������

CClientServerSocket::CClientServerSocket()
{
	SetMaxBlockSize(DISCONNET_BUF_LEN);
	m_OnCreateClient = std::bind(&CClientServerSocket::OnCreateClientSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CClientServerSocket::OnClientError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CClientServerSocket::OnClientConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CClientServerSocket::OnClientDisconnect, this, std::placeholders::_1);
	m_OnListenReady = std::bind(&CClientServerSocket::OnListenReady, this, std::placeholders::_1);

	m_pDBServer = new CDBClientSocket();
	m_pGameServer = new CGSClientSocket();
	m_pIMServer = new CIMClientSocket();
	LoadConfig();
}

CClientServerSocket::~CClientServerSocket()
{
	Close();
	delete m_pIMServer;
	delete m_pGameServer;
	delete m_pDBServer;
}

void CClientServerSocket::SMServerConfig(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (sizeof(TServerAddress) == usBufLen)
	{
		G_PublicGateIdx = iParam;
		pG_MainThread->StartLogSocket(iParam);
		std::string sGSIP;
		PServerAddress pSA = (PServerAddress)pBuf;
		if (m_sGSIP != "")
			sGSIP = m_sGSIP;
		else
			sGSIP = pSA->IPAddress;
		Log("GameGate " + std::to_string(iParam) + " ����.", lmtMessage);

		Log("Connect To GameServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		m_pGameServer->ConnectToServer(sGSIP, pSA->iPort);
		pSA->iPort += 10;
		Log("Connect To IMServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		m_pIMServer->ConnectToServer(sGSIP, pSA->iPort);
	}
}

void CClientServerSocket::ProcServerMessage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen)
{
	try
	{
		CPlayerClientConnector* pClient = nullptr;
		switch (usIdent)
		{
		case SM_PLAYER_MSG:
			{
				std::lock_guard<std::mutex> guard(m_LockCS);
				pClient = (CPlayerClientConnector*)ValueOf(usHandle);
				if (pClient != nullptr)
					pClient->ReceiveServerMsg(pBuf, usBufLen);
				else
					NotifyNotExistClient(usHandle, SM_PLAYER_MSG);
			}
			break;
		case SM_MULPLAYER_MSG:
			{
				int iHandlePos = sizeof(unsigned short) * usHandle;
				if (iHandlePos > usBufLen)
					return;	

				unsigned short usDataLen = usBufLen - iHandlePos;
				char* pData = (char*)malloc(usDataLen);
				try
				{
					unsigned short* pHandle = (unsigned short*)pBuf;
					std::lock_guard<std::mutex> guard(m_LockCS);
					for (int i = 0; i < usHandle; i++)
					{
						pClient = (CPlayerClientConnector*)ValueOf(*pHandle);
						if (pClient != nullptr)
						{
							memcpy(pData, pBuf + iHandlePos, usDataLen);
							pClient->ReceiveServerMsg(pData, usDataLen);
						}
						else
							NotifyNotExistClient(*pHandle, SM_MULPLAYER_MSG);
						++pHandle;
					}
					free(pData);
				}
				catch (...)
				{
					free(pData);
				}
			}
			break;
		case SM_BROADCAST_MSG:
			{
				char* pData = (char*)malloc(usBufLen);
				try
				{
					std::lock_guard<std::mutex> guard(m_LockCS);
					std::list<void*>::iterator vIter;
					for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
					{
						pClient = (CPlayerClientConnector*)*vIter;
						if ((pClient != nullptr) && (pClient->m_iObjectID != 0))
						{
							memcpy(pData, pBuf, usBufLen);
							pClient->ReceiveServerMsg(pData, usBufLen);
						}
					}
					free(pData);
				}
				catch (...)
				{
					free(pData);
				}
			}
			break;
		default:
			break;
		}
	}
	catch (...)
	{
		Log("ProcServerMessage: " + std::to_string(usIdent), lmtException);
	}
}

void CClientServerSocket::ClientManage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen, bool bInGame)
{
	CPlayerClientConnector* pClient = nullptr;
	switch (usIdent)
	{
	case SM_PLAYER_CONNECT:
		{
			std::lock_guard<std::mutex> guard(m_LockCS);
			pClient = (CPlayerClientConnector*)ValueOf(usHandle);
			if (pClient != nullptr)
			{
				if (bInGame)
				{
					pClient->m_OnSendToServer = std::bind(&CGSClientSocket::SendToServerPeer, m_pGameServer, 
						std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
				}
				else
				{
					pClient->m_OnSendToServer = std::bind(&CDBClientSocket::SendToServerPeer, m_pDBServer,
						std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
				}					
			}
			else if (m_ActiveConnects.size() > 0)
			{
				NotifyNotExistClient(usHandle, SM_PLAYER_CONNECT);
			}				
		}
		break;
	case SM_PLAYER_DISCONNECT:
		{
			std::lock_guard<std::mutex> guard(m_LockCS);
			pClient = (CPlayerClientConnector*)ValueOf(usHandle);
			if (pClient != nullptr)
			{
				int iReason = 0;
				if ((pBuf != nullptr) && (usBufLen >= sizeof(int)))
					iReason = *(int*)pBuf;
				pClient->DelayClose(iReason);
			}
		}
		break;
	default:
		Log("�յ�δ֪Э�飬Ident=" + std::to_string(usIdent), lmtWarning);
		break;
	}
}

void CClientServerSocket::GameServerShutDown()
{
	CPlayerClientConnector* pClient = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	std::list<void*>::iterator vIter;
	for (vIter = m_ActiveConnects.begin(); vIter != m_ActiveConnects.end(); ++vIter)
	{
		pClient = (CPlayerClientConnector*)*vIter;
		if (pClient != nullptr)
		{
			pClient->OpenWindow(cwMessageBox, 0, "������ͣ��ά�������ӽ��Ͽ�");
			pClient->m_bNormalClose = true;
			pClient->DelayClose(-11);
		}
	}
}

void CClientServerSocket::DoActive()
{
	++m_iLoopCount;
	if (_ExGetTickCount - m_uiSlowerTick >= 1000)
	{
		m_uiSlowerTick = _ExGetTickCount;
		if (G_BoardStatus[bsMinRun] < m_iLoopCount)
			G_BoardStatus[bsMinRun] = m_iLoopCount;
		G_BoardStatus[bsRun] = m_iLoopCount;
		m_iLoopCount = 0;
		UpdateLabel("Run:" + std::to_string(G_BoardStatus[bsRun]) + "/" + std::to_string(G_BoardStatus[bsMinRun]), LABEL_RUN_ID);
		if (!m_bListenOK)
			return;

		m_pDBServer->DoHeartBeat();
		m_pGameServer->DoHeartBeat();
		m_pIMServer->DoHeartBeat();
	}
}

void CClientServerSocket::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	try
	{
		m_sDBAddr = pIniFileParser->getString("DBServer", "IP", "127.0.0.1");					// DB��ַ
		m_iDBPort = pIniFileParser->getInteger("DBServer", "Port", DEFAULT_DBServer_GG_PORT);	// DB�˿�
		m_sGSIP = pIniFileParser->getString("GameServer", "IP", "");							// Gameserver��ַ
		int iPort = pIniFileParser->getInteger("Setup", "Port", DEFAULT_GameGate_PORT);			// �����˿�
		m_sInternetIP = GetInternetIP(pIniFileParser->getString("Setup", "Bind", ""));
		G_BoCheckGPS = pIniFileParser->getBoolean("Setup", "CheckGPS", false);
		iPort += G_LocalGateIdx;
		if (!IsActive())
		{
			m_sLocalIP = "0.0.0.0";
			m_iListenPort = iPort;
			Open();
		}
		delete pIniFileParser;
	}
	catch (...)
	{
		delete pIniFileParser;
	}
}

CClientConnector* CClientServerSocket::OnCreateClientSocket(const std::string &sIP)
{
	return new CPlayerClientConnector();
}

void CClientServerSocket::OnClientError(void* Sender, int& iErrorCode)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	std::string sIP;
	if (pConnector != nullptr)
		sIP = pConnector->GetRemoteAddress();
	Log(sIP + " ListenSocket Error, Code = " + std::to_string(iErrorCode), lmtMessage);
	iErrorCode = 0;
}

void CClientServerSocket::OnListenReady(void* Sender)
{
	m_bListenOK = true;
	Log("Connect To DBServer(" + m_sDBAddr + ":" + std::to_string(m_iDBPort) + ")");
	m_pDBServer->ConnectToServer(m_sDBAddr, m_iDBPort);
	UpdateLabel("Port:" + std::to_string(m_iListenPort), LABEL_PORT_ID);
}

void CClientServerSocket::OnClientConnect(void* Sender)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	if (pConnector != nullptr)
	{
		pConnector->m_OnSendToServer = std::bind(&CDBClientSocket::SendToServerPeer, m_pDBServer, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		pConnector->m_bDisconnected = false;
		UpdateLabel("Connect: " + std::to_string(m_ActiveConnects.size()) + "/" + std::to_string(m_iDelayFreeHandleCount), LABEL_CONNECT_ID);
	}
}

void CClientServerSocket::OnClientDisconnect(void* Sender)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	if (pConnector != nullptr)
	{
		UpdateLabel("Connect: " + std::to_string(m_ActiveConnects.size()) + "/" + std::to_string(m_iDelayFreeHandleCount), LABEL_CONNECT_ID);
		m_pDBServer->ClientDisconnect(pConnector->GetSocketHandle());
		m_pGameServer->ClientDisconnect(pConnector->GetSocketHandle());
		m_pIMServer->ClientDisconnect(pConnector->GetSocketHandle());;
		pConnector->OnDisconnect();
	}
}

void CClientServerSocket::NotifyNotExistClient(unsigned short usHandle, int iReason)
{
	m_pDBServer->ClientDisconnect(usHandle);
	m_pGameServer->ClientDisconnect(usHandle);
	m_pIMServer->ClientDisconnect(usHandle);	
#ifdef TEST
	Log("Not Found Client " + std::to_string(usHandle) + ", Reason=" + std::to_string(iReason), lmtError);
#endif
}

/************************End Of CClientServerSocket******************************************/