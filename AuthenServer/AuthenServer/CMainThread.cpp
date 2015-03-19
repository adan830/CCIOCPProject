/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"
#include "CDBServerSocket.h"
#include "CRechargeManager.h"
#include "CGiveItemManager.h"
#include "CSQLDBManager.h"
#include "CHumanReportManager.h"
#include "CSecureManager.h"
#include "CAuthFailLog.h"
#include "CIWebClientSocket.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName) : m_uiSlowRunTick(0), m_pLogSocket(nullptr)
{
	m_pLogSocket = new CC_UTILS::CLogSocket(sServerName);
	m_pLogSocket->m_OnConnectEvent = std::bind(&CMainThread::OnAddLabel, this, std::placeholders::_1);
	pG_SQLDBManager = new CSQLDBManager;
	pG_DBSocket = new CDBServerSocket(sServerName);
	pG_IWebSocket = new CIWebClientSocket();
	pG_RechargeManager = new CRechargeManager();
	pG_GiveItemManager = new CGiveItemManager();
	pG_HumanReportManager = new CHumanReportManager();
	pG_SecureManager = new CSecureManager();
	pG_AuthFailLog = new CAuthFailFileLog();
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete pG_DBSocket;
	delete pG_SQLDBManager;
	delete pG_RechargeManager;
	delete pG_GiveItemManager;
	delete pG_IWebSocket;
	delete pG_HumanReportManager;
	delete pG_SecureManager;
	delete m_pLogSocket;
	delete pG_AuthFailLog;
}

void CMainThread::DoExecute()
{
	m_pLogSocket->InitialWorkThread();
	pG_DBSocket->InitialWorkThread();
	pG_AuthFailLog->InitialWorkThread();
	pG_IWebSocket->InitialWorkThread();

	Log("AuthenServer 启动.");
	pG_DBSocket->Open();
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
				m_pLogSocket->UpdateLabel(std::to_string(pG_SQLDBManager->GetPoolCount()), LABEL_POOL_COUNT_ID);
				pG_IWebSocket->DoHeartBeat();
				pG_HumanReportManager->Run(uiTick);
				pG_SecureManager->Execute();
				int iCount1 = pG_SecureManager->GetLoginFailCount();
				int iCount2 = pG_SecureManager->GetRegistFailCount();
				int iCount3 = pG_SecureManager->GetMacFailCount();
				m_pLogSocket->UpdateLabel(std::to_string(iCount1) + std::to_string(iCount2) + std::to_string(iCount3), LABEL_SECURE_COUNT_ID);
			}
		}
		catch (...)
		{
			Log("TMainThread.Execute,ErrCode=" + std::to_string(GetLastError()), lmtException);
		}
		WaitForSingleObject(m_Event, 10);
	}
	pG_DBSocket->Close();
}

void CMainThread::OnAddLabel(void* Sender)
{
	try
	{	
		m_pLogSocket->AddLabel("Secure：", 16, 8);                 
		m_pLogSocket->AddLabel("0", 88, 8, LABEL_SECURE_COUNT_ID);
		m_pLogSocket->AddLabel("SQL Pool：", 16, 30);
		m_pLogSocket->AddLabel("0", 88, 30, LABEL_POOL_COUNT_ID);
		m_pLogSocket->AddLabel("Children:", 180, 30);
		m_pLogSocket->AddLabel("0", 248, 30, LABEL_CHILD_COUNT_ID);
	}
	catch (...)
	{
		Log("CMainThread::OnAddLabel,ErrCode=" + std::to_string(GetLastError()), lmtException);
	}
}
/************************End Of CMainThread****************************************************/

void Log(const std::string& sInfo, byte loglv)
{
	if ((pG_MainThread != nullptr) && (pG_MainThread->m_pLogSocket != nullptr))
		pG_MainThread->m_pLogSocket->SendLogMsg(sInfo, loglv);
}

void UpdateLabel(const std::string& sDesc, int iTag)
{
	if ((pG_MainThread != nullptr) && (pG_MainThread->m_pLogSocket != nullptr))
		pG_MainThread->m_pLogSocket->UpdateLabel(sDesc, iTag);
}