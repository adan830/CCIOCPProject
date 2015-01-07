/**************************************************************************************
@author: 陈昌
@content: 认证错误日志单元
**************************************************************************************/
#include "stdafx.h"
#include "CAuthFailLog.h"

using namespace CC_UTILS;

CAuthFailFileLog* pG_AuthFailLog;
const int MAX_FILE_CACHE_SIZE = 8 * 1024;

/************************Start Of CAuthFailFileLog**************************************************/
CAuthFailFileLog::CAuthFailFileLog() : m_ulLastWriteTick(0), m_iCacheLen(0)
{
	m_sPath = G_CurrentExeDir + "logs\\AuthFailLog\\";
	m_pCache = (char*)malloc(MAX_FILE_CACHE_SIZE);
	/*
  m_LastDay := Trunc(Now());
  try
    ForceDirectories(m_Path);
  except
  end;
  */
}

CAuthFailFileLog::~CAuthFailFileLog()
{
	WaitThreadExecuteOver();
	WriteCache;
	free(m_pCache);
}

void CAuthFailFileLog::DoExecute()
{
	while (!IsTerminated())
	{
		try
		{
			/*
			  ToDay := Trunc(Now());
			  if (ToDay <> m_LastDay) then
			  begin
				m_LastDay := ToDay;
				WriteCache;
			  end;
			  if (m_CacheLen >= MAX_FILE_CACHE_SIZE) or
				(GetTickCount > m_LastWriteTick + WRITE_INTERVAL) then
				WriteCache;
			*/
		}
		catch (...)
		{
		}
		WaitForSingleObject(m_Event, 20);
	}
}

void CAuthFailFileLog::WriteLog(const std::string& sLog)
{

}

void CAuthFailFileLog::WriteCache()
{

}

/************************End Of CAuthFailFileLog****************************************************/
