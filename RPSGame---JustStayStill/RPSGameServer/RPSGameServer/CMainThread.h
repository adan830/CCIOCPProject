/**************************************************************************************
@author: 陈昌
@content: 主线程单元
**************************************************************************************/
#ifndef __CC_DISPATCH_GATE_MAIN_THREAD_H__
#define __CC_DISPATCH_GATE_MAIN_THREAD_H__

#include "CCTcpSocketCommon.h"
#include "CCFileLog.h"

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
public:
	CC_UTILS::CFileLogManager* m_pLogFile; //日志文件记录对象
private:
	void CheckConfig(const unsigned int uiTick);
private:
	unsigned int m_uiSlowRunTick;          //慢速执行tick
	unsigned int m_uiCheckConfigTick;      //config文件检测
	int m_iConfigFileAge;                  //记录config文件的版本号
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);

#endif //__CC_DISPATCH_GATE_MAIN_THREAD_H__