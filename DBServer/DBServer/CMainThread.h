/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#ifndef __CC_DBSERVER_MAIN_THREAD_H__
#define __CC_DBSERVER_MAIN_THREAD_H__

#include "stdafx.h"
#include "CCHashClass.h"
#include <mutex>

const int LABEL_CONNECT_COUNT_ID = 1;
const int LABEL_PLAYER_COUNT_ID = 2;
const int LABEL_IMAGE_COUNT_ID = 3;
const int LABEL_MSGQUEUE_COUNT_ID = 4;
const int LABEL_SAVEQUEUE_COUNT_ID = 5;

enum TStateType{
	stMsgQueue = 0, 
	stSaveCount, 
	stSaveSuccess, 
	stSaveFail, 
	stSaveQueue
};

typedef struct _TNetBarIPInfo
{
	int iNetBarIP;
	char szName[64];
}TNetBarIPInfo, *PNetBarIPInfo;

typedef std::function<bool(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)> TOnSendToServer;

/**
*
* CExecutableBase的子类----主线程(无子类继承，DoExecute不作为虚函数)
*
*/
class CMainThread : public CExecutableBase
{
public:
	CMainThread(const std::string &sServerName);
	virtual ~CMainThread();
	void DoExecute();
	void ReceiveMessage(PInnerMsgNode pNode);
	bool IsFilterWord(const std::string &sStr);
	PNetBarIPInfo FindNetBarIP(const int iIP);
	void SendFilterWords(TOnSendToServer CallBack);
	bool IsAllowGuest();
	bool IsDenyRecharge();
	void SetDenyRecharge(bool bFlag);
public:
	CC_UTILS::CLogSocket* m_pLogSocket;		// 日志管理类
private:
	void ClearMsgQueue();					// 关闭时清除消息队列	
	void ProcDBMessage();					// 主线程处理消息
	void CheckConfig(unsigned int uiTick);  // 配置文件的检测
	void ProcInnerMessage(PInnerMsgNode pNode);
	void LoadFilterWord();
	void OnAddLabel(void* Sender);
	void UpdateLabels();
	void UpdateQueueCount(TInnerMsgType msgType, bool bAdd);
	void OnRemoveNetBarIP(void* p, int iKey);
	void LoadNetBarIPList();	
private:
	unsigned int m_uiSlowTick;
	std::mutex m_MsgCS;
	PInnerMsgNode m_pFirst;
	PInnerMsgNode m_pLast;
	int m_iNetBarIPFileAge;
	int m_iConfigFileAge;
	int m_iFilterFileAge;
	unsigned int m_uiLastCheckTick;
	CC_UTILS::CIntegerHash m_NoNeedActivateIPHash;
	std::vector<std::string> m_FilterWords;
	int m_iQueueCountList[8];
	bool m_bAllowGuest;
	bool m_bDenyRecharge;
	CC_UTILS::TMMTimer mmTimer;
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);
void UpdateLabel(const std::string& sDesc, int iTag);

#endif //__CC_DBSERVER_MAIN_THREAD_H__