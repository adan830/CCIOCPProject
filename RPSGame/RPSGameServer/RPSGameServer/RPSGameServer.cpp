// RPSGameServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{

	if (DoInitialWinSocket())
	{
		pG_MainThread = new CMainThread();
		pG_MainThread->InitialWorkThread();
		while (true)
		{
			Sleep(1000);
		}
		delete pG_MainThread;
	}
	else
	{
		std::cout << "DoInitialWinSocket Fail!" << std::endl;
	}
	return 0;
}
