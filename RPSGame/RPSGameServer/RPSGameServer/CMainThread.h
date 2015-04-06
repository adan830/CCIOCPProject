/**************************************************************************************
@author: �²�
@content: ���̵߳�Ԫ
**************************************************************************************/
#ifndef __CC_DISPATCH_GATE_MAIN_THREAD_H__
#define __CC_DISPATCH_GATE_MAIN_THREAD_H__

#include "CCTcpSocketCommon.h"
#include "CCFileLog.h"

/**
*
* CExecutableBase������----���߳�(������̳У�DoExecute����Ϊ�麯��)
*
*/
class CMainThread : public CExecutableBase
{
public:
	CMainThread();
	virtual ~CMainThread();
	void DoExecute();
public:
	CC_UTILS::CFileLogManager* m_pLogFile; //��־�ļ���¼����
private:
	void CheckConfig(const unsigned int uiTick);
private:
	unsigned int m_uiSlowRunTick;          //����ִ��tick
	unsigned int m_uiCheckConfigTick;      //config�ļ����
	int m_iConfigFileAge;                  //��¼config�ļ��İ汾��
};

extern CMainThread* pG_MainThread;

void Log(const std::string& sInfo, byte loglv = 0);

#endif //__CC_DISPATCH_GATE_MAIN_THREAD_H__