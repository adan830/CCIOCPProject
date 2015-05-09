/**************************************************************************************
@author: �²�
@content: �������ݵ�DB����������
**************************************************************************************/
#ifndef __CC_HUMAN_DB_MANAGER_H__
#define __CC_HUMAN_DB_MANAGER_H__

#include "stdafx.h"

const int MAX_JOB_NUM = 3;
const int MAX_BUFFER_DELAY_FREE_TIME = 24 * 60 * 1000;              //����ʱ��

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
* �Ѿ���ȡ���ݵ���Ҷ���
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
* �������ݵ�DB����������
*
*/
class CHumanDBManager
{

};


#endif //__CC_HUMAN_DB_MANAGER_H__