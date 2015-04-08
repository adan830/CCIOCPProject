/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CClientServerSocket.h"
#include "CGGGlobal.h"

using namespace CC_UTILS;

CClientServerSocket* pG_ClientServerSocket;

enum TBoardStatus { bsRun=0, bsMinRun };						    // GG的状态信息
int G_BoardStatus[2];

const int MAX_GPS_TIME = 60 * 1000;									// 最长1分钟需要返回
const int MAX_GPS_PACKAGE_COUNT = 20;                               // 至多等20包需要返回
const int MAX_GPS_ACTION_COUNT = 100;                               // 最多100个动作就需要检测
const int MAX_DELAY_PACKAGE = 100;
const int SKILL_Z_TY = 1005;										//跳跃
const int SKILL_Z_YM = 1006;										//野蛮

//行会操作字协议
const int GOP_CREATE_TITLE = 6;										// 创建称号 strOpStr1为新称号名称
const int GOP_RENAME_TITLE = 7;										// 称号改名 strOpStr1为旧称号，strOpStr2为新称号
const int GOP_MODIFY_NOTICE = 12;                                   // 修改行会通告

// CD常量
const int CD_ATTACK_TIME = 760;										//物理攻击公共CD
const int CD_MAGIC_TIME = 1300;										//魔法攻击公共CD
const int CD_MOVE_TIME = 600;										//位移公共CD
const int CD_USEITEM_TIME = 200;
const int CD_CLICKNPC_TIME = 1000;
const int CD_GUILDOP_TIME = 500;
const int CD_SAY_TIME = 2000;
const int CD_RUSH_TIME = 540;
const int CD_EMAIL_TIME = 1000;

// 硬直时间常量
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

//服务端与客户端容错时间
const int SEVER_CLIENT_DIFF_TIME = 20;

/************************Start Of CClientConnector******************************************/

CPlayerClientConnector::CPlayerClientConnector() : m_OnSendToServer(nullptr), m_iObjectID(0), m_pFirst(nullptr), m_pLast(nullptr),
	m_EnCodeFunc(nullptr), m_DeCodeFunc(nullptr), m_bDisconnected(false), m_bDeath(false), m_bNormalClose(false)
{
}

CPlayerClientConnector::~CPlayerClientConnector()
{
	PSkillCoolDelay pSkillCD = nullptr;
	std::vector<PSkillCoolDelay>::iterator vIter;
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
		Log(m_sRoleName + " " + GetRemoteAddress() + " 被服务器断开,Reason=" + std::to_string(iReason), lmtWarning);
	m_bNormalClose = true;
}

void CPlayerClientConnector::OnDisconnect()
{
	m_bDisconnected = true;
#ifdef TEST
	if ((!m_bNormalClose) && (m_iObjectID != 0) && (m_sRoleName != ""))
		Log(CC_UTILS::FormatStr("%s[%s]异常断开,Map=%d Cmd=%d/%d NoData=%d秒", m_sRoleName, 
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
			//单个数据包超长或者小于正常长度则扔掉
			if ((iPackageLen >= MAXWORD) || (iPackageLen < sizeof(TClientSocketHead)))
			{
				iOffset = iTempBufLen;
				iErrorCode = 1;
				break;
			}
			//加载m_pReceiveBuffer数据时，解析最新的包长度iPackageLen在当前位移iOffset上超出iTempBufLen
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

			//处理收到的数据包，子类实现
			ProcessReceiveMsg((char*)pHeader, pTempBuf + sizeof(TClientSocketHead), pHeader->usPackageLen - sizeof(TClientSocketHead));
			//移动指针，继续加载socket读入的数据
			iOffset += iPackageLen;
			pTempBuf += iPackageLen;
		}
		else
		{	//向下寻找包头
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
		std::string sOperStr;
		std::string sFindStr;
		PPkgGuildOpRec pPkg = (PPkgGuildOpRec)pBuf;
		switch (pPkg->ucOpID)
		{
		case GOP_CREATE_TITLE:
			pPkg->strOpStr1[GUILD_OP_MAX_LEN - 1] = '\0';
			sOperStr = pPkg->strOpStr1;
			break;
		case GOP_RENAME_TITLE:
		case GOP_MODIFY_NOTICE:
			pBuf += sizeof(TPkgGuildOpRec);
			usBufLen -= sizeof(TPkgGuildOpRec);
			if (usBufLen > 0)
				sOperStr.assign(pBuf, usBufLen);
			break;
		default:
			break;
		}
		if (sOperStr != "")
		{
			if (pG_DBServer != nullptr)
				sFindStr = pG_DBServer->IsIncludeForbiddenWord(sOperStr);
			if (sFindStr != "")
			{
				SendMsg(CC_UTILS::FormatStr("含有违禁字[%s],设置失败！", sFindStr.c_str()));
				retFlag = true;
			}
		}
	}
	return retFlag;
}

bool CPlayerClientConnector::CheckInputWords(char* pBuf, unsigned short usBufLen)
{
	bool bRetFlag = false;

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
      SendMsg(Format('含有违禁字[%s]，输入错误。', [pf]));
      Result := True;
    end
    else
    begin
      if IsEmptyChinese(sMsg) then
      begin
        SendMsg('含有非法字符，输入错误。');
        Result := True;
      end;
    end;
  end;
end;
	*/

	return bRetFlag;
}

void CPlayerClientConnector::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
{
	unsigned short usIdent = ((PClientSocketHead)pHeader)->usIdent;
	switch (usIdent)
	{
	case CM_PING:
		SendToClientPeer(SCM_PING, _ExGetTickCount, nullptr, 0);
		break;
	case CM_GAME_CONNECT:
		if ((!pG_GameServer->IsConnected()) || (pG_GameServer->m_bShutDown))
		{
			OpenWindow(cwMessageBox, 0, "目前服务器处于维护中，请稍候！！！");
			DelayClose(-10);
			return;
		}
		if (sizeof(int) == iDataLen)
		{
			TPlayerConnectRec connectRec;
			connectRec.iSessionID = *(int*)pData;
			connectRec.iIntAddr = inet_addr(GetRemoteAddress().c_str());
			m_OnSendToServer(SM_PLAYER_CONNECT, GetSocketHandle(), (char*)&connectRec, sizeof(TPlayerConnectRec));			
#ifndef NO_ENCODE
			InitDynCode(0);
#endif
		}
		break;
	default:
		{
			if (m_bNormalClose)
				m_bNormalClose = false;
			char* pSendData = pHeader + sizeof(TClientSocketHead)-sizeof(unsigned short);
			*(unsigned short*)pSendData = usIdent;
			unsigned short usSendLen = iDataLen + sizeof(unsigned short);
			unsigned char ucCDType = 0;
			unsigned int uiCDTime = 0;
			TActionType actType = AnalyseIdent(pSendData, usSendLen, ucCDType, uiCDTime);
			m_usLastCMCmd = *(unsigned short*)pSendData;

			switch (ucCDType)
			{
			case CD_NOT_DELAY:
				m_OnSendToServer(SM_PLAYER_MSG, GetSocketHandle(), pSendData, usSendLen);
				break;
			case CD_DENY:
				break;
			default:
				PClientActionNode pNode = new TClientActionNode;
				pNode->usBufLen = usSendLen;
				pNode->szBuf = (char*)malloc(usSendLen);
				memcpy(pNode->szBuf, pSendData, usSendLen);
				pNode->ucCDType = ucCDType;
				pNode->uiCDTime = uiCDTime;
				pNode->ActType = actType;
				AddToDelayQueue(pNode);
				break;
			}
		}
		break;
	}
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
	//非GM号跟踪
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
	int iStrLen = sMsg.length();
	if (iStrLen > 0)
	{
		int iBufLen = sizeof(TPkgMsgHead)+iStrLen + 1;
		char* pBuf = (char*)malloc(iBufLen);
		memset(pBuf, 0, iBufLen);
		try
		{
			GetMsgColor(msgType, ucColor, ucBackColor);
			((PPkgMsgHead)pBuf)->iObjID = 0;
			((PPkgMsgHead)pBuf)->ucBackColor = ucBackColor;
			((PPkgMsgHead)pBuf)->ucColor = ucColor;
			((PPkgMsgHead)pBuf)->ucMsgType = msgType;
			((PPkgMsgHead)pBuf)->ucMsgLen = iStrLen;
			char* pData = pBuf + sizeof(TPkgMsgHead);
			memcpy_s(pData, iStrLen + 1, sMsg.c_str(), iStrLen);
			SendToClientPeer(SCM_SYSTEM_MESSAGE, 0, pBuf, iBufLen);
			free(pBuf);
		}
		catch (...)
		{
			free(pBuf);
		}
	}
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
	TActionType retAction = acNone;
	if (m_bTrace && (m_sRoleName != ""))
		TracertPackage(m_sRoleName, pBuf, usBufLen);
	ucCDType = CD_NOT_DELAY;
	uiCDTime = 0;
	char* pData = pBuf;
	unsigned short usDataLen = usBufLen;
	unsigned short usIdent = *(unsigned short*)pData;
	pData += sizeof(unsigned short);
	usDataLen -= sizeof(unsigned short);

	switch (usIdent)
	{
	case CM_PHYHIT:
		retAction = acAttack1;
		ucCDType = CD_ATTACK;
		uiCDTime = m_usHitSpeed;
		break;
	case CM_WALK:
		retAction = acWalk;
		ucCDType = CD_MOVE;
		uiCDTime = CD_MOVE_TIME;
		break;
	case CM_RUN:
		retAction = acRun;
		ucCDType = CD_MOVE;
		uiCDTime = CD_MOVE_TIME;
		break;
	case CM_STEP_BACKWARD:
		retAction = acStepBack;
		ucCDType = CD_MOVE;
		uiCDTime = CD_MOVE_TIME;
		break;
	case CM_RUSH:
		retAction = acRush;
		ucCDType = CD_RUSH;
		uiCDTime = CD_RUSH_TIME;
		break;
	case CM_SPELL:
		retAction = acMagic1;
		ucCDType = CD_MAGIC;
		uiCDTime = CD_MAGIC_TIME;
		if (usDataLen >= sizeof(TPkgSpellReq))
		{
			PPkgSpellReq pSpell = (PPkgSpellReq)pData;
			PSkillCoolDelay pCD = nullptr;
			std::vector<PSkillCoolDelay>::iterator vIter;
			for (vIter = m_SkillCDTable.begin(); vIter != m_SkillCDTable.end(); ++vIter)
			{
				pCD = *vIter;
				if ((pCD != nullptr) && (pSpell->usSkillID == pCD->usSkillID))
				{
					retAction = pCD->ActType;
					ucCDType = pCD->ucCDType;  // CD类型,不能为1
					uiCDTime = pCD->uiCDTime;
					break;
				}
			}
		}
		if ((acAttack1 == retAction) || (acAttack2 == retAction))
			retAction = acNone;
		break;
	case CM_USE_BAG_ITEM:
		if (usDataLen >= sizeof(TPkgUseItem))
		{
			PPkgUseItem pItem = (PPkgUseItem)pData;
			if (pItem->ucCDType > 10)
			{
				ucCDType = pItem->ucCDType;    // CD类型,不能为1
				uiCDTime = pItem->usCDTime;
			}
		}
		break;
	case CM_CLICK_NPC:
		ucCDType = CD_CLICK_NPC;
		uiCDTime = CD_CLICKNPC_TIME;
		break;
    case CM_GUILD_OP:
		ucCDType = CD_RELATION_OP;
		uiCDTime = CD_GUILDOP_TIME;
		if (CheckGuildWords(pData, usDataLen))
			ucCDType = CD_DENY;
		break;
    case CM_SAY:
		ucCDType = CD_SAY;
		if (m_bGM)
			uiCDTime = 100;
		else
			uiCDTime = CD_SAY_TIME;
		break;
	case CM_CLOSE_WINDOW:
		ucCDType = CD_SAY;
		uiCDTime = CD_SAY_TIME;
		if ((usDataLen <= sizeof(TClientWindowRec)) || (((PClientWindowRec)pData)->WinType = cwPayPwd))
			return retAction;

		pData += sizeof(TClientWindowRec);
		usDataLen -= sizeof(TClientWindowRec);
		if (CheckGuildWords(pData, usDataLen))
			ucCDType = CD_DENY;
		break;
	case CM_TRAN_COMMIT:
		if ((usDataLen <= sizeof(TPkgCommitTran)) || (((PPkgCommitTran)pData)->ucResult != 1) || (usDataLen != sizeof(TPkgCommitTran)+((PPkgCommitTran)pData)->usDataLen))
			return retAction;

		pData += sizeof(TPkgCommitTran);
		usDataLen -= sizeof(TPkgCommitTran);
		if (CheckGuildWords(pData, usDataLen))
			ucCDType = CD_DENY;
		break;	   
	default:
		break;
	}

//#ifdef TESTACT
	if (ucCDType > CD_NOT_DELAY)
		Log(CC_UTILS::FormatStr("action:%d  cdType:%d   cdTime:%d  Diff=%d", retAction, ucCDType, uiCDTime, _ExGetTickCount - m_LastCDTicks[ucCDType]));
//#endif

	if (ucCDType > MAX_CD_ID)
	{
		Log(CC_UTILS::FormatStr("DENY cdType=%d, Ident=%d, RoleName=%s", ucCDType, usIdent, m_sRoleName));
		ucCDType = CD_DENY;
	}
	if (CD_NOT_DELAY == ucCDType)
		uiCDTime = 0;
	return retAction;
}

bool CPlayerClientConnector::AcceptNextAction()
{
	return _ExGetTickCount - m_uiLastActionTick >= m_usStiffTime;
}

void CPlayerClientConnector::Stiffen(TActionType aType)
{
	m_uiLastActionTick = _ExGetTickCount;
	switch (aType)
	{
	case acWalk:
	case acRun:
	case acStepBack:
		m_usStiffTime = STIFF_MOVE;
		break;
	case acAttack1:
	case acAttack2:
		m_usStiffTime = STIFF_ATTACK;
		break;
	case acMagic1:
		m_usStiffTime = STIFF_MAGIC1;
		break;
	case acMagic2:
		m_usStiffTime = STIFF_MAGIC2;
		break;
	case acShoot1:
		m_usStiffTime = STIFF_SHOOT1;
		break;
	case acShoot2:
		m_usStiffTime = STIFF_SHOOT2;
		break;
	case acJump:
		m_usStiffTime = STIFF_JUMP;
		break;
	case acRush:
		m_usStiffTime = STIFF_RUSH;
		break;
	case acBackStep:
		m_usStiffTime = STIFF_PUSH;
		break;
	default:
		m_usStiffTime = STIFF_DEFAULT;
		break;
	}

	if (m_usStiffTime > SEVER_CLIENT_DIFF_TIME)
		m_usStiffTime -= SEVER_CLIENT_DIFF_TIME;
}

void CPlayerClientConnector::IncStiffen(unsigned int uiIncValue)
{
	unsigned int uiStiffTime = _ExGetTickCount - m_uiLastActionTick;
	if (uiStiffTime > m_usStiffTime)
		m_usStiffTime = 0;
	else
		m_usStiffTime -= uiStiffTime;
	m_usStiffTime += uiIncValue;
	m_uiLastActionTick = _ExGetTickCount;
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
		Log(m_sRoleName + " 被强行断开", lmtMessage);
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
	bool bRetFlag = false;
	if (CD_NOT_DELAY == pNode->ucCDType)
		bRetFlag = true;
	else if (acNone == pNode->ActType)
		bRetFlag = _ExGetTickCount > m_LastCDTicks[pNode->ucCDType];
	else if (AcceptNextAction())
	{
		bRetFlag = _ExGetTickCount > m_LastCDTicks[pNode->ucCDType];
		if (bRetFlag)
			Stiffen(pNode->ActType);
	}

	if (bRetFlag)
	{
#ifdef TESTACT
		Log(CC_UTILS::FormatStr("Ident %d Send To GameServer!", *(unsigned short*)(pNode->szBuf)));
#endif
		m_OnSendToServer(SM_PLAYER_MSG, GetSocketHandle(), pNode->szBuf, pNode->usBufLen);
		m_LastCDTicks[pNode->ucCDType] = _ExGetTickCount + pNode->uiCDTime - SEVER_CLIENT_DIFF_TIME;
	}
	return bRetFlag;
}

void CPlayerClientConnector::SCMSkillList(char* pBuf, unsigned short usBufLen)
{
	if (usBufLen > sizeof(TRecordMsgHead))
	{
		usBufLen -= sizeof(TRecordMsgHead);
		PRecordMsgHead pHead = (PRecordMsgHead)pBuf;
		char* pCurr = pBuf + sizeof(TRecordMsgHead);
		if (usBufLen >= pHead->usRecordSize * pHead->usRecordCount)
		{
			while (usBufLen >= pHead->usRecordCount)
			{
				SCMAddSkill(pCurr, pHead->usRecordSize);
				usBufLen -= pHead->usRecordSize;
				pCurr += pHead->usRecordSize;
			}
		}
		else
			Log(CC_UTILS::FormatStr("技能列表长度不正确：%d/%d", pHead->usRecordSize * pHead->usRecordCount, usBufLen));
	}
}

void CPlayerClientConnector::SCMAddSkill(char* pBuf, unsigned short usBufLen)
{
	if (usBufLen >= sizeof(TClientMagicInfo))
	{
		PClientMagicInfo pInfo = (PClientMagicInfo)pBuf;
		if (pInfo->ucCDType < MAX_CD_ID)
		{
			std::lock_guard<std::mutex> guard(m_LockQueueCS);
			std::vector<PSkillCoolDelay>::iterator vIter;
			PSkillCoolDelay pCD = nullptr;
			for (vIter = m_SkillCDTable.begin(); vIter != m_SkillCDTable.end(); ++vIter)
			{
				pCD = *vIter;
				if ((pCD != nullptr) && (pCD->usSkillID == pInfo->usMagicID))
					return;
			}
			pCD = new TSkillCoolDelay;
			pCD->usSkillID = pInfo->usMagicID;
			pCD->ActType = TActionType(pInfo->ucActType);
			pCD->ucCDType = pInfo->ucActType;
			pCD->uiCDTime = pInfo->uiCDTime;
			m_SkillCDTable.push_back(pCD);
		}
		else
			Log("CDType Error " + std::to_string(pInfo->ucCDType));
	}
}

void CPlayerClientConnector::SCMUpdateCDTime(char* pBuf, unsigned short usBufLen)
{
	if (usBufLen == sizeof(TPkgCDTimeChg))
	{
		PPkgCDTimeChg pCD = (PPkgCDTimeChg)pBuf;
		unsigned char ucType = pCD->ucCDType;
		if (ucType < MAX_CD_ID)
		{
			if (0 == pCD->uiCDTime)
				m_LastCDTicks[ucType] = 0;
		}
		if ((CD_ATTACK == ucType) && (pCD->uiCDTime > 0))
			m_usHitSpeed = pCD->uiCDTime - 50;
	}	
}

/************************End Of CClientConnector******************************************/


/************************Start Of CClientServerSocket******************************************/

const int DISCONNET_BUF_LEN = 512 * 1024;     //系统中本身有260k缓存是不会计算的

CClientServerSocket::CClientServerSocket()
{
	SetMaxBlockSize(DISCONNET_BUF_LEN);
	m_OnCreateClient = std::bind(&CClientServerSocket::OnCreateClientSocket, this, std::placeholders::_1);
	m_OnClientError = std::bind(&CClientServerSocket::OnClientError, this, std::placeholders::_1, std::placeholders::_2);
	m_OnConnect = std::bind(&CClientServerSocket::OnClientConnect, this, std::placeholders::_1);
	m_OnDisConnect = std::bind(&CClientServerSocket::OnClientDisconnect, this, std::placeholders::_1);
	m_OnListenReady = std::bind(&CClientServerSocket::OnListenReady, this, std::placeholders::_1);
	LoadConfig();
}

CClientServerSocket::~CClientServerSocket()
{}

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
		Log("GameGate " + std::to_string(iParam) + " 启动.", lmtMessage);

		Log("Connect To GameServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		pG_GameServer->ConnectToServer(sGSIP, pSA->iPort);
		pSA->iPort += 10;
		Log("Connect To IMServer(" + sGSIP + ":" + std::to_string(pSA->iPort) + ")");
		pG_IMServer->ConnectToServer(sGSIP, pSA->iPort);
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
					pClient->m_OnSendToServer = std::bind(&CGSClientSocket::SendToServerPeer, pG_GameServer, 
						std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
				}
				else
				{
					pClient->m_OnSendToServer = std::bind(&CDBClientSocket::SendToServerPeer, pG_DBServer,
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
		Log("收到未知协议，Ident=" + std::to_string(usIdent), lmtWarning);
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
			pClient->OpenWindow(cwMessageBox, 0, "服务器停机维护，连接将断开");
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

		pG_DBServer->DoHeartBeat();
		pG_GameServer->DoHeartBeat();
		pG_IMServer->DoHeartBeat();
	}
}

void CClientServerSocket::LoadConfig()
{
	std::string sConfigFileName(GetAppPathA() + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	try
	{
		m_sDBAddr = pIniFileParser->getString("DBServer", "IP", "127.0.0.1");					// DB地址
		m_iDBPort = pIniFileParser->getInteger("DBServer", "Port", DEFAULT_DBServer_GG_PORT);	// DB端口
		m_sGSIP = pIniFileParser->getString("GameServer", "IP", "");							// Gameserver地址
		int iPort = pIniFileParser->getInteger("Setup", "Port", DEFAULT_GameGate_PORT);			// 侦听端口
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
	pG_DBServer->ConnectToServer(m_sDBAddr, m_iDBPort);
	UpdateLabel("Port:" + std::to_string(m_iListenPort), LABEL_PORT_ID);
}

void CClientServerSocket::OnClientConnect(void* Sender)
{
	CPlayerClientConnector* pConnector = (CPlayerClientConnector*)Sender;
	if (pConnector != nullptr)
	{
		pConnector->m_OnSendToServer = std::bind(&CDBClientSocket::SendToServerPeer, pG_DBServer, 
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
		pG_DBServer->ClientDisconnect(pConnector->GetSocketHandle());
		pG_GameServer->ClientDisconnect(pConnector->GetSocketHandle());
		pG_IMServer->ClientDisconnect(pConnector->GetSocketHandle());;
		pConnector->OnDisconnect();
	}
}

void CClientServerSocket::NotifyNotExistClient(unsigned short usHandle, int iReason)
{
	pG_DBServer->ClientDisconnect(usHandle);
	pG_GameServer->ClientDisconnect(usHandle);
	pG_IMServer->ClientDisconnect(usHandle);
#ifdef TEST
	Log("Not Found Client " + std::to_string(usHandle) + ", Reason=" + std::to_string(iReason), lmtError);
#endif
}

/************************End Of CClientServerSocket******************************************/