/*****************************************************************//**
 * @file   xplugin_test.cpp
 * @brief  此文件代码是使用PlayerManager类调用WeChatPlayer.exe的示例
 *********************************************************************/

#include <Windows.h>
#include <iostream>

#include "mmmojo_player.h"

using namespace qqimpl;

//简单Player播放窗口类名
const char PLAYER_CLASS_NAME[] = "Simple Player Window Class";

//简单Player播放窗口的消息处理事件
LRESULT CALLBACK SimplePlayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//注册窗口类 为TRUE则注册 FALSE则取消注册
void RegisterSimplePlayerWin32(BOOL reg)
{
	if (reg)
	{
		// Register the window class.
		WNDCLASS wc = { };

		wc.lpfnWndProc = SimplePlayerWndProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = PLAYER_CLASS_NAME;

		RegisterClassA(&wc);
	}
	else
	{
		HINSTANCE hInst = GetModuleHandle(NULL);
		UnregisterClassA(PLAYER_CLASS_NAME, hInst);
	}
}

//创建窗口
DWORD WINAPI CreateSimplePlayerWIN32(LPVOID lpParam)
{
	//解析参数
	HWND* lpHwnd = (HWND*)(((DWORD64*)lpParam)[0]);
	HANDLE* lpEvent = (LPHANDLE)(((DWORD64*)lpParam)[1]);

	// Create the window.
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		PLAYER_CLASS_NAME,              // Window class
		"Simple Player Window",			// Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		GetModuleHandle(NULL),  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		SetEvent(*lpEvent);//设置Event
		return 0;
	}

	*lpHwnd = hwnd;//传递参数
	SetEvent(*lpEvent);//设置Event

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	// 消息循环:
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


HWND CreateSimplePlayerWindow()
{
	HWND winHwnd = NULL;
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//前8个字节为LPHWND, 后8个字节为LPEVENT, 最后8个字节为this类指针
	DWORD64* lpParam = (DWORD64*)calloc(16, 1);
	lpParam[0] = (DWORD64)(&winHwnd);
	lpParam[1] = (DWORD64)(&hEvent);
	CloseHandle(CreateThread(NULL, 0, CreateSimplePlayerWIN32, lpParam, 0, NULL));
	WaitForSingleObject(hEvent, INFINITE);
	free(lpParam);
	CloseHandle(hEvent);
	return winHwnd;
}

int main()
{	
	SetConsoleOutputCP(CP_UTF8);
	
	std::string wechat_player_dir;//"C:\\Users\\{YourName}\\AppData\Roaming\\Tencent\\WeChat\\XPlugin\\Plugins\\ThumbPlayer\\4063\\extracted\\WeChatPlayer.exe
	std::string wechat_dir;//Such as "D:\\WeChat\\[3.9.7.29]"
	
	std::cout << "\033[34m[=] Enter WeChatPlayer.exe Path:\n[>]\033[0m ";
	std::getline(std::cin, wechat_player_dir);

    std::cout << "\033[34m[=] Enter mmmojo(_64).dll Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_dir);

    //	加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.	
	if (!mmmojocall::InitMMMojoDLLFuncs(wechat_dir.c_str()))
	{
		std::cout << "\033[31m[!] mmmojocall::InitMMMojoDLLFuncs ERR!\033[0m\n";
        return 1;
	}

	//	初始化MMMojo (包括ThreadPool等), 只需要调用一次.	
	mmmojocall::InitMMMojoGlobal(NULL, NULL);


	//注册窗口类
	RegisterSimplePlayerWin32(TRUE);

	mmmojocall::PlayerManager player_manager;
	mmmojocall::PlayerManager::CoreStatus* core_status;

	//设置启动所需参数
	player_manager.SetExePath(wechat_player_dir.c_str());
	player_manager.SetSwitchArgs(wechat_dir.c_str());

	//启动WeChatPlayer.exe
	player_manager.StartWeChatPlayer();
	std::cout << "\033[31m[+] StartWeChatPlayer OK!\033[0m\n";
	
	
	std::string video_path, file_name;
	std::cout << "\033[34m[=] Enter The Video Path:\n[>]\033[0m ";
	std::getline(std::cin, video_path);

	std::string::size_type iPos = video_path.find_last_of('\\') + 1;
	std::string filename_ = video_path.substr(iPos, video_path.length() - iPos);
	file_name = filename_.substr(0, filename_.rfind("."));

	std::cout << "[=] Press any key to Create Player Core...\n"; getchar();
	//创建一个播放器内核 (可以创建多个 一个内核就是一个视频)
	int player_id_1 = player_manager.CreatePlayerCore();
	std::cout << "[*] Player Core ID: " << player_id_1 << std::endl;
	
	//创建WIN32窗口
	HWND video_win_1 = CreateSimplePlayerWindow();
	std::cout << "[*] Create Player Window OK [HANDLE: " << video_win_1 << "]\n";

	player_manager.InitPlayerCore(player_id_1, video_win_1, file_name.c_str(), video_path.c_str());
	std::cout << "[*] Init Player [" << player_id_1 << "] Core OK!\n";
	
	std::cout << "[*] Player [" << player_id_1 << "] Core Status:\n{\n";
	core_status = player_manager.GetPlayerCoreStatus(player_id_1);
	std::cout << "\tSize: " << core_status->org_width << " | " << core_status->org_height << std::endl;
	std::cout << "\tDuration: " << core_status->duration << std::endl;
	std::cout << "}\n";

	std::cout << "[=] Press any key to Start Video...\n"; getchar();
	player_manager.StartVideo(player_id_1);

	std::cout << "[=] Press any key to Pause Video...\n"; getchar();
	player_manager.PauseVideo(player_id_1);

	std::cout << "[=] Press any key to Resume Video...\n"; getchar();
	player_manager.ResumeVideo(player_id_1);

	std::cout << "[=] Press any key to Get Video Current Pos(ms)...\n"; getchar();
	int pos_ms = player_manager.GetCurrentPosition(player_id_1);
	std::cout << "[*] Player [" << player_id_1 << "] Core Pos: [" << pos_ms << "]\n";

	std::cout << "[=] Press any key to Seek to pos 0(begin)...\n"; getchar();
	player_manager.SeekToVideo(player_id_1, 0.0);

	std::cout << "[=] Press any key to Mute Video...\n"; getchar();
	player_manager.MuteVideo(player_id_1, true);

	std::cout << "[=] Press any key to Cancel Mute Video...\n"; getchar();
	player_manager.MuteVideo(player_id_1, false);

	std::cout << "[=] Press any key to Set Video Speed Ratio x2...\n"; getchar();
	player_manager.SetSpeedVideo(player_id_1, 2.0);

	std::cout << "[=] Press any key to Stop Video...\n"; getchar();
	player_manager.StopVideo(player_id_1);

	std::cout << "[=] Press any key to Destroy Player Core...\n"; getchar();
	player_manager.DestroyPlayerCore(player_id_1);

	std::cout << "[=] Press any key to Kill WeChatPlayer.exe...\n"; getchar();
	player_manager.KillWeChatPlayer();

	//卸载窗口类
	RegisterSimplePlayerWin32(FALSE);

	mmmojocall::ShutdownMMMojoGlobal();
	return 0;
}
