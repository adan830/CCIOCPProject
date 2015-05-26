/**************************************************************************************
@author: 陈昌
@content: 人物数据的DB操作管理器
**************************************************************************************/
#include "stdafx.h"
#include "CHumanDBManager.h"

/************************Start Of CDBPlayer******************************************/

CDBPlayer::CDBPlayer() : m_bOnline(true), m_iSession(0), m_uiLastReadTick(0), m_uiDelayUnlockTick(0)
{
	memset(&m_Data, 0, sizeof(TDBDetailRec));
	Clear();
}

CDBPlayer::~CDBPlayer()
{
	Clear();
}

PDBDetailRec CDBPlayer::GetData()
{
	m_uiLastReadTick = GetTickCount();
	m_bOnline = true;
	return &m_Data;
}

void CDBPlayer::UnlockData()
{
	m_uiDelayUnlockTick = GetTickCount();
}

void CDBPlayer::Clear()
{
	char* pBuf = m_Data.pShareBlobData;
	m_Data.pShareBlobData = nullptr;
	m_Data.usShareBlobSize = 0;
	if (pBuf != nullptr)
		free(pBuf);

	for (int i = 0; i < MAX_JOB_NUM; i++)
	{
		pBuf = m_Data.JobDataList[i].pBlobData;
		m_Data.JobDataList[i].pBlobData = nullptr;
		m_Data.JobDataList[i].usBlobSize = 0;
		if (pBuf != nullptr)
			free(pBuf);
	}
}

bool CDBPlayer::Execute(unsigned int uiTick)
{
	bool bRetFlag = uiTick - m_uiLastReadTick >= MAX_BUFFER_DELAY_FREE_TIME;
	if (bRetFlag && m_bOnline)
	{
		//----------------------------
		//----------------------------
		//----------------------------
		//G_UserManage.RemoveUser(FSessionID);
		std::string sRoleName(m_Data.RoleInfo.szRoleName);
		Log(sRoleName + " Free.");
	}
	if ((m_uiDelayUnlockTick > 0) && (uiTick > m_uiDelayUnlockTick + 5000))
	{
		m_uiDelayUnlockTick = 0;
		m_bOnline = false;
	}
	return bRetFlag;
}

void CDBPlayer::Update(PSavePlayerRec pInfo, char* pShareBlob, unsigned short usShareBlobLen, char* pJobBlob, unsigned short usJobBlobLen)
{
	m_Data.RoleInfo = pInfo->Detail;
	if ((pShareBlob != nullptr) && (usShareBlobLen > 0))
	{
		if (m_Data.pShareBlobData != nullptr)
			free(m_Data.pShareBlobData);
		m_Data.usShareBlobSize = usShareBlobLen;
		m_Data.pShareBlobData = (char*)malloc(usShareBlobLen);
		memcpy(m_Data.pShareBlobData, pShareBlob, usShareBlobLen);
	}
	
	if ((pJobBlob != nullptr) && (usJobBlobLen > 0))
	{
		PJobDataRec pJob = &(m_Data.JobDataList[m_Data.RoleInfo.ucJob]);
		if (pJob->pBlobData != nullptr)
			free(pJob->pBlobData);
		pJob->usBlobSize = usJobBlobLen;
		pJob->pBlobData = (char*)malloc(usJobBlobLen);
		memcpy(pJob->pBlobData, pJobBlob, usJobBlobLen);
	}

	m_uiLastReadTick = GetTickCount();
	if ((DB_SAVE_QUIT == pInfo->ucSaveMode) || (DB_SAVE_CHG_JOB == pInfo->ucSaveMode))
	{
		m_bOnline = false;
		//-------------------------
		//-------------------------
		//-------------------------
		//G_UserManage.PlayerOffLine(PInfo^.Detail.szRoleName);
	}
	else
		m_bOnline = true;
}

/************************End Of CDBPlayer******************************************/


/************************Start Of CHumanDBManager******************************************/

CHumanDBManager::CHumanDBManager() : m_bDenyEnter(false), m_sDenyEnterHint(""), m_iServerID(0), m_uiGuestIndex(3001), m_OnlinePlayers(40961), m_RenameList(1023)
{
	m_pMySQLProc = new CC_UTILS::CMySQLManager();
	m_OnlinePlayers.Remove = std::bind(&CHumanDBManager::OnRemovePlayer, this, std::placeholders::_1, std::placeholders::_2);
	m_RenameList.Remove = std::bind(&CHumanDBManager::OnRemoveRename, this, std::placeholders::_1, std::placeholders::_2);
	m_uiLastSaveTick = GetTickCount();
	LoadReNameList();
}

CHumanDBManager::~CHumanDBManager()
{
	SaveRenameList();
	GMListClear();
	m_OnlinePlayers.Clear();
	m_RenameList.Clear();
	delete m_pMySQLProc;
}

void CHumanDBManager::LoadConfig(CWgtIniFile* pIniFileParser)
{
	LoadGMList();
	if (0 == m_iServerID)
	{
		m_iServerID = G_ServerID;
		std::string sHostName = pIniFileParser->getString("DataBase", "Host", DB_HOSTNAME);
		int iPort = pIniFileParser->getInteger("DataBase", "Port", DB_PORT);
		std::string sUserName = pIniFileParser->getString("DataBase", "User", DB_USERNAME);
		std::string sPassword = pIniFileParser->getString("DataBase", "Password", DB_PASSWORD);
		std::string sDatabase = G_DataBase;

		m_pMySQLProc->Connect(sHostName, sDatabase, sUserName, sPassword, "gbk", iPort);
		if (m_pMySQLProc->IsMySQLConnected())
			CheckTables();
	}
	m_bDenyEnter = pIniFileParser->getBoolean("Setup", "DenyEnter", false);
	m_sDenyEnterHint = pIniFileParser->getString("Setup", "DenyEnterHint", "目前服务器只开放预注册");
}

void CHumanDBManager::Execute()
{
	//-----------------------------
	//-----------------这里的绑定方法有问题，需要本对象 
	m_OnlinePlayers.Touch(DBPlayerExecute, GetTickCount);
	if ((m_uiLastSaveTick > 0) && (GetTickCount() > m_uiLastSaveTick + 1000 * 60))
	{
		SaveRenameList();
		m_uiLastSaveTick = 0;
	}
}

void CHumanDBManager::AddNewPlayer(TRoleDetail RoleDetail)
{
	CDBPlayer* pPlayer = new CDBPlayer();
	if (pPlayer->GetData() != nullptr)
	{
		TSavePlayerRec saveRec;
		memset((void*)(&saveRec), 0, sizeof(TSavePlayerRec));
		saveRec.Detail = RoleDetail;
		saveRec.ucSaveMode = DB_SAVE_NORMAL;
		saveRec.usShareBlobLen = 0;
		saveRec.usJobBlobLen = 0;
		pPlayer->Update(&saveRec, nullptr, 0, nullptr, 0);
		m_OnlinePlayers.Add(RoleDetail.iDBIndex, pPlayer);
	}
	else
		delete pPlayer;

}

int CHumanDBManager::DBPlayer_Read(int iSessionID, int iDBIdx, unsigned char ucJob, PDBDetailRec pData)
{
	int iRetCode = -1;
	if (ucJob > 0)
	{
		iRetCode = 1;
		CDBPlayer* pPlayer = (CDBPlayer*)m_OnlinePlayers.ValueOf(iDBIdx);
		if (pPlayer != nullptr)
		{
		}
		
	}
	/*
var
  Player            : TDBPlayer;
begin
  if bJob > 0 then
  begin
    Player := TDBPlayer(FonLinePlayers.ValueOf(DBIdx));
    if not Assigned(Player) then                            // 不在线
    begin
      _ReadData(DBIdx, bJob, Player);
      if not Assigned(Player) then
      begin
        Result := 3;                                        // DB数据读取错误
        Exit;
      end
      else
      begin
        FOnLinePlayers.Add(DBIdx, Player);
      end;
    end
    else if Player.FBoOnline then
    begin
      Result := 2;                                          //当前用户在线
      Exit;
    end                                                     //在线再判断所选职业是否有数据
    else if not Assigned(Player.FData.JobDataList[bJob].szBlobData) or (Player.FData.JobDataList[bJob].wBlobSize < 1) then
    begin
      _ReadData(DBIdx, bJob, Player);
    end;
    PData^ := Player.GetData^;
    Player.FSessionID := SessionID;
    Result := 1;
  end
  else
    Result := -1;
end;
	*/
}

int CHumanDBManager::DBPlayer_Save(int iSessionID, char* pBuf, unsigned short usBufLen)
{
}

void CHumanDBManager::DBPlayer_Guest(PRoleDetail pDetail)
{
}

int CHumanDBManager::DBPlayer_New(PRoleDetail pDetail, bool bForce)
{
}

int CHumanDBManager::DBPlayer_Logon(PRoleDetail pDetail, unsigned char ucSelJob, bool bGM)
{
}

void CHumanDBManager::DBPlayer_UpdateAddress(int iAreaID, const std::string &sAccount, const std::string &sIP, const std::string &sMAC)
{
}

void CHumanDBManager::DBPlayer_UnLock(int iDBIdx, const std::string &sRoleName)
{
}

int CHumanDBManager::DBPlayer_QueryAccount(const std::string &sRoleName, PRoleDetail pDetail, int iAreaID)
{
}

int CHumanDBManager::DBPlayer_ReName(const std::string &sOldName, const std::string &sNewName)
{
}

PGMRoleInfo CHumanDBManager::FindGMInfo(const std::string &sAccount, const std::string &sRoleName)
{
}

PGMRoleInfo CHumanDBManager::FindGMByRoleName(const std::string &sRoleName)
{
}

bool CHumanDBManager::IsNeedReName(const std::string &sRoleName, std::string &sOldName)
{
}

bool CHumanDBManager::AddReNameToList(const std::string &sOldName, const std::string &sNewName, const int iAreaID)
{
}

bool CHumanDBManager::DelReNameFromList(const std::string &sRoleName)
{
}

void CHumanDBManager::OnRoleReName(int iDBIdx)
{
}

void CHumanDBManager::UpdatePlayerDBIdx(int iOldIdx, int iNewIdx)
{
}

bool CHumanDBManager::ProcYBRecharge(PYBCrushInfo pInfo)
{
}

bool CHumanDBManager::ProcGiveItem(PGiveItemInfo pInfo)
{
}

bool CHumanDBManager::GetMySQLConnected()
{
}

void CHumanDBManager::MakeGameMaster()
{
}

void CHumanDBManager::CheckTables()
{
}

void CHumanDBManager::LoadGMList()
{
}

void CHumanDBManager::LoadReNameList()
{
}

void CHumanDBManager::GMListClear()
{
}

void CHumanDBManager::SaveRenameList()
{
}

void CHumanDBManager::OnDBError(unsigned int uiErrorCode, const std::string &sStr)
{
}

bool CHumanDBManager::DBPlayerExecute(void* p, unsigned int uiParam, int &iResult)
{
}

void CHumanDBManager::OnRemovePlayer(void* p, int iKey)
{
}

void CHumanDBManager::OnRemoveRename(void* p, const std::string &sKey)
{
}

bool CHumanDBManager::BeUsable(const std::string &sRoleName)
{
}

int CHumanDBManager::_ReadData(int iDbIdx, unsigned char ucSelJob, CDBPlayer* pPlayer)
{
}


/************************End Of CHumanDBManager******************************************/