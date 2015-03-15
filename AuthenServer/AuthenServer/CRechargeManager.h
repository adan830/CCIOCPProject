/**************************************************************************************
@author: 陈昌
@content: 充值付费管理器
**************************************************************************************/
#ifndef __CC_RECHARGE_MANAGER_H__
#define __CC_RECHARGE_MANAGER_H__

#include "stdafx.h"
using namespace CC_UTILS;

/**
*
* 数据库处理线程
*
*/
class CRechargeSQLWorkThread : public CExecutableBase
{
public:
	CRechargeSQLWorkThread(void* owner, const std::string &sConnectStr);
	virtual ~CRechargeSQLWorkThread();
	virtual void DoExecute();
	bool IsEnabled();
private:
	void OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg);
	void CheckProcExists(void* Sender);
	std::string BuildJsonResult(IMySQLFields* pDataSet);
	bool QueryAreaRecharge(PJsonJobNode pNode);
	bool DBRechargeAck(PJsonJobNode pNode);			
private:
	void* m_Owner;
	std::string m_sConnectStr;
	bool m_bEnabled;
	CMySQLManager* m_pMySQLProc;
};

/**
*
* 充值管理器
*
*/
class CRechargeManager
{
public:
	CRechargeManager();
	virtual ~CRechargeManager();
	bool AddRechargeJob(int iCmd, int iHandle, int iParam, const std::string &sTxt, const bool bForce = false);
	PJsonJobNode PopRechargeJob();
	bool IsEnable();
private:
	std::string LoadSQLConfig();
	void Clear();
private:
	std::mutex m_LockCS;
	PJsonJobNode m_pFirst;
	PJsonJobNode m_pLast;
	int m_iCount;
	CRechargeSQLWorkThread* m_pWorkThread;
};

extern CRechargeManager* pG_RechargeManager;

#endif //__CC_RECHARGE_MANAGER_H__