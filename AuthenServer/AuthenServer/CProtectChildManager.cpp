/**************************************************************************************
@author: 陈昌
@content: 账号防沉迷管理器
**************************************************************************************/
#include "stdafx.h"
#include "CProtectChildManager.h"

using namespace CC_UTILS;

const int MAX_OFFLINE_SECOND = 5 * 60 * 60;         // 离线时长
const int MAX_ONLINE_SECOND = 5 * 60 * 60;          // 在线时长
const std::string CHILD_FILE_NAME = "child.save";

CProtectChildManager* pG_ProtectChildManager;

/************************Start Of CProtectChildManager******************************************/

CProtectChildManager::CProtectChildManager() : m_uiNowTime(0), m_OnLineChildren(3011)
{
	m_OnLineChildren.m_RemoveEvent = std::bind(&CProtectChildManager::OnRemoveChild, this, std::placeholders::_1, std::placeholders::_2);
}

CProtectChildManager::~CProtectChildManager()
{
	WaitThreadExecuteOver();
}

void CProtectChildManager::DoExecute()
{
	LoadChildOnlineInfo();
	unsigned int uiSaveTick = 0;
	while (!IsTerminated())
	{
		//--------------------------------
		//--------------------------------
		//-Now()如何实现？？？？？？？？？
		//-Now()如何实现？？？？？？？？？
		//m_uiNowTime = (unsigned int)((Now() - 40000) * 86400);
		CheckOnline();
		++uiSaveTick;
		if (0 == uiSaveTick % 600)
			SaveChildOnlineInfo();
		WaitForSingleObject(m_Event, 1000);
	}
	SaveChildOnlineInfo();
}

bool CProtectChildManager::IsChild(const std::string &sIdentificationID, bool &bIsDeny)
{
	bool bRetFlag = false;
	bIsDeny = false;
	if (sIdentificationID == "")
		return bRetFlag;
	{
		std::lock_guard<std::mutex> guard(m_LockCS);
		POnLineChild p = (POnLineChild)m_OnLineChildren.ValueOf(sIdentificationID);
		if (p != nullptr)
		{
			bRetFlag = true;
			bIsDeny = (p->iOnLineSecond >= MAX_ONLINE_SECOND);
		}
		else
			bRetFlag = IsChildByCheckBirthday(sIdentificationID);
	}
	return bRetFlag;
}

void CProtectChildManager::Logon(const std::string &sIdentificationID, const std::string &sRoleName, int iServerID)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	POnLineChild pChild = (POnLineChild)m_OnLineChildren.ValueOf(sIdentificationID);
	POnLineRoleName pRoleName = nullptr;
	if (pChild != nullptr)
	{
		std::vector<POnLineRoleName>::iterator vIter;
		for (vIter=pChild->RoleNameList.begin(); vIter!=pChild->RoleNameList.end(); ++vIter)
		{
			pRoleName = *vIter;
			if ((iServerID == pRoleName->iServerID) && (pRoleName->sRoleName.compare(sRoleName) == 0))
			{
				pChild->uiLogoutTime = m_uiNowTime;
				// 防沉迷，连续在线秒数
				Notify(sIdentificationID, sRoleName, iServerID, pChild->iOnLineSecond);
				return;
			}
		}
	}
	else
	{
		if (!IsChildByCheckBirthday(sIdentificationID))
		{
			Log("未成年人才需要加入防沉迷限制！" + sIdentificationID + ":" + sRoleName
				+ "(" + std::to_string(iServerID) + ")", lmtError);
			return;
		}
		pChild = new TOnLineChild();
		pChild->sIdentificationID = sIdentificationID;
		pChild->iOnLineSecond = 0;
		pChild->iOnLinePeriod = 0;
		m_OnLineChildren.Add(sIdentificationID, pChild);
	}

	pRoleName = new TOnLineRoleName();
	pRoleName->sRoleName = sRoleName;
	pRoleName->iServerID = iServerID;
	pChild->uiLogoutTime = m_uiNowTime;
	pChild->RoleNameList.push_back(pRoleName);
	// 防沉迷，连续在线秒数
	Notify(sIdentificationID, sRoleName, iServerID, pChild->iOnLineSecond);
	++m_iTotalChildCount;
	ShowStatus();
}

void CProtectChildManager::Logout(const std::string &sIdentificationID, const std::string &sRoleName, int iServerID)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	POnLineChild pChild = (POnLineChild)m_OnLineChildren.ValueOf(sIdentificationID);
	POnLineRoleName pRoleName = nullptr;
	if (pChild != nullptr)
	{
		std::vector<POnLineRoleName>::iterator vIter;
		for (vIter = pChild->RoleNameList.begin(); vIter != pChild->RoleNameList.end(); ++vIter)
		{
			pRoleName = *vIter;
			if ((iServerID == pRoleName->iServerID) && (pRoleName->sRoleName.compare(sRoleName) == 0))
			{
				pChild->uiLogoutTime = m_uiNowTime;
				pChild->RoleNameList.erase(vIter);
				delete pRoleName;
				--m_iTotalChildCount;
				ShowStatus();
				break;
			}
		}
	}
}

void CProtectChildManager::OnRemoveChild(void* pValue, const std::string &sKey)
{
	POnLineChild pChild = (POnLineChild)pValue;
	if (pChild != nullptr)
	{
		POnLineRoleName pRoleName = nullptr;
		std::vector<POnLineRoleName>::iterator vIter;
		for (vIter = pChild->RoleNameList.begin(); vIter != pChild->RoleNameList.end(); ++vIter)
		{
			pRoleName = *vIter;
			delete pRoleName;
		}
		delete pChild;
	}	
}

bool CProtectChildManager::IsChildByCheckBirthday(const std::string &sIdentificationID)
{
	/*
var
  wYear, wMonth, wDay: Word;
  BirthDay          : TDateTime;
begin
  Result := False;
  wYear := 0;
  wMonth := 0;
  wDay := 0;
  if Length(identificationID) = 15 then
  begin
    wYear := StrToIntDef(Copy(identificationID, 7, 2), 0) + 1900;
    wMonth := StrToIntDef(Copy(identificationID, 9, 2), 0);
    wDay := StrToIntDef(Copy(identificationID, 11, 2), 0);
  end
  else if Length(identificationID) = 18 then
  begin
    wYear := StrToIntDef(Copy(identificationID, 7, 4), 0);
    wMonth := StrToIntDef(Copy(identificationID, 11, 2), 0);
    wDay := StrToIntDef(Copy(identificationID, 13, 2), 0);
  end;

  if (wYear > 0) and
    TryEncodeDateTime(wYear, wMonth, wDay, 0, 0, 0, 0, birthDay) and
    (YearsBetween(Now(), BirthDay) < 18) then
    Result := True;
end;
	*/
}

void CProtectChildManager::CheckOnline()
{

}

void CProtectChildManager::LoadChildOnlineInfo()
{}

void CProtectChildManager::SaveChildOnlineInfo()
{}

bool CProtectChildManager::Notify(const std::string &sCardID, const std::string &sRoleName, int iServerID, int iOnLineSec)
{}

void CProtectChildManager::BroadcastNotify(POnLineChild p)
{}

void CProtectChildManager::ShowStatus()
{}

/************************End Of CProtectChildManager******************************************/

