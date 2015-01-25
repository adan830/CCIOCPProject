/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName) : m_ulSlowRunTick(0), m_pLogSocket(nullptr)
{
	m_pLogSocket = new CC_UTILS::CLogSocket(sServerName);
	/*
	m_LogSocket.OnConnect := OnAddLabel;


	G_SQLInterFace := TSQLDBInterface.Create;
	G_ServerSocket := TServerSocket.Create(ServerName);
	G_ChildManager := TChildManager.Create(OnChildNotify);
	G_IWebSocket := TIWebSocket.Create;
	G_RechargeManager := TRechargeManager.Create;
	G_ReportManager := TReportManager.Create;
	G_GiveItemManager := TGiveItemManager.Create;
	G_AuthenSecure := TAuthenSecure.Create;
	G_AuthFailLog := TAuthFailFileLog.Create;
	*/
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	/*
	FreeAndnil(G_ChildManager);
	FreeObject(G_ServerSocket);
	FreeObject(G_SQLInterFace);
	FreeObject(G_RechargeManager);
	FreeObject(G_GiveItemManager);
	FreeObject(G_IWebSocket);
	FreeObject(G_ReportManager);
	FreeObject(G_AuthenSecure);
	FreeObject(m_LogSocket);
	FreeObject(G_AuthFailLog);
	*/
}

void CMainThread::DoExecute()
{
	m_pLogSocket->InitialWorkThread();
	Log("AuthenServer 启动.");
	//-------------------
	//-------------------
	//G_ServerSocket.Open;
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

				//---------------------------------------
				//---------------------------------------
				/*
				m_pLogSocket->UpdateLabel(std::string(G_SQLInterFace.Count), LABEL_POOL_COUNT_ID);
				G_IWebSocket.DoHeartbest;
				G_ReportManager.Run(Tick);
				G_AuthenSecure.Execute();
				m_LogSocket.UpdateLabel(Format('%d/%d/%d', [G_AuthenSecure.LoginFailCount, G_AuthenSecure.RegistFailCount, G_AuthenSecure.MacFailCount]), LABEL_SECURE_COUNT_ID);
				*/
			}
		}
		catch (...)
		{
			Log("TMainThread.Execute,ErrCode=" + std::to_string(GetLastError()), lmtException);
		}
		WaitForSingleObject(m_Event, 10);
	}
	//--------------------
	//--------------------
	//G_ServerSocket.Close;
}

void CMainThread::OnAddLabel(void* Sender)
{
	m_pLogSocket->AddLabel("Secure：", 16, 8);                 
	m_pLogSocket->AddLabel("0", 88, 8, LABEL_SECURE_COUNT_ID);
	m_pLogSocket->AddLabel("SQL Pool：", 16, 30);
	m_pLogSocket->AddLabel("0", 88, 30, LABEL_POOL_COUNT_ID);
	m_pLogSocket->AddLabel("Children:", 180, 30);
	m_pLogSocket->AddLabel("0", 248, 30, LABEL_CHILD_COUNT_ID);
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