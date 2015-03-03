/**************************************************************************************
@author: �²�
@content: ��ֵ���ѹ�����
**************************************************************************************/
#ifndef __CC_RECHARGE_MANAGER_H__
#define __CC_RECHARGE_MANAGER_H__

#include "stdafx.h"
using namespace CC_UTILS;

/**
*
* ���ݿ⴦���߳�
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
	std::string BuildJsonResult(IMySQLFields* pDataSet);
	bool QueryAreaRecharge(PJsonJobNode pNode);
	bool DBRechargeAck(PJsonJobNode pNode);			
private:
	void* m_Owner;
	std::string m_sConnectStr;
	bool m_bEnabled;
	CMySQLManager* m_pMySQLProc;
};


#endif //__CC_RECHARGE_MANAGER_H__