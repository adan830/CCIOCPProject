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

/************************End Of CHumanDBManager******************************************/