/**************************************************************************************
@author: 陈昌
@content: 认证错误日志单元
**************************************************************************************/
#ifndef __CC_AUTH_FAIL_LOG_H__
#define __CC_AUTH_FAIL_LOG_H__

#include "stdafx.h"

/**
*
* CExecutableBase的子类----主线程(无子类继承，DoExecute不作为虚函数)
*
*/
class CAuthFailFileLog : public CExecutableBase
{
public:
	CAuthFailFileLog();
	virtual ~CAuthFailFileLog();
	void DoExecute();
	void WriteLog(const std::string& sLog);
private:
	void WriteCache();
private:	
	char* m_pCache;
	int m_iCacheLen;
	std::string m_sPath;
	unsigned long m_ulLastWriteTick;
	int m_iLastDay;
};

extern CAuthFailFileLog* pG_AuthFailLog;

#endif //__CC_AUTH_FAIL_LOG_H__