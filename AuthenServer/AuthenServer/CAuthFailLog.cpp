/**************************************************************************************
@author: �²�
@content: ��֤������־��Ԫ
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
