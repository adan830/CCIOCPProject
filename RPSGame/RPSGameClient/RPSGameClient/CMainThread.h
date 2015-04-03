/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#ifndef __CC_DISPATCH_GATE_MAIN_THREAD_H__
#define __CC_DISPATCH_GATE_MAIN_THREAD_H__

#include "CCTcpSocketCommon.h"

/**
*
* CExecutableBase的子类----主线程(无子类继承，DoExecute不作为虚函数)
*
*/
class CMainThread : public CExecutableBase
{
public:
	CMainThread();
	virtual ~CMainThread();
	void DoExecute();
private:
	unsigned int m_uiSlowRunTick;        //慢速执行tick
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);

#endif //__CC_DISPATCH_GATE_MAIN_THREAD_H__