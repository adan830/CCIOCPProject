/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#ifndef __CC_SQLDB_MANAGER_H__
#define __CC_SQLDB_MANAGER_H__

#include "stdafx.h"
using namespace CC_UTILS;

class CSQLWorkerUnit;

enum TSQLWorkDB {swMain=1, swSlave} ;

const int MAX_SQL_WORK_COUNT = 3;

/**
*
* 单个数据库处理线程
*
*/
class CSingleSQLWorker : public CExecutableBase
{
public:
	CSingleSQLWorker(CSQLWorkerUnit* owner, unsigned short usIdx, const std::string &sConnectStr);
	virtual ~CSingleSQLWorker();
	virtual void DoExecute();
private:
	std::string _EscapeString(const std::string &str);
	void MySQLAuthenRes(Json::Value js, PJsonJobNode pNode, IMySQLFields* pDataSet, TAccountFlagInfo AccountFlag);
	void OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg);
	bool SQLDB_Authen(PJsonJobNode pNode);
	bool SQLDB_Regist(PJsonJobNode pNode);
	bool SQLDB_AuthenLog(PJsonJobNode pNode);
	bool SQLDB_SafeCardAuthen(PJsonJobNode pNode);
private:
	CSQLWorkerUnit* m_pOwner;
	unsigned short m_usThreadIdx;
	TSQLWorkDB m_WorkType;
	std::string m_sConnectStr;
	CMySQLManager* m_pMySQLProc;
};

/**
*
* 数据库处理单元，按照TSQLWorkDB划分单元，每个里面有多个处理线程
*
*/
class CSQLWorkerUnit
{
public:
	CSQLWorkerUnit(const std::string &s, TSQLWorkDB dbtype);
	virtual ~CSQLWorkerUnit();
	bool AddWorkJob(int iCmd, int iHandle, int iParam, const std::string &s);
	PJsonJobNode PopWorkJob();
private:
	void Clear();
private:
	std::mutex m_LockCS;
	PJsonJobNode m_pFirst;
	PJsonJobNode m_pLast;
	int m_iCount;
	CSingleSQLWorker* m_pWorkThreads[MAX_SQL_WORK_COUNT];
};

/**
*
* 数据库操作管理器
*
*/
class CSQLDBManager
{
public:
	CSQLDBManager();
	virtual ~CSQLDBManager();
	bool AddWorkJob(int iCmd, int iHandle, int iParam, const std::string &s);
	int GetPoolCount();
private:
	void LoadSQLConfig();
private:
	CSQLWorkerUnit* m_pWorkUnits[2];
};

extern CSQLDBManager* pG_SQLDBManager;

#endif //__CC_SQLDB_MANAGER_H__