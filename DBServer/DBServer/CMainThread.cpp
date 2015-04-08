/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"

using namespace CC_UTILS;

CMainThread* pG_MainThread;

/************************Start Of CMainThread**************************************************/
CMainThread::CMainThread(const std::string &sServerName) : m_uiSlowTick(0), m_pFirst(nullptr), m_pLast(nullptr), m_iNetBarIPFileAge(0), m_iConfigFileAge(0),
m_iFilterFileAge(0), m_uiLastCheckTick(0), m_NoNeedActivateIPHash(3111), m_FilterWords(512), m_bAllowGuest(false), m_bDenyRecharge(false)
{
	m_NoNeedActivateIPHash.Remove = std::bind(&CMainThread::OnRemoveNetBarIP, this, std::placeholders::_1, std::placeholders::_2);	
	/*
  FNoNeedActivateIPList.OnRemove := OnRemoveNetBarIP;
  m_LogSocket := TLogSocket.Create(ServiceName);
  m_LogSocket.OnConnect := OnAddLabel;
  G_DynImageQueue := TDynImageQueue.Create;
  G_DBSecurity := TDBSecurity.Create;
  G_HumanDB := TDBManager.Create;
  G_GSSocket := TGSSocket.Create;
  G_DispatchGate := TDispatchGate.Create;
  G_AuthenSocket := TAuthenSocket.Create;
  G_GGSocket := TGGSocket.Create;
  G_UserManage := TUserManage.Create;
  G_SaveThread := TSaveThread.Create;
	*/
	mmTimer.Initialize(1);	
}

CMainThread::~CMainThread()
{
	WaitThreadExecuteOver();
	/*
  ClearMsgQueue;
  FreeObject(G_GGSocket);
  FreeObject(G_GSSocket);
  FreeObject(G_DispatchGate);
  FreeObject(G_AuthenSocket);
  FreeObject(G_HumanDB);
  FreeObject(G_UserManage);
  FreeObject(G_SaveThread);
  FreeObject(G_DBSecurity);
  FreeObject(G_DynImageQueue);
  FFilterWords.Free;
  FNoNeedActivateIPList.Free;
  FreeObject(m_LogSocket);
  DeleteCriticalSection(FMsgCS);
	*/
	mmTimer.Finalize();
}

void CMainThread::DoExecute()
{
	LoadNetBarIPList();
	CheckConfig(_ExGetTickCount);
	/*
var
  Tick, LastTick    : Cardinal;
  ErrCode           : Integer;
begin
  LoadNetBarIPList;
  CheckConfig(GetTickCount);
  if (G_ServerID < 1) then
  begin
    Log('请正确配置服务器编号', lmtWarning);
    Terminate;
    Exit;
  end;
  while not G_HumanDB.Connected do
  begin
    if Terminated then
      Exit;
    WaitForSingleObject(FDelayEvent, 10 * 1000);
  end;
  G_SaveThread.Resume;
  while not Terminated do
  begin
    ErrCode := 1;
    try
      Tick := GetTickCount;
      if Tick - FSlowerTick >= 1000 then                    // 慢速执行
      begin
        CheckConfig(Tick);
        FSlowerTick := Tick;
        ErrCode := 2;
        G_HumanDB.Execute;
        ErrCode := 3;
        G_DBSecurity.Execute;
        ErrCode := 4;
        G_UserManage.Execute;
        ErrCode := 5;
        UpdateLabels;
        G_AuthenSocket.DoHeartbeat;
        G_DispatchGate.DoHeartbest;
      end;
      ErrCode := 6;
      ProcDBMessage;
      if GetTickCount - Tick >= 25 then
        Continue;
    except
      on E: Exception do
        Log('TMainThread.Execute,ErrCode=' + IntToStr(ErrCode) + '  ' + E.Message, lmtWarning);
    end;
    WaitForSingleObject(FDelayEvent, 1);
  end;
  G_BoClose := True;
  while not G_DispatchGate.Closed do
  begin
    WaitForSingleObject(FDelayEvent, 300);
    ProcDBMessage;
  end;
  while not G_AuthenSocket.Closed do
  begin
    WaitForSingleObject(FDelayEvent, 300);
    ProcDBMessage;
  end;
  G_GGSocket.Close;
  LastTick := GetTickCount;
  while GetTickCount < LastTick + 3000 do
  begin
    ProcDBMessage;
    if G_GSSocket.CanClosed then
      Break;
    WaitForSingleObject(FDelayEvent, 100);
  end;
  G_GSSocket.Close;
  WaitForSingleObject(FDelayEvent, 300);
  G_SaveThread.Close;
  WaitForSingleObject(FDelayEvent, 300);
  Log('DBServer 正常退出', lmtWarning);
  G_HumanDB.Execute;
  WaitForSingleObject(FDelayEvent, 300);
end;
	*/
}

void CMainThread::StartLogSocket(int idx)
{}

void CMainThread::ReceiveMessage(PInnerMsgNode pNode)
{}

bool CMainThread::IsFilterWord(const std::string &sStr)
{}

PNetBarIPInfo CMainThread::FindNetBarIP(const int iIP)
{}

void CMainThread::SendFilterWords(TOnSendToServer CallBack)
{}

bool CMainThread::IsAllowGuest()
{}

bool CMainThread::IsDenyRecharge()
{}

void CMainThread::SetDenyRecharge(bool bFlag)
{}

void CMainThread::ClearMsgQueue()
{}

void CMainThread::ProcDBMessage()
{}

void CMainThread::CheckConfig(unsigned int uiTick)
{}

void CMainThread::ProcInternalMessage(PInnerMsgNode pNode)
{}

void CMainThread::LoadFilterWord()
{}

void CMainThread::OnAddLabel(void* Sender)
{}

void CMainThread::UpdateLabels()
{}

void CMainThread::UpdateQueueCount(TInnerMsgType msgType, bool bAdd)
{}

void CMainThread::OnRemoveNetBarIP(void* p, int iKey)
{}

void CMainThread::LoadNetBarIPList()
{}

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