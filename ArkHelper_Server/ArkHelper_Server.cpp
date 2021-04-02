#include"Class/app.h"

bool g_bExit;
HANDLE g_hEvent = INVALID_HANDLE_VALUE;

BOOL CALLBACK CosonleHandler(DWORD ev)
{
	BOOL bRet = FALSE;
	switch (ev)
	{
		// the user wants to exit. 
	
	case CTRL_CLOSE_EVENT:
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
		//MessageBox(NULL, L"CTRL+BREAK received!", L"CEvent", MB_OK);
		g_bExit = true;
		WaitForSingleObject(g_hEvent, INFINITY);
		bRet = TRUE;
		break;
	default:
		break;
	}
	return bRet;
}



int main()
{
	//system("mode con cp select=65001");//默认utf-8显示
	//system("cls");
	DEBUGLOG("Program Start");
	SetConsoleCtrlHandler(CosonleHandler, TRUE);
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//程序运行
    auto app = ArkHelperServerAPP::create();
	if (app) {

		COUT("\033[1,32,40mInit Succeed!\033[0m");
		app->run(&g_bExit);

		
		delete(app);

	}
	else {

		COUT("\033[1,31,40Init Failed！See Log File to the detials.\033[0m");
		Sleep(2000);

	}
	
	DEBUGLOG("Program Exit");
	SetEvent(g_hEvent);
	// 注意事件在这里清理
	CloseHandle(g_hEvent);
	SetConsoleCtrlHandler(CosonleHandler, FALSE);

	return 1;
}



