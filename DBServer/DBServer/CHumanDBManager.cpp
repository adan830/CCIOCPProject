/**************************************************************************************
@author: 陈昌
@content: 人物数据的DB操作管理器
**************************************************************************************/
#include "stdafx.h"
#include "CHumanDBManager.h"

using namespace CC_UTILS;
const std::string GUEST_HUMAN_NAME = "游客";

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
	m_OnlinePlayers.Touch(DBPlayerExecute, GetTickCount());
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
		CDBPlayer* pPlayer = (CDBPlayer*)m_OnlinePlayers.ValueOf(iDBIdx);
		if (nullptr == pPlayer)
		{
			_ReadData(iDBIdx, ucJob, pPlayer);
			if (nullptr == pPlayer)
			{
				//DB数据读取错误
				iRetCode = 3;
				return iRetCode;
			}
			else
			{
				m_OnlinePlayers.Add(iDBIdx, pPlayer);
			}
		}
		else if (pPlayer->m_bOnline)
		{
			//已在线
			iRetCode = 2;
			return iRetCode;
		}
		else if ((nullptr == pPlayer->m_Data.JobDataList[ucJob].pBlobData) || (0 == pPlayer->m_Data.JobDataList[ucJob].usBlobSize))
		{
			_ReadData(iDBIdx, ucJob, pPlayer);
		}

		pData = pPlayer->GetData();
		pPlayer->m_iSession = iSessionID;
		iRetCode = 1;
	}
	else
		iRetCode = -1;

	return iRetCode;
}

int CHumanDBManager::DBPlayer_Save(int iSessionID, char* pBuf, unsigned short usBufLen)
{
	int iRetCode = 0;
	PSavePlayerRec pData = nullptr;
	__try
	{
		if (usBufLen > sizeof(TSavePlayerRec))
		{
			pData = (PSavePlayerRec)pBuf;
			int iDBIndex = pData->Detail.iDBIndex;
			if (0 == iDBIndex)
			{
				int iRet = DBPlayer_New(&(pData->Detail), false);
				if ((iRet != 1) && (iRet != 3))
				{
					Log("PData^.DBIndex = 0", lmtWarning);
					return iRetCode;
				}				
			}
			iRetCode = 3;
			if (usBufLen >= sizeof(TSavePlayerRec)+pData->usShareBlobLen + pData->usJobBlobLen)
			{
				if (iDBIndex > 0)
				{
					CDBPlayer* pPlayer = (CDBPlayer*)m_OnlinePlayers.ValueOf(pData->Detail.iDBIndex);
					if (nullptr == pPlayer)
					{
						//不在线
						_ReadData(iDBIndex, pData->Detail.ucJob, pPlayer);
						if (nullptr == pPlayer)
						{
							//读取失败
							iRetCode = 2;
							return iRetCode;
						}
						m_OnlinePlayers.Add(iDBIndex, pPlayer);
					}

					char* pShareData = pBuf + sizeof(TSavePlayerRec);
					char* pJobData = pBuf + sizeof(TSavePlayerRec) + pData->usShareBlobLen;
					pPlayer->Update(pData, pShareData, pData->usShareBlobLen, pJobData, pData->usJobBlobLen);

					//---------------------------
					//---------------------------
					//---------------------------
					//if G_SaveThread.AddSaveNode(@PData^.Detail, ShareData, PData^.ShareBlobLen, JobData, PData^.JobBlobLen) then
					{
						iRetCode = 1;
						if ((DB_SAVE_CHG_JOB == pData->ucSaveMode) && (pData->ucChgJob > 0))
						{
							/*
							User := G_UserManage.FindUser(SessionID);
							if Assigned(User) then
								User.EnterIntoGame(PData^.ChgJob, True)
							else
								Log(Format('切换职业无法找到User对象,RoleName: %s DBIndex: %d ChgJob：%d !', [PData^.Detail.szRoleName, PData^.Detail.DBIndex, PData^.ChgJob]), lmtError);
							*/
						}
					}
				}
			}
		}
	}
	__finally
	{
		if ((iRetCode != 1) && (pData != nullptr))
			Log(CC_UTILS::FormatStr("RoleName: %s DBIndex: %d ErrCode=%d write Error.", pData->Detail.szRoleName, pData->Detail.iDBIndex, iRetCode), lmtError);
		return iRetCode;
	}
}

void CHumanDBManager::DBPlayer_Guest(PRoleDetail pDetail)
{
	/*
var
  GuestName         : ansistring;
begin
  inc(FGuestIndex);
  GuestName := GUEST_HUMAN_NAME + IntToHex(FGuestIndex, 10);
  StrPLCopy(P^.szRoleName, GuestName, ACTOR_NAME_MAX_LEN - 1);
  GuestName := '#' + inttoHex(FGuestIndex, 10);
  StrPLCopy(P^.szAccount, GuestName, ACCOUNT_MAX_LEN - 1);
  P^.DBIndex := 0;
end;
	*/
}

//1:成功 2：创建失败 3：每个区只能创建1个角色 4：角色名已经存在 
//-1:所选区组错误 -2：角色名长度不足 -3：角色名包含非法字符 -4:角色名长度太长 -5：数据库操作异常
int CHumanDBManager::DBPlayer_New(PRoleDetail pDetail, bool bForce)
{
	int iRetCode = 0;
	int iLen = strlen(pDetail->szRoleName);
	if (pDetail->iAreaID < 1)
		iRetCode = -1;
	else if (iLen < 4)
		iRetCode = -2;
	else if (iLen > 14)
		iRetCode = -4;
	else if ((!bForce) && (!BeUsable(pDetail->szRoleName)))
		iRetCode = -3;
	else
	{
		IMySQLFields* pDataSet = nullptr;
		IMySQLFields** ppDataSet = &pDataSet;
		int iAffectNum = 0;
		std::string sSQL = CC_UTILS::FormatStr("call sp_insertuser(\"%s\", \"%s\", %d, %d, %d);", pDetail->szAccount, pDetail->szRoleName, pDetail->ucJob, pDetail->ucGender, pDetail->iAreaID);
		if (m_pMySQLProc->Exec(sSQL, ppDataSet, iAffectNum) && (1 == iAffectNum))
		{
			iRetCode = pDataSet->FieldByName("retcode")->AsInteger();
			pDetail->iDBIndex = pDataSet->FieldByName("dbindex")->AsInteger();
		}
		else
			iRetCode = -5;
	}
	return iRetCode;
}

int CHumanDBManager::DBPlayer_Logon(PRoleDetail pDetail, unsigned char ucSelJob, bool bGM)
{
	int iRetCode = -1;

	return iRetCode;
	/*
var
  SQL, RoleName, FieldName: ansistring;
  RecCount          : Integer;
  ErrorCode         : Cardinal;
  DataSet           : IMySQLFields;
  Player            : TDBPlayer;
begin
  Result := -1;                                             // Error or IsaNewbie
  with P^ do
  begin
    if AreaID > 0 then
    begin
      if bGM then                                           //GM则不限制区组，只根据帐号获取即可
        SQL := Format('select idx, Job, sex, RoleName,AreaID,warriorLv,wizardLv,paladinLv,LastOnLineTime,OnLineTime,CreateTime from user_detail where IsDelete=0 and Account="%s" limit 1;',
          [szAccount])
      else
        SQL := Format('select idx, Job, sex, RoleName,AreaID,warriorLv,wizardLv,paladinLv,LastOnLineTime,OnLineTime,CreateTime from user_detail where IsDelete=0 and AreaId=%d and Account="%s";',
          [AreaID, szAccount]);
      if FMySQLBase.Exec(SQL, DataSet, RecCount) then
      begin
        if RecCount > 0 then
        begin
          DBIndex := DataSet.FieldByName('Idx').AsInteger;
          bGender := DataSet.FieldByName('sex').AsInteger;
          RoleName := DataSet.FieldByName('RoleName').AsString;
          nLastOnLineTime := DataSet.FieldByName('LastOnLineTime').AsInteger;
          nOnLineTime := DataSet.FieldByName('OnLineTime').AsInteger;
          dwCreateTime := TimeToSecond(DataSet.FieldByName('CreateTime').AsDateTime);
          StrPLCopy(szRoleName, RoleName, ACTOR_NAME_MAX_LEN - 1);
          if bGM then                                       //GM ip则纠正其区号
            AreaID := DataSet.FieldByName('AreaID').AsInteger;
          Player := FonLinePlayers.ValueOf(DBIndex);
          if bSelJob = 0 then
          begin
            //存在缓存则使用缓存
            if Assigned(Player) then
              bJob := Player.FData.RoleInfo.bJob
            else
              bJob := DataSet.FieldByName('Job').AsInteger;
          end
          else
            bJob := bSelJob;
          wLevel := DataSet.FieldByName(GetJobLvFieldName(bJob)).AsInteger;
          FieldName := GetJobDataFieldName(bJob);
          Result := 1;
        end
        else if RecCount = 0 then
          Result := 0;
      end
      else
        Log(FMySQLBase.GetLastError(ErrorCode), lmtError);
    end;
  end;
end;
	*/
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