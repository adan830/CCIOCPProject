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
	bool bRetFlag = false;
	unsigned short usYear = 0;
	unsigned short usMonth = 0;
	unsigned short usDay = 0;
	std::string sTemp;
	if (sIdentificationID.length() == 15)
	{
		sTemp.assign(sIdentificationID, 6, 2);
		usYear = StrToIntDef(sTemp, 0) + 1900;
		sTemp.assign(sIdentificationID, 8, 2);
		usMonth = StrToIntDef(sTemp, 0);
		sTemp.assign(sIdentificationID, 10, 2);
		usDay = StrToIntDef(sTemp, 0);
	}
	else if (sIdentificationID.length() == 18)
	{
		sTemp.assign(sIdentificationID, 6, 4);
		usYear = StrToIntDef(sTemp, 0);
		sTemp.assign(sIdentificationID, 10, 2);
		usMonth = StrToIntDef(sTemp, 0);
		sTemp.assign(sIdentificationID, 12, 2);
		usDay = StrToIntDef(sTemp, 0);
	}

	/*
	//---------------------
	//---------------------
	//---------------------
	  if (wYear > 0) and TryEncodeDateTime(wYear, wMonth, wDay, 0, 0, 0, 0, birthDay) and (YearsBetween(Now(), BirthDay) < 18) then
		Result := True;
	*/

	return bRetFlag;
}

void CProtectChildManager::CheckOnline()
{
	POnLineChild pChild = nullptr;
	int iCount = 0;
	std::lock_guard<std::mutex> guard(m_LockCS);	
	m_OnLineChildren.First();
	while (!m_OnLineChildren.Eof())
	{
		pChild = (POnLineChild)m_OnLineChildren.GetNextNode();
		iCount = pChild->RoleNameList.size();
		if (0 == iCount)
		{
			// 下线时间超过5小时
			if (m_uiNowTime - pChild->uiLogoutTime >= MAX_OFFLINE_SECOND)
				m_OnLineChildren.Remove(pChild->sIdentificationID);
		}
		else
		{
			// 更新下线时间
			pChild->uiLogoutTime = m_uiNowTime;
			int iIncSec = iCount;
			int iPeriod = 0;
			pChild->iOnLineSecond += iIncSec;
			if (pChild->iOnLineSecond < MAX_ONLINE_SECOND)
				iPeriod = pChild->iOnLineSecond / (30 * 60);
			else
				iPeriod = pChild->iOnLineSecond / (5 * 60);

			if (iPeriod != pChild->iOnLinePeriod)
			{
				pChild->iOnLinePeriod = iPeriod;
				if ((iPeriod != 0) && (iPeriod != 1) && (iPeriod != 3) && (iPeriod != 5))
					BroadcastNotify(pChild);
			}
		}
	}
}

void CProtectChildManager::LoadChildOnlineInfo()
{
	try
	{
		std::string sJsonStr;
		std::string sTemp;
		//c++解析utf-8的json文件乱码，还是需要ascii
		ifstream configFile;
		configFile.open(G_CurrentExeDir + CHILD_FILE_NAME);
		while (!configFile.eof())
		{
			configFile >> sTemp;
			sJsonStr.append(sTemp);
		}
		configFile.close();

		Json::Reader reader;
		Json::Value root;
		if (reader.parse(sJsonStr, root))
		{
			if (root.isArray())
			{
				std::string sCD;
				std::string sTemp;
				Json::Value childItem;
				Json::Value roleListItem;
				Json::Value roleItem;
				POnLineChild pChild = nullptr;
				POnLineRoleName pRoleName = nullptr;
				for (int i = 0; i < (int)root.size(); i++)
				{
					childItem = root.get(i, nullptr);
					if (childItem == nullptr)
						continue;

					sCD = childItem.get("CD", "").asString();
					if (sCD.compare("") == 0)
						return;
					pChild = new TOnLineChild();
					pChild->sIdentificationID = sCD;
					pChild->iOnLineSecond = childItem.get("OS", 0).asInt();
					pChild->iOnLinePeriod = childItem.get("OP", 0).asInt();
					pChild->uiLogoutTime = childItem.get("LT", 0).asInt64();
					m_OnLineChildren.Add(sCD, pChild);

					roleListItem = childItem.get("RL", nullptr);
					if ((roleListItem != nullptr) && (roleListItem.isArray()))
					{
						for (int j = 0; j < (int)roleListItem.size(); j++)
						{
							roleItem = roleListItem.get(j, nullptr);
							if (roleItem == nullptr)
								continue;

							pRoleName = new TOnLineRoleName();
							pRoleName->sRoleName = roleItem.get("RN", "").asString();
							pRoleName->iServerID = roleItem.get("SD", 0).asInt();
							pChild->RoleNameList.push_back(pRoleName);
							++m_iTotalChildCount;
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		Log("LoadChildOnlineInfo:", lmtException);
	}
}

void CProtectChildManager::SaveChildOnlineInfo()
{
	/*
var
  i, iCount         : integer;
  FileName          : ansistring;
  jsonlist, jl      : TlkJSONlist;
  jo, js            : TlkJSONobject;
  PR                : POnLineRoleName;
  PC                : POnLineChild;
begin
  FileName := ExtractFilePath(ParamStr(0)) + CHILD_FILE_NAME;
  if m_OnLine_Children.Count = 0 then
  begin
    DeleteFile(FileName);
    Exit;
  end;
  try
    jsonlist := TlkJSONlist.Create;
    try
      with m_OnLine_Children do
      begin
        Lock;
        try
          First;
          while not Eof do
          begin
            PC := Next;
            jo := TlkJSONobject.Create;
            jsonlist.Add(jo);
            with PC^ do
            begin
              jo.Add('CD', identificationID);
              jo.Add('OS', OnLineSecond);
              jo.Add('OP', OnLinePeriod);
              jo.Add('LT', LogoutTime);
              iCount := RoleNameList.Count;
              if iCount > 0 then
              begin
                jl := TlkJSONlist.Create;
                jo.Add('RL', jl);                           // RoleName List
                for i := 0 to RoleNameList.Count - 1 do
                begin
                  PR := RoleNameList[i];
                  js := TlkJSONobject.Create;
                  jl.Add(js);
                  with PR^ do
                  begin
                    js.Add('RN', RoleName);
                    js.Add('SD', ServerID);
                  end;
                end;
              end;
            end;
          end;
        finally
          UnLock;
        end;
      end;
      TlkJSONstreamed.SaveToFile(jsonlist, FileName);
    finally
      jsonlist.Free;
    end;
  except
    on E: Exception do
      Log('SaveChildOnlineInfo:' + E.Message, lmtException);
  end;
end;
	*/
}

bool CProtectChildManager::Notify(const std::string &sCardID, const std::string &sRoleName, int iServerID, int iOnLineSec)
{
	bool bRetFlag = false;
	if (m_OnNotifyEvent != nullptr)
	{
		TGameChildInfo info;
		memset(&info, 0, sizeof(TGameChildInfo));
		memcpy_s(info.Child.szCard_ID, sizeof(info.Child.szCard_ID), sCardID.c_str(), sCardID.length() + 1);
		memcpy_s(info.Child.szRoleName, sizeof(info.Child.szRoleName), sRoleName.c_str(), sRoleName.length() + 1);
		info.iOnLineSecond = iOnLineSec;
		bRetFlag = m_OnNotifyEvent(iServerID, &info);
	}
	return bRetFlag;
}

void CProtectChildManager::BroadcastNotify(POnLineChild p)
{
	if (m_OnNotifyEvent != nullptr)
	{
		std::vector<POnLineRoleName>::iterator vIter;
		POnLineRoleName pRoleName = nullptr;
		for (vIter = p->RoleNameList.begin(); vIter != p->RoleNameList.end(); ++vIter)
		{
			pRoleName = *vIter;
			if (!Notify(p->sIdentificationID, pRoleName->sRoleName, pRoleName->iServerID, p->iOnLineSecond))
			{
				vIter = p->RoleNameList.erase(vIter);
				delete pRoleName;
				--m_iTotalChildCount;
			}

			if (vIter == p->RoleNameList.end())
				break;
		}
	}
}

void CProtectChildManager::ShowStatus()
{
	std::string sTemp = std::to_string(m_OnLineChildren.GetCount()) + std::to_string(m_iTotalChildCount);
	UpdateLabel(sTemp, LABEL_CHILD_COUNT_ID);
}

/************************End Of CProtectChildManager******************************************/

