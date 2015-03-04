/**************************************************************************************
@author: 陈昌
@content: 道具推送管理器
**************************************************************************************/
#ifndef __CC_GIVEITEM_MANAGER_H__
#define __CC_GIVEITEM_MANAGER_H__

#include "stdafx.h"
using namespace CC_UTILS;

/**
*
* 数据库处理线程
*
*/
class CGiveItemSQLWorkThread : public CExecutableBase
{
public:
	CGiveItemSQLWorkThread(void* owner);
	virtual ~CGiveItemSQLWorkThread();
	virtual void DoExecute();
	bool IsEnabled();
	void InitConnectString(const std::string &sHostName, const std::string &sDBName, const std::string &sUserName, 
						   const std::string &sPassword, const std::string &sCharSet, const int iPort);
private:
	void OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg);
	void CheckProcExists(void* Sender);
	std::string BuildJsonResult(IMySQLFields* pDataSet);
	bool QueryAreaGiveItem(PJsonJobNode pNode);
	bool DBGiveItemAck(PJsonJobNode pNode);
private:
	void* m_Owner;
	std::string m_sConnectStr;
	bool m_bEnabled;
	CMySQLManager* m_pMySQLProc;
};

/**
*
* 道具推送管理器
*
*/
class CGiveItemManager
{
public:
	CGiveItemManager();
	virtual ~CGiveItemManager();
	bool AddGiveItemJob(int iCmd, int iHandle, int iParam, const std::string &sTxt, const bool bForce = false);
	PJsonJobNode PopGiveItemJob();
	bool IsEnable();
private:
	void LoadSQLConfig();
	void Clear();
private:
	std::mutex m_LockCS;
	PJsonJobNode m_pFirst;
	PJsonJobNode m_pLast;
	int m_iCount;
	CGiveItemSQLWorkThread* m_pWorkThread;
};

#endif //__CC_GIVEITEM_MANAGER_H__