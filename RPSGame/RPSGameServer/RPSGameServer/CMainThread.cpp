/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/

#include "stdafx.h"
#include "CMainThread.h"
#include "CClientServerSocket.h"
#include "CCUtils.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread() : m_uiSlowRunTick(0), m_uiCheckConfigTick(0), m_iConfigFileAge(0)
{
	pG_GameSocket = new CClientServerSocket();
	m_pLogFile = new CFileLogManager("RPSServer");
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete pG_GameSocket;
	delete m_pLogFile;
}

void CMainThread::CheckConfig(const unsigned int uiTick)
{	
	if ((0 == m_uiCheckConfigTick) || (uiTick - m_uiCheckConfigTick >= 30 * 1000))
	{
		m_uiCheckConfigTick = uiTick;
		/*********************************************
		 for running without config, I just comment below
		 *********************************************
		std::string sConfigFileName("./config.ini");
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
				pG_GameSocket->LoadConfig(pIniFileParser);
				delete pIniFileParser;
			}
			catch (...)
			{
				delete pIniFileParser;
			}
		}
		*********************************************/
		pG_GameSocket->LoadConfig(nullptr);
	}
}

void CMainThread::DoExecute()
{
	pG_GameSocket->InitialWorkThread();

	Log("RPSGameServer Start...");
	unsigned int uiTick;
	while (!IsTerminated())
	{
		int iErrorCode = 1;
		try
		{
			uiTick = GetTickCount();
			if (uiTick - m_uiSlowRunTick >= 1000)
			{
				m_uiSlowRunTick = uiTick;
				CheckConfig(uiTick);			
			}
		}
		catch (...)
		{
			Log("CMainThread::DoExecute Exception!", lmtException);
		}
		WaitForSingleObject(m_Event, 10);
	}
	pG_GameSocket->Close();
}
/************************End Of CMainThread****************************************************/


void Log(const std::string& sInfo, byte loglv)
{
	SendDebugString(sInfo);
	if ((pG_MainThread != nullptr) && (pG_MainThread->m_pLogFile != nullptr))
		pG_MainThread->m_pLogFile->WriteLog(sInfo);	
}