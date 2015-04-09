/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#include "stdafx.h"
#include "CMainThread.h"
#include "CDBGlobal.h"

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
	if (G_ServerID < 1)
	{
		Log("请正确配置服务器编号", lmtWarning);
		Terminate();
		return;
	}
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

void CMainThread::ReceiveMessage(PInnerMsgNode pNode)
{
	pNode->pNext = nullptr;
	{
		std::lock_guard<std::mutex> guard(m_MsgCS);
		if (m_pLast != nullptr)
			m_pLast->pNext = pNode;
		else
			m_pFirst = pNode;
		m_pLast = pNode;
	}
	UpdateQueueCount(pNode->MsgFrom, true);
}

bool CMainThread::IsFilterWord(const std::string &sStr)
{
	bool bRetFlag = false;
	std::string sTemp;
	std::vector<std::string>::iterator vIter;
	for (vIter = m_FilterWords.begin(); vIter != m_FilterWords.end(); ++vIter)
	{
		sTemp = *vIter;
		if ((sStr.find(sTemp) != std::string::npos) || (sTemp.find(sStr) != std::string::npos))
		{
			bRetFlag = true;
			break;
		}
	}
	if (!bRetFlag)
		bRetFlag = IsEmptyChinese(sStr);
	return bRetFlag;
}

PNetBarIPInfo CMainThread::FindNetBarIP(const int iIP)
{
	return (PNetBarIPInfo)m_NoNeedActivateIPHash.ValueOf(iIP);
}

void CMainThread::SendFilterWords(TOnSendToServer CallBack)
{

}

bool CMainThread::IsAllowGuest()
{
	return m_bAllowGuest;
}

bool CMainThread::IsDenyRecharge()
{
	return m_bDenyRecharge;
}

void CMainThread::SetDenyRecharge(bool bFlag)
{

}

void CMainThread::ClearMsgQueue()
{
	PInnerMsgNode pNext;
	std::lock_guard<std::mutex> guard(m_MsgCS);
	m_pLast = nullptr;
	while (m_pFirst != nullptr)
	{
		pNext = m_pFirst->pNext;
		if (m_pFirst->pBuf)
			free(m_pFirst->pBuf);		 
		delete(m_pFirst);
		m_pFirst = pNext;
	}
}

void CMainThread::ProcDBMessage()
{
	PInnerMsgNode pCurWork;
	PInnerMsgNode pCurNext;
	{
		std::lock_guard<std::mutex> guard(m_MsgCS);
		pCurWork = m_pFirst;
		m_pFirst = nullptr;
		m_pLast = nullptr;
		if (nullptr == pCurWork)
			memset(m_iQueueCountList, 0, sizeof(m_iQueueCountList));
	}

	while (pCurWork)
	{
		pCurNext = pCurWork->pNext;
		try
		{
			switch (pCurWork->MsgFrom)			
			{
			case fromInternal:                               
				ProcInnerMessage(pCurWork);
				break;
			case fromAuthenServer:
				//G_AuthenSocket.ProcAuthenServerMessage(nWork);
				break;				
			case fromDispatchGate:
				//G_DispatchGate.ProcDispatchMessage(nWork);
				break;				
			case fromGameServer:
				//G_GSSocket.ProcGameServerMessage(nWork);
				break;
			case fromGameGate:
				//G_GGSocket.ProcGameGateMessage(nWork);
				break;				
			default:
				break;
			}
			if (pCurWork->pBuf != nullptr)
				free(pCurWork->pBuf);
			delete(pCurWork);

			pCurWork = pCurNext;
			UpdateQueueCount(pCurWork->MsgFrom, false);
		}
		catch (...)
		{
			Log("TMainThread.ProcDBMessage,MsgFrom=" + std::to_string(pCurWork->MsgFrom) + "  Ident=" + std::to_string(pCurWork->usIdent), lmtWarning);
			pCurWork = pCurNext;
			UpdateQueueCount(pCurWork->MsgFrom, false);
		}
	}
}

void CMainThread::CheckConfig(unsigned int uiTick)
{

}

void CMainThread::ProcInnerMessage(PInnerMsgNode pNode)
{
	int iReason = 0;
	switch (pNode->usIdent)
	{
	case SM_PLAYER_DISCONNECT:
		/*
          if Assigned(szBuf) and (wBufLen >= SizeOf(Integer)) then
            iReason := PInteger(szBuf)^
          else
            iReason := 0;
          G_GGSocket.KickOutClient(idx, Param, iReason);
		*/
		break;
	case SM_PLAYER_UPDATE_IDX:
		//G_HumanDB.UpdatePlayerDBIdx(idx, Param);
		break;
	default:
		break;
	}
}

void CMainThread::LoadFilterWord()
{
	std::string sFileName(GetAppPathA() + "FilterWord.txt");
	if (!IsFileExistsA(sFileName.c_str()))
		return;

	int iAge = CC_UTILS::GetFileAge(sFileName);
	if (iAge == m_iFilterFileAge)
		return;

	bool bReload = m_iFilterFileAge > 0;
	m_iFilterFileAge = iAge;
	/*
  TempList := TStringList.Create;
  try
    TempList.LoadFromFile(FileName);
    FFilterWords.Clear;
    for i := 0 to TempList.Count - 1 do
    begin
      FilterStr := Trim(TempList.Strings[i]);
      if FilterStr <> '' then
        FFilterWords.Add(FilterStr);
    end;
    if BoReLoad then
    begin
      Log(Format('Reload FilterWord.txt (%d)', [FFilterWords.Count]), lmtMessage);
      G_GGSocket.ReSendFilterWords;
    end;
  finally
    TempList.Free;
  end;
	*/
}

void CMainThread::OnAddLabel(void* Sender)
{
	m_pLogSocket->AddLabel("连接数：", 16, 8);
	m_pLogSocket->AddLabel("0", 64, 8, LABEL_CONNECT_COUNT_ID);
	m_pLogSocket->AddLabel("游戏人数：", 120, 8);
	m_pLogSocket->AddLabel("0", 184, 8, LABEL_PLAYER_COUNT_ID);
	m_pLogSocket->AddLabel("保存：", 224, 8, LABEL_SAVEQUEUE_COUNT_ID);
	m_pLogSocket->AddLabel("队列：", 16, 32, LABEL_MSGQUEUE_COUNT_ID);
	m_pLogSocket->AddLabel("图片缓存：", 224, 32);
	m_pLogSocket->AddLabel("0", 288, 32, LABEL_IMAGE_COUNT_ID);
}

void CMainThread::UpdateLabels()
{
	UpdateLabel(std::to_string(G_UserManage.Count), LABEL_CONNECT_COUNT_ID);
	UpdateLabel(std::to_string(G_GSSocket.HumanCount), LABEL_PLAYER_COUNT_ID);
	int iTotal = 0;
	std::string sCount("");
	for (int iMsgType = fromDispatchGate; iMsgType <= fromIMServer; iMsgType++)
	{
		iTotal += m_iQueueCountList[iMsgType];
		if ((fromGameServer == iMsgType) || (fromGameGate == iMsgType))
		{
			if ("" != sCount)
				sCount = sCount + "/";
			sCount = sCount + std::to_string(m_iQueueCountList[iMsgType]);
		}
	}
	sCount = sCount + "/" + std::to_string(iTotal);
	UpdateLabel("队列：" + sCount, LABEL_MSGQUEUE_COUNT_ID);
	/*
  with G_SaveThread do
    UpdateLabel(Format('保存：%d/%d/%d/%d', [
      QueueCount, SaveCount, SuccessCount, FailCount]),
        LABEL_SAVEQUEUE_COUNT_ID);
	*/
}

void CMainThread::UpdateQueueCount(TInnerMsgType msgType, bool bAdd)
{
	int iType = msgType;
	if (iType <= fromGameGate)
	{
		if (bAdd)
			InterlockedIncrement((LPLONG)&m_iQueueCountList[iType]);
		else
			InterlockedDecrement((LPLONG)&m_iQueueCountList[iType]);
	}
}

void CMainThread::OnRemoveNetBarIP(void* p, int iKey)
{
	if (p != nullptr)
		delete ((PNetBarIPInfo)p);
}

void CMainThread::LoadNetBarIPList()
{
	/*
var
  FileName, TempStr, sIP, sName: AnsiString;
  i, iage, nIP      : Integer;
  TempList          : TStringList;
  P                 : PNetBarIPInfo;
begin
  FileName := ExtractFilePath(ParamStr(0)) + 'NetBarIP.txt';
  if not FileExists(FileName) then
    Exit;
  iage := FileAge(FileName);
  if iage = FNetBarIPFileAge then
    Exit;
  if FNetBarIPFileAge > 0 then
    Log('Reload NetBarIP.txt....', lmtMessage);
  FNoNeedActivateIPList.Clear;
  FNetBarIPFileAge := iage;
  TempList := TStringList.Create;
  try
    TempList.LoadFromFile(FileName);
    for i := 0 to TempList.Count - 1 do
    begin
      TempStr := TempList.Strings[i];
      if (TempStr = '') or (TempStr[1] = ';') or (TempStr[1] = '#') then
        Continue;
      TempStr := GetValidStr3(TempStr, sIP, [' ', #9]);
      TempStr := GetValidStr3(TempStr, sName, [' ', #9]);
      nIP := inet_addr(PAnsiChar(sIP));
      P := FindNetBarIP(nIP);
      if not Assigned(P) then
      begin
        New(P);
        FillChar(P^, SizeOf(TNetBarIPInfo), 0);
        P^.nNetBarIP := nIP;
        StrPLCopy(P^.szName, sName, 64);
        FNoNeedActivateIPList.Add(nIP, P);
      end
      else
        Log(Format('%s %s重复', [sIP, sName]));
    end;
  finally
    TempList.Free;
  end;
end;
	*/
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