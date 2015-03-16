/**************************************************************************************
@author: 陈昌
@content: 账号防沉迷管理器
**************************************************************************************/
#ifndef __CC_PROTECT_CHILD_MANAGER_H__
#define __CC_PROTECT_CHILD_MANAGER_H__

#include "stdafx.h"

typedef std::function<bool(int iServerID, PGameChildInfo pInfo)> TOnNotifyChildEvent;

typedef struct _TOnLineRoleName
{
	std::string sRoleName;
	int iServerID;
}TOnLineRoleName, *POnLineRoleName;

typedef struct _TOnLineChild
{
	std::string sIdentificationID;
	std::vector<POnLineRoleName> RoleNameList;
	int iOnLineSecond;			// 连续在线的秒数
	int iOnLinePeriod;			// 每半小时一个时段
	unsigned int uiLogoutTime;	// (Now() - 40000) * 86400

}TOnLineChild, *POnLineChild;

/**
*
* 账号防沉迷管理线程
*
*/
class CProtectChildManager : public CExecutableBase
{
public:
	CProtectChildManager();
	virtual ~CProtectChildManager();
	virtual void DoExecute();
	bool IsChild(const std::string &sIdentificationID, bool &bIsDeny);
	void Logon(const std::string &sIdentificationID, const std::string &sRoleName, int iServerID);
	void Logout(const std::string &sIdentificationID, const std::string &sRoleName, int iServerID);
public:
	int m_iTotalChildCount;
	TOnNotifyChildEvent m_OnNotifyEvent;
private:
	void OnRemoveChild(void* pValue, const std::string &sKey);
	bool IsChildByCheckBirthday(const std::string &sIdentificationID);
	void CheckOnline();
	void LoadChildOnlineInfo();
	void SaveChildOnlineInfo();
	/*
	procedure EnumJSON(ElName: string; Elem: TlkJSONbase;data: pointer; var Continue: Boolean);
	procedure EnumRole(ElName: string; Elem: TlkJSONbase;data: pointer; var Continue: Boolean);
	*/
	bool Notify(const std::string &sCardID, const std::string &sRoleName, int iServerID, int iOnLineSec);
	void BroadcastNotify(POnLineChild p);
	void ShowStatus();
private:
	unsigned int m_uiNowTime;
	CC_UTILS::CStringHash m_OnLineChildren;
	std::mutex m_LockCS;
};

extern CProtectChildManager* pG_ProtectChildManager;

#endif //__CC_PROTECT_CHILD_MANAGER_H__