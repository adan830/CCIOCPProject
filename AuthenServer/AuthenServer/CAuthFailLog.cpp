/**************************************************************************************
@author: 陈昌
@content: 认证错误日志单元
**************************************************************************************/
#include "stdafx.h"
#include "CAuthFailLog.h"

using namespace CC_UTILS;

CAuthFailFileLog* pG_AuthFailLog;
const int MAX_FILE_CACHE_SIZE = 8 * 1024;
const int WRITE_INTERVAL = 1000 * 300;

/************************Start Of CAuthFailFileLog**************************************************/
CAuthFailFileLog::CAuthFailFileLog() : m_ulLastWriteTick(0), m_iCacheLen(0)
{
	m_sPath = G_CurrentExeDir + "logs\\AuthFailLog\\";
	m_pCache = (char*)malloc(MAX_FILE_CACHE_SIZE);
	m_iLastDay = CC_UTILS::GetTodayNum();
	try
	{
		CC_UTILS::ForceCreateDirectories(m_sPath);
	}
	catch (...)
	{
	}
}

CAuthFailFileLog::~CAuthFailFileLog()
{
	WaitThreadExecuteOver();
	WriteCache();
	free(m_pCache);
}

void CAuthFailFileLog::DoExecute()
{
	int iTodayNum;
	while (!IsTerminated())
	{
		try
		{
			iTodayNum = CC_UTILS::GetTodayNum();
			if (iTodayNum != m_iLastDay)
			{
				m_iLastDay = iTodayNum;
				WriteCache();
			}
			if ((m_iCacheLen >= MAX_FILE_CACHE_SIZE) || (GetTickCount() > m_ulLastWriteTick + WRITE_INTERVAL))
				WriteCache();
		}
		catch (...)
		{
		}
		WaitForSingleObject(m_Event, 20);
	}
}

void CAuthFailFileLog::WriteLog(const std::string& sLog)
{
	std::lock_guard<std::mutex> guard(m_WriteCacheCS);
	int iBufLen = sLog.length();
	if ((iBufLen > MAX_FILE_CACHE_SIZE) || (m_iCacheLen + iBufLen > MAX_FILE_CACHE_SIZE))
		return;
	char* pBuf = &(m_pCache[m_iCacheLen]);
	memcpy_s(pBuf, MAX_FILE_CACHE_SIZE - m_iCacheLen, sLog.c_str(), sLog.length() + 1);
	m_iCacheLen = m_iCacheLen + iBufLen;
}

void CAuthFailFileLog::WriteCache()
{
	if (m_iCacheLen > 0)
	{
		if (_access(m_sPath.c_str(), 0) == -1)
			CC_UTILS::ForceCreateDirectories(m_sPath);

		//FormatDateTime('yyyymmdd', Now())
		std::string sFileName = m_sPath + "" + ".txt";
		{
			std::lock_guard<std::mutex> guard(m_WriteCacheCS);
			std::fstream file(sFileName, std::ios::in | std::ios::ate);
			try
			{
				if (file.is_open())
				{
					file.write(m_pCache, m_iCacheLen);
					file.close();
				}	
			}
			catch (...)
			{
				file.close();
			}		
		}
		m_iCacheLen = 0;
		m_ulLastWriteTick = GetTickCount();
	}
}

/************************End Of CAuthFailFileLog****************************************************/
