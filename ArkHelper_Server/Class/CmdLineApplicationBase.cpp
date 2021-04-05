#include"CmdLineApplicationBase.h"

using namespace std;

#ifdef _WIN32

bool g_bExit = false;

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

#endif // _WIN32



CmdLineApplicationBase::CmdLineApplicationBase(std::string appname, unsigned long interval)
	: _appName(appname)
	, _exitFlag(&g_bExit)
	, _inputExit({ false,false })
	, _mainExit({ false,false })
	, _interval(interval)
{
#ifdef _WIN32
	SetConsoleCtrlHandler(CosonleHandler, TRUE);
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	system("cls");
#endif // _WIN32


}

CmdLineApplicationBase::~CmdLineApplicationBase()
{
	//wait for input thread exit 
	while (!this->getInputExitState()) {
		Sleep(10);
	}

#ifdef _WIN32
	SetEvent(g_hEvent);
	// 注意事件在这里清理
	CloseHandle(g_hEvent);
	SetConsoleCtrlHandler(CosonleHandler, FALSE);
#endif // _WIN32
}

void CmdLineApplicationBase::run()
{
	thread inputthread(&CmdLineApplicationBase::inputThread, this);
	inputthread.detach();

	while (!this->getMainExitFlag()) {

		this->mainWork();

	}

	this->setMainExitState();
}

void CmdLineApplicationBase::addWork(time_t time, std::function<void()> func)
{
	this->_workMap.insert({ time,func });
}

void CmdLineApplicationBase::inputThread()
{
	string cmdStr;

	while (!this->getMainExitFlag())
	{

		

		while (true) { //wait for the cmd hase been solved

			this->_cmdMutex.lock();
			cmdStr = this->_cmd;
			this->_cmdMutex.unlock();

			if (cmdStr == "")break;

			Sleep(5);

		}

		if (this->getMainExitFlag())break;

		std::cout << this->_appName;

		cin >> cmdStr;
		this->_cmdMutex.lock();
		this->_cmd = cmdStr;
		this->_cmdMutex.unlock();

	}

	this->setInputExitState();
}

void CmdLineApplicationBase::mainWork()
{
	string cmd;
	this->_cmdMutex.lock();
	cmd = this->_cmd;
	this->_cmdMutex.unlock();
	this->solveInput(cmd);
	//更新退出信号
	this->updateMainExit();
	//cleat cmdstr
	this->clearCmd();

	for (auto i = this->_workMap.begin(); i != this->_workMap.end(); i++) {

		auto now = time(NULL);
		if (i->first <= now) {
			i->second();
			this->_workMap.erase(i++);
		}

	}

	Sleep(this->_interval);
}

void CmdLineApplicationBase::clearCmd()
{
	this->_cmdMutex.lock();
	this->_cmd.clear();
	this->_cmdMutex.unlock();
}


