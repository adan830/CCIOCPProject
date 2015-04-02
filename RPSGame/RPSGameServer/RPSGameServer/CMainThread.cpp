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
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete pG_GameSocket;
}

void CMainThread::CheckConfig(const unsigned int uiTick)
{	
	if ((0 == m_uiCheckConfigTick) || (uiTick - m_uiCheckConfigTick >= 30 * 1000))
	{
		m_uiCheckConfigTick = uiTick;
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

		}
		WaitForSingleObject(m_Event, 10);
	}
	pG_GameSocket->Close();
}
/************************End Of CMainThread****************************************************/


void Log(const std::string& sInfo, byte loglv)
{
	/*
	if ((pG_MainThread != nullptr) && (pG_MainThread->m_pLogSocket != nullptr))
		pG_MainThread->m_pLogSocket->SendLogMsg(sInfo, loglv);
	*/
	SendDebugString(sInfo);
}