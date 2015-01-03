/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#ifndef __CC_AUTHEN_SERVER_MAIN_THREAD_H__
#define __CC_AUTHEN_SERVER_MAIN_THREAD_H__

#include "stdafx.h"

const int LABEL_POOL_COUNT_ID = 1;
const int LABEL_CHILD_COUNT_ID = 2;
const int LABEL_SECURE_COUNT_ID = 3;

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
public:
	CC_UTILS::CLogSocket* m_pLogSocket;  //日志管理类
private:
	void OnAddLabel(void* Sender);
private:
	unsigned long m_ulSlowRunTick;       //慢速执行tick
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);
void UpdateLabel(const std::string& sDesc, int iTag);

#endif //__CC_AUTHEN_SERVER_MAIN_THREAD_H__