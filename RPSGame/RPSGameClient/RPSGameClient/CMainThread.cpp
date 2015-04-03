/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/

#include "stdafx.h"
#include "CMainThread.h"
#include "CGameClientSocket.h"
#include "CCUtils.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread() : m_uiSlowRunTick(0)
{
	pG_ClientSocket = new CGameClientSocket();
	srand(time(0));
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete pG_ClientSocket;
}

void CMainThread::DoExecute()
{
	pG_ClientSocket->InitialWorkThread();

	Log("RPSGame Client Start...");
	unsigned int uiTick;
	while (!IsTerminated())
	{
		try
		{
			uiTick = GetTickCount();
			if (uiTick - m_uiSlowRunTick >= 1000)
			{
				m_uiSlowRunTick = uiTick;
				pG_ClientSocket->DoHeartBeat();
				pG_ClientSocket->PlayGame();
			}
		}
		catch (...)
		{

		}
		WaitForSingleObject(m_Event, 10);
	}
	pG_ClientSocket->Close();
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