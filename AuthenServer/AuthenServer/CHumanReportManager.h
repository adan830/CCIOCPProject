/**************************************************************************************
@author: 陈昌
@content: 人数上报管理器
**************************************************************************************/
#ifndef __CC_HUMAN_REPORT_MANAGER_H__
#define __CC_HUMAN_REPORT_MANAGER_H__

#include "stdafx.h"
using namespace CC_UTILS;

/**
*
* 人数上报管理器
*
*/
class CHumanReportManager
{
public:
	CHumanReportManager();
	virtual ~CHumanReportManager();
	void Run(unsigned int uiTick);
private:
	void Initialize();
	void LoadConfig();
	void SaveHumanCount();
	void OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg);
private:
	CMySQLManager* m_pMySQLProc;
	unsigned int m_uiLastRunTick;
	unsigned short m_usRunInterval;
	bool m_bInitial;
	std::string m_sDBHost;
	std::string m_sDBName;
	std::string m_sDBUserName;	
	std::string m_sUserPwd;
};

extern CHumanReportManager* pG_HumanReportManager;

#endif //__CC_HUMAN_REPORT_MANAGER_H__