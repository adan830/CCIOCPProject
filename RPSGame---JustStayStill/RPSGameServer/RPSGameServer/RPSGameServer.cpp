// RPSGameServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_CLOSE_EVENT:
		if (pG_MainThread != nullptr)
			delete pG_MainThread;
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{	
	if (DoInitialWinSocket())
	{
		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
		{
			std::cout << "Unable to install handler!" << std::endl;
			return -1;
		}
		pG_MainThread = new CMainThread();		
		pG_MainThread->InitialWorkThread();
		while (true)
		{
			Sleep(1000);
		}
	}
	else
	{
		std::cout << "DoInitialWinSocket Fail!" << std::endl;
	}
	return 0;
}
