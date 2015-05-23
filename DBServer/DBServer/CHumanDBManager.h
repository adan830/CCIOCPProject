/**************************************************************************************
@author: 陈昌
@content: 人物数据的DB操作管理器
**************************************************************************************/
#ifndef __CC_HUMAN_DB_MANAGER_H__
#define __CC_HUMAN_DB_MANAGER_H__

#include "stdafx.h"

const int MAX_JOB_NUM = 3;
const int MAX_BUFFER_DELAY_FREE_TIME = 24 * 60 * 1000;              //缓存时间

typedef struct _TJobDataRec
{
	char* pBlobData;
	unsigned short usBlobSize;
}TJobDataRec, *PJobDataRec;

typedef struct _TDBDetailRec
{
	TRoleDetail RoleInfo;
	char* pShareBlobData;
	unsigned short usShareBlobSize;
	TJobDataRec JobDataList[MAX_JOB_NUM];
}TDBDetailRec, *PDBDetailRec;

typedef struct _TGMRoleInfo
{
    std::string sAccount;
    std::string sRoleName;
    int iGMLevel;
}TGMRoleInfo, *PGMRoleInfo;

/**
*
* 已经读取数据的玩家对象
*
*/
class CDBPlayer
{
public:
	CDBPlayer();
	virtual ~CDBPlayer();
	PDBDetailRec GetData();
	void UnlockData();
private:
	void Clear();
	bool Execute(unsigned int uiTick);
	void Update(PSavePlayerRec pInfo, char* pShareBlob, unsigned short usShareBlobLen, char* pJobBlob, unsigned short usJobBlobLen);
private:
	bool m_bOnline;	
	int m_iSession;
	TDBDetailRec m_Data;
	unsigned int m_uiLastReadTick;
	unsigned int m_uiDelayUnlockTick;
};

/**
*
* 人物数据的DB操作管理器
*
*/
class CHumanDBManager
{
public:
	CHumanDBManager();
	virtual ~CHumanDBManager();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	void Execute();
	void AddNewPlayer(TRoleDetail RoleDetail);
	int DBPlayer_Read(int iSessionID, int iDBIdx, unsigned char ucJob, PDBDetailRec pData);
	int DBPlayer_Save(int iSessionID, char* pBuf, unsigned short usBufLen);
	void DBPlayer_Guest(PRoleDetail pDetail);
	int DBPlayer_New(PRoleDetail pDetail, bool bForce = false);
	int DBPlayer_Logon(PRoleDetail pDetail, unsigned char ucSelJob = 0, bool bGM = false);
	void DBPlayer_UpdateAddress(int iAreaID, const std::string &sAccount, const std::string &sIP, const std::string &sMAC);
	void DBPlayer_UnLock(int iDBIdx, const std::string &sRoleName);
	int DBPlayer_QueryAccount(const std::string &sRoleName, PRoleDetail pDetail, int iAreaID);
	int DBPlayer_ReName(const std::string &sOldName, const std::string &sNewName);
	PGMRoleInfo FindGMInfo(const std::string &sAccount, const std::string &sRoleName);
	PGMRoleInfo FindGMByRoleName(const std::string &sRoleName);
	bool IsNeedReName(const std::string &sRoleName, std::string &sOldName);
	bool AddReNameToList(const std::string &sOldName, const std::string &sNewName, const int iAreaID);
	bool DelReNameFromList(const std::string &sRoleName);
	void OnRoleReName(int iDBIdx);
	void UpdatePlayerDBIdx(int iOldIdx, int iNewIdx);
	bool ProcYBRecharge(PYBCrushInfo pInfo);
	bool ProcGiveItem(PGiveItemInfo pInfo);
	bool GetMySQLConnected();
public:
	bool m_bDenyEnter;
	std::string m_sDenyEnterHint;
private:
	void MakeGameMaster();
	void CheckTables();
	void LoadGMList();
	void LoadReNameList();
	void GMListClear();
	void SaveRenameList();
	void OnDBError(unsigned int uiErrorCode, const std::string &sStr);
	bool DBPlayerExecute(void* p, unsigned int uiParam, int &iResult);
	void OnRemovePlayer(void* p, int iKey);
	void OnRemoveRename(void* p, const std::string &sKey);
	bool BeUsable(const std::string &sRoleName);
	int _ReadData(int iDbIdx, unsigned char ucSelJob, CDBPlayer* pPlayer); // 读取一个玩家数据
private:
	CC_UTILS::CMySQLManager* m_pMySQLProc;
	int m_iServerID;
	unsigned int m_uiGuestIndex;
	CC_UTILS::CIntegerHash m_OnlinePlayers;   // 以DBIDX为Key
	CC_UTILS::CStringHash m_RenameList;		  // 需要更名玩家列表
	unsigned int m_uiLastSaveTick;  

    m_GMList: TThreadList;    
};


#endif //__CC_HUMAN_DB_MANAGER_H__