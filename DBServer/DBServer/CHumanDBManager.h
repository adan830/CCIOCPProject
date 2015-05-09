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

};


#endif //__CC_HUMAN_DB_MANAGER_H__