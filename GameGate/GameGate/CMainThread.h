/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#ifndef __CC_GAME_GATE_MAIN_THREAD_H__
#define __CC_GAME_GATE_MAIN_THREAD_H__

#include "stdafx.h"

const int LABEL_CONNECT_ID = 1;
const int LABEL_RUN_ID = 2;
const int LABEL_PORT_ID = 3;

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
	void StartLogSocket(int idx);
public:
	CC_UTILS::CLogSocket* m_pLogSocket;  //日志管理类
private:
	void OnAddLabel(void* Sender);
private:
	CC_UTILS::TMMTimer mmTimer;
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);
void UpdateLabel(const std::string& sDesc, int iTag);
void TracertPackage(const std::string& sRoleName, char* pBuf, unsigned short usBufLen);

#endif //__CC_GAME_GATE_MAIN_THREAD_H__