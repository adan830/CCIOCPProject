/**************************************************************************************
@author: 陈昌
@content: 生成日志文件类
**************************************************************************************/

#include "CCFileLog.h"
#include "CCUtils.h"
#include <fstream>

namespace CC_UTILS{

	const int MAX_FILE_CACHE_SIZE = 8 * 1024;
	const int WRITE_INTERVAL = 300 * 1000;

/************************Start Of CCFileLog******************************************/

	CFileLogManager::CFileLogManager(const std::string &sLogName) : m_sName(sLogName), m_iCacheLen(0), m_uiLastWriteTick(0)
	{
		m_iLastDay = GetTodayNum();
		if ("" == sLogName)
			m_sPath = GetAppPathA() + "logs\\";
		else
			m_sPath = GetAppPathA() + "logs\\" + sLogName + "\\";
		m_pCache = (char*)malloc(MAX_FILE_CACHE_SIZE);
		try
		{
			ForceCreateDirectories(m_sPath);
		}
		catch (...)
		{
		}
	}

	CFileLogManager::~CFileLogManager()
	{
		WriteCache();
		free(m_pCache);
	}

	void CFileLogManager::WriteLog(const std::string &sLogStr)
	{
		int iTodayNum = GetTodayNum();
		if (iTodayNum != m_iLastDay)
		{
			m_iLastDay = iTodayNum;
			WriteCache();
		}
		int iStrLen = sLogStr.length();
		if ((m_iCacheLen + iStrLen >= MAX_FILE_CACHE_SIZE) || (_ExGetTickCount > m_uiLastWriteTick + WRITE_INTERVAL))
			WriteCache();
		{
			std::lock_guard<std::mutex> guard(m_LogCS);
			//超长的日志信息回被丢掉！！！！
			if ((iStrLen >= MAX_FILE_CACHE_SIZE) || (m_iCacheLen + iStrLen >= MAX_FILE_CACHE_SIZE))
				return;

			char* pBuf = m_pCache + m_iCacheLen;
			memcpy_s(pBuf, iStrLen, sLogStr.c_str(), iStrLen);
			m_iCacheLen += iStrLen;
		}		
	}

	void CFileLogManager::WriteCache()
	{
		if (m_iCacheLen > 0)
		{
			if (!IsFileExistsA(m_sPath.c_str()))
				ForceCreateDirectories(m_sPath);

			std::string sFileName = m_sPath + "RPSGameLog.txt";
			std::lock_guard<std::mutex> guard(m_LogCS);
			std::ofstream logFile(sFileName, std::ios::app);
			try
			{				
				if (logFile.is_open())
				{				
					logFile.write(m_pCache, m_iCacheLen);
					m_iCacheLen = 0;
				}		
				logFile.close();
			}
			catch (...)
			{
				m_iCacheLen = 0;
				logFile.close();
			}
			m_uiLastWriteTick = _ExGetTickCount;
		}
	}

/************************End Of CCFileLog******************************************/

}