/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName)
{
	mmTimer.Initialize(1);
	m_pLogSocket = new CLogSocket("");
	m_pLogSocket->m_OnConnectEvent = std::bind(&CMainThread::OnAddLabel, this, std::placeholders::_1);
	/*
	//-------------------------
	//-------------------------
	//-------------------------
	G_ServerSocket := TServerSocket.Create;
	*/
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete m_pLogSocket;
	mmTimer.Finalize();
	/*
	//---------------------
	//---------------------
	//---------------------
	G_ServerSocket.Free;
	*/
}

void CMainThread::DoExecute()
{
	m_pLogSocket->InitialWorkThread();
	//-----------------
	//-----------------
	//G_ServerSocket->InitialWorkThread();

	Log("GameGate 启动.");

	while (!IsTerminated())
	{
		WaitForSingleObject(m_Event, 100);
	}
}

void CMainThread::OnAddLabel(void* Sender)
{
	/*
	//------------------------
	//------------------------
	  with m_LogSocket do
	  begin
		if Assigned(G_ServerSocket) then
		  AddLabel('Port:' + IntToStr(G_ServerSocket.Port), 16, 12, LABEL_PORT_ID)
		else
		  AddLabel('Port: 0', 16, 12, LABEL_PORT_ID);
		AddLabel('Connect: 0', 179, 12, LABEL_CONNECT_ID);
		AddLabel('RUN: 0', 16, 37, LABEL_RUN_ID);
	  end;
	*/
}

void CMainThread::StartLogSocket(int idx)
{
	std::string sServerName;
	if (idx < 10)
		sServerName = "GameGate-0" + std::to_string(idx);
	else
		sServerName = "GameGate-" + std::to_string(idx);

	m_pLogSocket->SetServiceName(sServerName);
	if (G_BoCheckGPS)
		Log(sServerName + " 启用反外挂", lmtWarning);
	else
		Log(sServerName + " 未启用反外挂", lmtWarning);
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

void TracertPackage(const std::string& sRoleName, char* pBuf, unsigned short usBufLen)
{
	if ((sRoleName != "") && (pG_MainThread != nullptr) && (pG_MainThread->m_pLogSocket != nullptr))
		pG_MainThread->m_pLogSocket->SendTracerData(sRoleName, pBuf, usBufLen);
}