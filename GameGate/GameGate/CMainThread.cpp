/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"
#include "CClientServerSocket.h"
#include "CGSClientSocket.h"
#include "CDBClientSocket.h"
#include "CIMClientSocket.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName)
{
	mmTimer.Initialize(1);
	G_LocalGateIdx = 1;
	m_pLogSocket = new CLogSocket("");
	m_pLogSocket->m_OnConnectEvent = std::bind(&CMainThread::OnAddLabel, this, std::placeholders::_1);
	pG_ClientServerSocket = new CClientServerSocket();
	pG_DBServer = new CDBClientSocket();
	pG_GameServer = new CGSClientSocket();
	pG_IMServer = new CIMClientSocket();
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	delete pG_IMServer;
	delete pG_GameServer;
	delete pG_DBServer;
	delete pG_ClientServerSocket;
	delete m_pLogSocket;
	mmTimer.Finalize();
}

void CMainThread::DoExecute()
{
	m_pLogSocket->InitialWorkThread();
	pG_ClientServerSocket->InitialWorkThread();
	pG_DBServer->InitialWorkThread();
	pG_GameServer->InitialWorkThread();
	pG_IMServer->InitialWorkThread();
	Log("GameGate 启动.");

	while (!IsTerminated())
	{
		WaitForSingleObject(m_Event, 100);
	}
	pG_ClientServerSocket->Close();
	pG_DBServer->Close();
	pG_GameServer->Close();
	pG_IMServer->Close();
}

void CMainThread::OnAddLabel(void* Sender)
{
	if (pG_ClientServerSocket != nullptr)
		m_pLogSocket->AddLabel("Port:" + std::to_string(pG_ClientServerSocket->m_iListenPort), 16, 12, LABEL_PORT_ID);
	else
		m_pLogSocket->AddLabel("Port: 0", 16, 12, LABEL_PORT_ID);

	m_pLogSocket->AddLabel("Connect: 0", 179, 12, LABEL_CONNECT_ID);
	m_pLogSocket->AddLabel("RUN: 0", 16, 37, LABEL_RUN_ID);
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