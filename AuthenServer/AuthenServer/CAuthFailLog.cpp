/**************************************************************************************
@author: �²�
@content: ��֤������־��Ԫ
**************************************************************************************/
#include "stdafx.h"
#include "CAuthFailLog.h"

using namespace CC_UTILS;

CAuthFailFileLog* pG_AuthFailLog;

/************************Start Of CAuthFailFileLog**************************************************/
CAuthFailFileLog::CAuthFailFileLog()
{

}

CAuthFailFileLog::~CAuthFailFileLog()
{
	WaitThreadExecuteOver();

}

void CAuthFailFileLog::DoExecute()
{

}

void CAuthFailFileLog::WriteLog(const std::string& sLog)
{

}

void CAuthFailFileLog::WriteCache()
{

}

/************************End Of CAuthFailFileLog****************************************************/
