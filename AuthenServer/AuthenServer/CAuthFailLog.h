/**************************************************************************************
@author: �²�
@content: ��֤������־��Ԫ
**************************************************************************************/
#ifndef __CC_AUTH_FAIL_LOG_H__
#define __CC_AUTH_FAIL_LOG_H__

#include "stdafx.h"

/**
*
* CExecutableBase������----���߳�(������̳У�DoExecute����Ϊ�麯��)
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