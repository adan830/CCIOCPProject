/**************************************************************************************
@author: 陈昌
@content: 认证错误日志单元
**************************************************************************************/
#include "stdafx.h"
#include "CAuthFailLog.h"

using namespace CC_UTILS;

CAuthFailFileLog* pG_AuthFailLog;

/************************Start Of CAuthFailFileLog**************************************************/
CAuthFailFileLog::CAuthFailFileLog() : m_ulLastWriteTick(0)
{

	/*
	char* m_pCache;
	int m_iCacheLen;
	std::string m_sPath;
	unsigned long m_ulLastWriteTick;
	int m_iLastDay;
	*/

}

CAuthFailFileLog::~CAuthFailFileLog()
{
	WaitThreadExecuteOver();

}

void CAuthFailFileLog::DoExecute()
{

}

void CAuthFailFileLog::WriteLog(const std::string& sLog)
{

}

void CAuthFailFileLog::WriteCache()
{

}

/************************End Of CAuthFailFileLog****************************************************/
