/**************************************************************************************
@author: 陈昌
@content: 充值付费管理器
**************************************************************************************/
#ifndef __CC_RECHARGE_MANAGER_H__
#define __CC_RECHARGE_MANAGER_H__

#include "stdafx.h"

/**
*
* 数据库处理线程
*
*/
class CSQLWorkThread : public CExecutableBase
{
public:
	CSQLWorkThread(void* owner, const std::string &sConnectStr);
	virtual ~CSQLWorkThread();
	virtual void DoExecute();
private:
	void OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg);
	void CheckProcExists(void* Sender);
	//function BuildJsonResult(DataSet: IMySQLFields) : AnsiString;
	//std::string BuildJsonResult(IMySQLFields DataSet);
	bool QueryAreaRecharge(PJsonJobNode pNode);
	bool DBRechargeAck(PJsonJobNode pNode);			
private:
	void* m_Owner;
	std::string m_sConnectStr;
	bool m_bEnabled;	
    //FMySQLProc: TMySQL;    
};


#endif //__CC_RECHARGE_MANAGER_H__