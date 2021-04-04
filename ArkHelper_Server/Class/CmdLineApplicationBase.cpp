#include"CmdLineApplicationBase.h"

using namespace std;

CmdLineApplicationBase::CmdLineApplicationBase(std::string appname)
	: _appName(appname)
	, _exitFlag(&g_bExit)
	, _inputExit({ false,false })
	, _mainExit({ false,false })
{
#ifdef _WIN32
	SetConsoleCtrlHandler(CosonleHandler, TRUE);
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
	thread inputthread(&inputThread, this);
	inputthread.detach();

	do {

		this->mainWork();

		//更新退出信号
		this->updateMainExit();

	} while (!this->getMainExitState());

	this->setMainExitState();
}

void CmdLineApplicationBase::inputThread()
{
	string cmdStr;

	while (!this->getInputExitFlag())
	{

		std::cout << this->_appName;

		while (true) { //wait for the cmd hase been solved

			this->_cmdMutex.lock();
			cmdStr = this->_cmd;
			this->_cmdMutex.unlock();

			if (cmdStr == "")break;

			Sleep(5);

		}

		CIN(cmdStr);
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
	//cleat cmdstr
	this->_cmdMutex.lock();
	this->_cmd = "";
	this->_cmdMutex.unlock();

	auto now = time(NULL);
	for (auto& i : this->_workMap) {
		
		if (i.first <= now) {
			i.second();
		}

	}

	Sleep(this->_interval);
}


