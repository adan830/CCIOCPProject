/**************************************************************************************
@author: �²�
@content: DBServer��Main��Ԫ������CCWindowsService.dll���������߳�
**************************************************************************************/

#include "stdafx.h"

using namespace CC_UTILS;

char DEFAULT_SERVICE_NAME[] = "DBServer";
char DEFAULT_DESCRIPTION[] = "GAME ��Ϸ���ݿ⻺�������";

bool DoAppStart(void* Sender)
{
	pG_MainThread = new CMainThread(DEFAULT_SERVICE_NAME);
	pG_MainThread->InitialWorkThread();
	return true;
}

bool DoAppStop(void* Sender)
{
	delete pG_MainThread;
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hWindowsServiceDll;
	TServiceManagerFunc ServiceManagerFunc;
#ifdef DEBUG
	hWindowsServiceDll = LoadLibrary("CCWindowsService_Debug.dll");
#else
	hWindowsServiceDll = LoadLibrary("CCWindowsService.dll");
#endif
	if (hWindowsServiceDll != nullptr)
	{
		if (DoInitialWinSocket())
		{
			ServiceManagerFunc = (TServiceManagerFunc)GetProcAddress(hWindowsServiceDll, "DoApplicationRun");
			if (ServiceManagerFunc != nullptr)
				ServiceManagerFunc(DEFAULT_SERVICE_NAME, DEFAULT_DESCRIPTION, DoAppStart, DoAppStop);
			else
				std::cout << "the calling is error!" << std::endl;
		}
		else
		{
			std::cout << "DoInitialWinSocket Fail!" << std::endl;
		}
		FreeLibrary(hWindowsServiceDll);
	}
	else
	{
		std::cout << "dll is missing!" << std::endl;
	}
	return 0;
}

