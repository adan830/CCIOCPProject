/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName) : m_ulSlowRunTick(0), m_ulCheckConfigTick(0), m_iConfigFileAge(0), m_pLogSocket(nullptr)
{
	m_pLogSocket = new CC_UTILS::CLogSocket(sServerName);
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
}

void CMainThread::CheckConfig(const unsigned long ulTick)
{	
	if ((0 == m_ulCheckConfigTick) || (ulTick - m_ulCheckConfigTick >= 30 * 1000))
	{
		m_ulCheckConfigTick = ulTick;	
		std::string sConfigFileName(G_CurrentExeDir + "config.ini");
		int iAge = GetFileAge(sConfigFileName);
		if ((iAge != -1) && (iAge != m_iConfigFileAge))
		{
			if (m_iConfigFileAge > 0)
				Log("Reload Config File...", lmtMessage);

			m_iConfigFileAge = iAge;
			CWgtIniFile* pIniFileParser = new CWgtIniFile();
			pIniFileParser->loadFromFile(sConfigFileName);
			try
			{
				delete pIniFileParser;
			}
			catch (...)
			{
				delete pIniFileParser;
			}
		}
	}
}

void CMainThread::DoExecute()
{
	m_pLogSocket->InitialWorkThread();

	Log("AuthenServer 启动.");
	unsigned long ulTick;
	while (!IsTerminated())
	{
		int iErrorCode = 1;
		try
		{
			ulTick = GetTickCount();
			if (ulTick - m_ulSlowRunTick >= 1000)
			{
				m_ulSlowRunTick = ulTick;
				CheckConfig(ulTick);
				
			}
		}
		catch (...)
		{

		}
		WaitForSingleObject(m_Event, 10);
	}
}
/************************End Of CMainThread****************************************************/


void Log(const std::string& sInfo, byte loglv)
{
	if ((pG_MainThread != nullptr) && (pG_MainThread->m_pLogSocket != nullptr))
		pG_MainThread->m_pLogSocket->SendLogMsg(sInfo, loglv);
}