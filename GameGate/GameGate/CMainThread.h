/**************************************************************************************
@author: �²�
@content: ���̵߳�Ԫ
**************************************************************************************/
#ifndef __CC_GAME_GATE_MAIN_THREAD_H__
#define __CC_GAME_GATE_MAIN_THREAD_H__

#include "stdafx.h"

const int LABEL_CONNECT_ID = 1;
const int LABEL_RUN_ID = 2;
const int LABEL_PORT_ID = 3;

/**
*
* CExecutableBase������----���߳�(������̳У�DoExecute����Ϊ�麯��)
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
	CC_UTILS::CLogSocket* m_pLogSocket;  //��־������
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