#ifdef USE_STDAFX
#include "stdafx.h"
#endif

#include "QQImpl64.h"
#include <format>
#include <filesystem>
#include <iostream>
#include <format>

#include "QQIpc64.h"

qqimpl::QQIpcParentWrapper::QQIpcParentWrapper()
{
	m_info = nullptr;
	m_ipc_dll = 0;
	m_ptr_IMojoIpc = NULL;
	_last_err = L"ERR_OK";
}

qqimpl::QQIpcParentWrapper::~QQIpcParentWrapper()
{
}

qqimpl::QQIpcChildWrapper::QQIpcChildWrapper()
{
	m_ipc_dll = 0;
	m_ptr_IMojoIpc = NULL;
	_last_err = L"ERR_OK";
}

qqimpl::QQIpcChildWrapper::~QQIpcChildWrapper()
{
}

void qqimpl::QQIpcParentWrapper::OnDefaultReceiveMsg(DWORD64* pArg, char* msg, DWORD64 arg3, char* addition_msg, int addition_msg_size)
{
	LAUNCHINFO* arg_info = (LAUNCHINFO*)(pArg);
	std::cout << "\nOnParentReceiveMsg: ";
	if (arg_info != 0)
		std::cout << "[ " << arg_info->magic_name << " ]: ";

	std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
	std::cout << "[ " << msg_str << " ]";
	if (addition_msg_size != 0)
		std::cout << ": [" << add_msg_str << "]";
	std::cout << std::endl;

	if (msg_str == "onScreenShotRequestOCR")
	{
#ifdef NEED_WECHATOCR
		add_msg_str.push_back('\0');//从QQScreenshot传过来的图片路径没有'\0'
		if (!qqimpl::qqocrwrapper::DoOCRTask(add_msg_str))
			std::wcout << std::format(L"[!] DoOCRTask Err: {}\n", qqimpl::qqocrwrapper::GetLastErrWStr());
#endif
	}
}

/*
HRESULT DllGetClassObject(
	[in]  REFCLSID rclsid,
	[in]  REFIID   riid,
	[out] LPVOID* ppv);
*/
typedef HRESULT(*LPFN_DLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID*);

bool qqimpl::QQIpcParentWrapper::InitEnv(std::wstring dll_path)
{
	m_ipc_dll = LoadLibraryW(dll_path.c_str());
	if (m_ipc_dll == NULL)
	{
		_last_err = std::format(L"ParentIpc LoadDll {} Err:[{}]", dll_path, GetLastError());
		return false;
	}

	LPFN_DLLGETCLASSOBJECT lpfnDllGetClassObject = (LPFN_DLLGETCLASSOBJECT)GetProcAddress(m_ipc_dll, "DllGetClassObject");
	if (!lpfnDllGetClassObject)
	{
		_last_err = std::format(L"Can't Get \'DllGetClassObject\' Addr:[{}]", GetLastError());
		return false;
	}

	IID clsid = { 0x14D5685E, 0x0574, 0x4F1F,{ 0xB6, 0xA4, 0x9B, 0xE9, 0xC3, 0xBE, 0x0C, 0x0D } };
	//IID m_iid = { 0x00000001, 0x0000, 0x00000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };//IID_IClassFactory
	IID riid = { 0x8504298A, 0xA491, 0x4B40,{ 0xAA, 0xC4, 0xB2, 0x60, 0x8D, 0xAC, 0x40, 0x45} };

	IClassFactory* pIClassFactory = NULL;
	if (lpfnDllGetClassObject(clsid, IID_IClassFactory, (LPVOID*)(&pIClassFactory)) != S_OK)
	{
		_last_err = std::format(L"lpfnDllGetClassObject Err:[{}]", GetLastError());
		return false;
	}

	DWORD64* pIMojoIpc = NULL;
	pIClassFactory->CreateInstance(NULL, riid, (void**)&pIMojoIpc);
	pIClassFactory->Release();
	if (pIMojoIpc == NULL)
	{
		_last_err = L"pIClassFactory->CreateInstance Err";
		return false;
	}

	this->m_ptr_IMojoIpc = pIMojoIpc;

	_last_err = L"ERR_OK";
	return true;
}

bool qqimpl::QQIpcChildWrapper::InitEnv(std::wstring dll_path)
{
	m_ipc_dll = LoadLibraryW(dll_path.c_str());
	if (m_ipc_dll == NULL)
	{
		_last_err = std::format(L"ChildIpc LoadDll {} Err:[{}]", dll_path, GetLastError());
		return false;
	}

	LPFN_DLLGETCLASSOBJECT lpfnDllGetClassObject = (LPFN_DLLGETCLASSOBJECT)GetProcAddress(m_ipc_dll, "DllGetClassObject");
	if (!lpfnDllGetClassObject)
	{
		_last_err = std::format(L"Can't Get \'DllGetClassObject\' Addr:[{}]", GetLastError());
		return false;
	}

	IID clsid = { 0x14D5685E, 0x0574, 0x4F1F,{ 0xB6, 0xA4, 0x9B, 0xE9, 0xC3, 0xBE, 0x0C, 0x0D } };
	//IID m_iid = { 0x00000001, 0x0000, 0x00000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };//IID_IClassFactory
	IID riid = { 0x2A106583, 0xA3C4, 0x43AF,{ 0x85, 0xA0, 0x57, 0x81, 0x74, 0x89, 0x36, 0x1A} };

	IClassFactory* pIClassFactory = NULL;
	if (lpfnDllGetClassObject(clsid, IID_IClassFactory, (LPVOID*)(&pIClassFactory)) != S_OK)
	{
		_last_err = std::format(L"lpfnDllGetClassObject Err:[{}]", GetLastError());
		return false;
	}

	DWORD64* pIMojoIpc = NULL;
	pIClassFactory->CreateInstance(NULL, riid, (void**)&pIMojoIpc);
	pIClassFactory->Release();
	if (pIMojoIpc == NULL)
	{
		_last_err = L"pIClassFactory->CreateInstance Err";
		return false;
	}

	this->m_ptr_IMojoIpc = pIMojoIpc;

	_last_err = L"ERR_OK";
	return true;
}

std::wstring qqimpl::QQIpcParentWrapper::GetLastErrStr()
{
	return _last_err;
}

std::wstring qqimpl::QQIpcChildWrapper::GetLastErrStr()
{
	return this->_last_err;
}

bool qqimpl::QQIpcParentWrapper::SetCurLaunchInfo(LAUNCHINFO* info)
{
	//if (info->exe_path_a.empty() || info->exe_path_w.empty()) return false;

	if (std::filesystem::exists(info->exe_path_a))
	{
		this->m_info = info;
		return true;
	}
	else
	{
		this->m_info = nullptr;
		return false;
	}
}

//把有直接操作内存的和内联汇编的代码片段的编译优化关掉
#pragma optimize( "", off )
void qqimpl::QQIpcParentWrapper::SetLogLevel(int level)//此方法被废弃
{
	if (m_ipc_dll == NULL) return;
	if (level < 0 || level > 5) level = 5;

	DWORD64* lpLogLevel_ = (DWORD64*)((DWORD64)m_ipc_dll + 0x18DAE8);//待定
	*lpLogLevel_ = level;
}

void qqimpl::QQIpcParentWrapper::InitLog(int level, void* callback)
{
	DWORD64 *pIMojoIpc = this->m_ptr_IMojoIpc, 
		func_addr = *((DWORD64*)(*pIMojoIpc) + 4), logLevel = level;
	
	InitLogAsm((DWORD64)pIMojoIpc, (DWORD64)callback, logLevel, func_addr);
	_last_err = L"ERR_OK";
}

void qqimpl::QQIpcParentWrapper::InitParentIpc()
{
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 3);//pIMojoIpc->InitParentIpc()
	
	InitParentIpcAsm((DWORD64)pIMojoIpc, func_addr);
	_last_err = L"ERR_OK";
}

UINT qqimpl::QQIpcParentWrapper::LaunchChildProcess(void* callback, char** cmdlines, int cmd_num)
{
	if (this->m_info == nullptr) return -1;

	//pIMojoIpc->LaunchChildProcess(wchar_t *path, (char*)[] cmdlines, DWORD64 cmdlines_num, LPVOID callback, LPVOID lpClass);//最后一个参数是callback的第一个参数 是一个类指针
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 5);
	DWORD64 lpCallback = (DWORD64)callback, child_pid = 0;
	const wchar_t* wzChildPath = m_info->exe_path_w.c_str();
	if (callback == NULL)
		lpCallback = (DWORD64)OnDefaultReceiveMsg;

	child_pid = LaunchChildProcessAsm((DWORD64)pIMojoIpc, (DWORD64)wzChildPath, cmd_num, (DWORD64)cmdlines, lpCallback, (DWORD64)(this->m_info), func_addr);
	if (child_pid <= 0)
	{
		_last_err = L"pIMojoIpc->LaunchChildProcess Internal Error";
		return 0;
	}
	this->m_info->child_pid = child_pid;

	_last_err = L"ERR_OK";
	return (UINT)child_pid;
}

bool qqimpl::QQIpcParentWrapper::ConnectedToChildProcess()
{
	if(this->m_info == nullptr) return false;

	if (this->m_info->child_pid <= 0)
	{
		_last_err = L"ChildPid is not valid!";
		return false;
	}

	//pIMojoIpc->ConnectedToChildProcess(DWORD64 child_pid);
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 6), child_pid = this->m_info->child_pid;
	
	ConnectedToChildProcessAsm((DWORD64)pIMojoIpc, child_pid, func_addr);

	_last_err = L"ERR_OK";
	return true;
}

bool qqimpl::QQIpcParentWrapper::SendIpcMessage(std::string command, std::string addition_msg)
{
	if (this->m_info == nullptr) return false;

	//pIMojoIpc->SendIpcMessage(DWORD64 child_pid, char* msg_ptr, DWORD64 arg4, char* addition_ptr, DWORD64 addition_len);
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 7), addition_len = 0, child_pid = this->m_info->child_pid;
	const char* command_c = command.c_str(), *addition_c = NULL;
	if (!addition_msg.empty())
	{
		addition_c = addition_msg.data();
		addition_len = addition_msg.size();
	}

	SendIpcMessageAsm((DWORD64)pIMojoIpc, child_pid, (DWORD64)command_c, 1, (DWORD64)addition_c, addition_len, func_addr);
	_last_err = L"ERR_OK";
	return true;
}

bool qqimpl::QQIpcParentWrapper::TerminateChildProcess(int exit_code, bool wait_)
{
	if (this->m_info == nullptr) return false;

	//pIMojoIpc->TerminateChildProcess(DWORD64 child_pid, int exit_code, bool wait);
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 8), child_pid = this->m_info->child_pid;
	DWORD64 arg_wait = (DWORD64)wait_;
	TerminateChildProcessAsm((DWORD64)pIMojoIpc, child_pid, exit_code, arg_wait, func_addr);
	_last_err = L"ERR_OK";
	return true;
}

DWORD64 qqimpl::QQIpcParentWrapper::ReLaunchChildProcess()
{
	if (this->m_info == nullptr) return false;

	//pIMojoIpc->ReLaunchChildProcess(DWORD64 child_pid);
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 9), child_pid = this->m_info->child_pid, call_result = 0;
	

	call_result = ReLaunchChildProcessAsm((DWORD64)pIMojoIpc, child_pid, func_addr);
	if (call_result == E_FAIL)
	{
		_last_err = L"pIMojoIpc->ReLaunchChildProcess Internal Error";
		return -1;
	}
	_last_err = L"ERR_OK";
	return call_result;
}

//以下为对ChildIpc操作的封装
void qqimpl::QQIpcChildWrapper::InitChildIpc()
{
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 3);//pIMojoIpc->InitChildIpc()
	
	InitChildIpcAsm((DWORD64)pIMojoIpc, func_addr);
	_last_err = L"ERR_OK";
}

void qqimpl::QQIpcChildWrapper::InitLog(int level, void * callback)
{
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 4),	//pIMojoIpc->InitChildLog() 
		logLevel = level;
	
	InitLogAsm((DWORD64)pIMojoIpc, logLevel, 0, func_addr);
	_last_err = L"ERR_OK";
}

void qqimpl::QQIpcChildWrapper::SetChildReceiveCallback(void* callback)
{
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 5);//pIMojoIpc->SetChildReceiveCallbackAndCOMPtr()
	DWORD64 ipc_ptr_addr = (DWORD64)(&(this->m_ptr_IMojoIpc));//这个参数要是一个全局变量, 这里用类成员来代替
	
	SetChildReceiveCallbackAndCOMPtrAsm((DWORD64)pIMojoIpc, (DWORD64)callback, ipc_ptr_addr, func_addr);
	_last_err = L"ERR_OK";
}

void qqimpl::QQIpcChildWrapper::ChildSendIpcMessage(std::string command, std::string addition_msg)
{
	//pIMojoIpc->ChildSendMessageUsingBuffer(char* command, DWORD64 0, DWORD64 0, char* addition_msg, int addition_msg_size);
	DWORD64* pIMojoIpc = this->m_ptr_IMojoIpc,
		func_addr = *((DWORD64*)(*pIMojoIpc) + 7), addition_len = 0;
	const char* command_c = command.c_str(), * addition_c = NULL;
	if (!addition_msg.empty())
	{
		addition_c = addition_msg.data();
		addition_len = addition_msg.size();
	}

	ChildSendIpcMessageAsm((DWORD64)pIMojoIpc, (DWORD64)command_c, 0, (DWORD64)addition_c, addition_len, func_addr);
	_last_err = L"ERR_OK";
}
#pragma optimize( "", on )

#ifdef NEED_WECHATOCR
//以下为qqocrwrapper的实现
namespace qqimpl::qqocrwrapper
{
	DWORD64* OnThrowException(DWORD64 arg1);
	int OnReadPush(DWORD64 arg_this, DWORD64 arg1, DWORD64* arg2);
	void OnReserved_(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2);
	int OnRemoteConnect(DWORD64 arg_this, DWORD64 arg1);
	int OnRemoteDisconnect(DWORD64 arg_this);
	int OnRemoteProcessLaunched(DWORD64 arg_this);
	void OnRemoteProcessLaunchFailed(DWORD64 arg_this, DWORD64 arg1);
	int OnRemoteMojoError(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2);

	std::wstring _last_err;
	DWORD64* g_callbacks[8] = { (DWORD64*)OnReadPushTransfer, (DWORD64*)OnReserved2Transfer, (DWORD64*)OnReserved3Transfer, (DWORD64*)OnRemoteConnectTransfer,
						(DWORD64*)OnRemoteDisconnectTransfer, (DWORD64*)OnRemoteProcessLaunchedTransfer, (DWORD64*)OnRemoteProcessLaunchFailedTransfer,
						(DWORD64*)OnRemoteMojoErrorTransfer };

	qqimpl::qqocrwrapper::QQOCRManager	g_ocr_manager;//因为不是去调用, 而是仿写, 所以这个结构体由自己保存
	qqimpl::qqocrwrapper::MMMojoDLL		g_mmmojo;

	#define MAX_TASK_ID 32
	BYTE g_task_id[MAX_TASK_ID] = { 0 };//最多同时发MAX_TASK_ID个task (1 - MAX_TASK_ID), 0代表空闲 1代表占用
	std::map<int, std::string> g_id_path;//TASK_ID和对应的路径, protobuf里没有图片路径的信息, 只能自己记录一下了 在SendOCRTask函数里记录
	std::mutex task_mutex;//用于互斥获取TASK ID

	LPFN_ONUSRREADPUSH g_usr_readpush = NULL;//void UsrReadPush(std::string pic_path, ocr_protobuf::OcrResponse ocr_response);
	std::wstring g_mmmojo_dll_path, g_usr_lib_path;

	bool is_disconnect_signal = false;//是否调用了disconnect回调函数
	bool is_remoteconnect_singal = false;//是否调用了remote connect回调函数
	std::mutex connect_mutex;//mutex和condition_variable用于只有在回调了RemoteConnect也就是Connect上后再发送OCR任务
	std::condition_variable connect_con_var;

	//获取空闲的Task ID
	int GetIdleTaskId()
	{
		task_mutex.lock();
		for (size_t i = 0; i < MAX_TASK_ID; i++)
		{
			if (g_task_id[i] == 0)
			{
				g_task_id[i] = 1;
				task_mutex.unlock();
				return (i + 1);
			}
		}
		task_mutex.unlock();
		return 0;//返回0说明没有空闲的
	}

	bool SetTaskIdIdle(int num)
	{
		if (num < 1 || num > MAX_TASK_ID) return false;
		g_task_id[num - 1] = 0;
		return true;
	}

	void InitManager(std::wstring exe_path, std::wstring dll_path)
	{
		g_ocr_manager.vTable = (OCRVTABLE*)calloc(sizeof(OCRVTABLE), sizeof(BYTE));

		g_ocr_manager.vTable->ThrowException = (DWORD64*)OnThrowException;
		g_ocr_manager.vTable->OnReadPush = (DWORD64*)OnReadPush;
		g_ocr_manager.vTable->Reserved_2 = (DWORD64*)OnReserved_;
		g_ocr_manager.vTable->Reserved_3 = (DWORD64*)OnReserved_;
		g_ocr_manager.vTable->OnRemoteConnect = (DWORD64*)OnRemoteConnect;
		g_ocr_manager.vTable->OnRemoteDisconnect = (DWORD64*)OnRemoteDisconnect;
		g_ocr_manager.vTable->OnRemoteMojoError = (DWORD64*)OnRemoteMojoError;
		g_ocr_manager.vTable->OnRemoteProcessLaunched = (DWORD64*)OnRemoteProcessLaunched;
		g_ocr_manager.vTable->OnRemoteProcessLaunchFailed = (DWORD64*)OnRemoteProcessLaunchFailed;

		g_ocr_manager.isEnvInit = 0;
		g_ocr_manager.MMMojoEnvPtr = NULL;

		//判断是否为64位系统
		SYSTEM_INFO si; GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			g_ocr_manager.isArch64 = true;
		else
			g_ocr_manager.isArch64 = false;
		
		std::cout << std::format("[*] isArch64?: {}\n", g_ocr_manager.isArch64);

		if (!exe_path.empty()) SetOcrExePath(exe_path);
		g_mmmojo_dll_path = dll_path;
		_last_err = L"ERR_OK";
	}

	void UnInitManager()
	{
		if (g_ocr_manager.vTable != NULL)
		{
			free(g_ocr_manager.vTable);
		}
		if (g_ocr_manager.MMMojoEnvPtr != NULL)
		{
			OCRdoUnInit();
		}
		is_disconnect_signal = false;
		is_remoteconnect_singal = false;
		g_usr_readpush = NULL;
		g_usr_lib_path = L"";
		g_id_path.clear();
		for (size_t i = 0; i < MAX_TASK_ID; i++) SetTaskIdIdle(i + 1);
		_last_err = L"ERR_OK";
	}

	bool SetOcrExePath(std::wstring exe_path)
	{
		if (!std::filesystem::exists(exe_path))
		{
			_last_err = L"TencentOCR.exe Path is not exist!";
			return false;
		}

		if (!std::filesystem::is_regular_file(exe_path))
		{
			_last_err = L"The Path is a file!";
			return false;
		}

		g_ocr_manager.ExePath = exe_path;
		g_ocr_manager.isEnvInit = 1;
		_last_err = L"ERR_OK";
		return true;
	}

	bool SetOcrUsrLibPath(std::wstring usr_lib_path)
	{
		if (!std::filesystem::exists(usr_lib_path))
		{
			_last_err = L"UsrLib Path is not exist!";
			return false;
		}

		if (!std::filesystem::is_directory(usr_lib_path))
		{
			_last_err = L"The Path is a Directory!";
			return false;
		}
		g_usr_lib_path = usr_lib_path;
		return true;
	}

	std::wstring GetLastErrWStr()
	{
		return _last_err;
	}

	bool DoOCRTask(std::string pic_path)
	{
		if (g_ocr_manager.isEnvInit < 1)
		{
			_last_err = L"OCR Env Doesn't Init!";
			return false;
		}

		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = L"TencentOCR can't run on x86 32bit!";
			return false;
		}

		g_ocr_manager.PicPath = pic_path;

		if (g_mmmojo.hMMMojo == NULL || g_ocr_manager.MMMojoEnvPtr == NULL)
		{
			if (!OCRdoInit())
				return false;

			//_last_err = L"ERR_OK";
			//return true;
		}


		int id = GetIdleTaskId();
		if (id == 0)
		{
			_last_err = L"No Idle TASK ID";
			return false;
		}
		if (!SendOCRTask(id, pic_path))
		{
			SetTaskIdIdle(id);
			_last_err = L"[!] SendOCRTask Err\n";
			return false;
		}

		_last_err = L"ERR_OK";
		return true;
	}

	bool OCRdoInit()
	{
		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = L"TencentOCR can't run on x86 32bit!";
			return false;
		}

		if (g_ocr_manager.ExePath.empty() || g_mmmojo_dll_path.empty())
		{
			_last_err = L"OCRdoInit Err: Exe or DLL Path is Empty!";
			return false;
		}


		if (g_mmmojo.hMMMojo == NULL)
		{
			g_mmmojo.hMMMojo = LoadLibraryW(g_mmmojo_dll_path.c_str());
			HMODULE hMMMojo = g_mmmojo.hMMMojo;
			if (hMMMojo == NULL)
			{
				_last_err = std::format(L"LoadMMMojoDll Err:[{}]\n", GetLastError());
				return false;
			}

			g_mmmojo.lpInitializeMMMojo = (LPFN_INITIALIZEMMMOJO)GetProcAddress(hMMMojo, "InitializeMMMojo");
			g_mmmojo.lpShutdownMMMojo = (LPFN_SHUTDOWNMMMOJO)GetProcAddress(hMMMojo, "ShutdownMMMojo");
			g_mmmojo.lpCreateMMMojoEnvironment = (LPFN_CREATEMMMOJOENVIRONMENT)GetProcAddress(hMMMojo, "CreateMMMojoEnvironment");
			g_mmmojo.lpSetMMMojoEnvironmentCallbacks = (LPFN_SETMMMOJOENVIRONMENTCALLBACKS)GetProcAddress(hMMMojo, "SetMMMojoEnvironmentCallbacks");
			g_mmmojo.lpSetMMMojoEnvironmentInitParams = (LPFN_SETMMMOJOENVIRONMENTINITPARAMS)GetProcAddress(hMMMojo, "SetMMMojoEnvironmentInitParams");
			g_mmmojo.lpAppendMMSubProcessSwitchNative = (LPFN_APPENDMMSUBPROCESSSWITCHNATIVE)GetProcAddress(hMMMojo, "AppendMMSubProcessSwitchNative");
			g_mmmojo.lpStartMMMojoEnvironment = (LPFN_STARTMMMOJOENVIRONMENT)GetProcAddress(hMMMojo, "StartMMMojoEnvironment");
			g_mmmojo.lpStopMMMojoEnvironment = (LPFN_STOPMMMOJOENVIRONMENT)GetProcAddress(hMMMojo, "StopMMMojoEnvironment");
			g_mmmojo.lpRemoveMMMojoEnvironment = (LPFN_REMOVEMMMOJOENVIRONMENT)GetProcAddress(hMMMojo, "RemoveMMMojoEnvironment");
			g_mmmojo.lpGetMMMojoReadInfoRequest = (LPFN_GETMMMOJOREADINFOREQUEST)GetProcAddress(hMMMojo, "GetMMMojoReadInfoRequest");
			g_mmmojo.lpGetMMMojoReadInfoAttach = (LPFN_GETMMMOJOREADINFOATTACH)GetProcAddress(hMMMojo, "GetMMMojoReadInfoAttach");
			g_mmmojo.lpRemoveMMMojoReadInfo = (LPFN_REMOVEMMMOJOREADINFO)GetProcAddress(hMMMojo, "RemoveMMMojoReadInfo");
			g_mmmojo.lpGetMMMojoReadInfoMethod = (LPFN_GETMMMOJOREADINFOMETHOD)GetProcAddress(hMMMojo, "GetMMMojoReadInfoMethod");
			g_mmmojo.lpGetMMMojoReadInfoSync = (LPFN_GETMMMOJOREADINFOSYNC)GetProcAddress(hMMMojo, "GetMMMojoReadInfoSync");
			g_mmmojo.lpCreateMMMojoWriteInfo = (LPFN_CREATEMMMOJOWRITEINFO)GetProcAddress(hMMMojo, "CreateMMMojoWriteInfo");
			g_mmmojo.lpGetMMMojoWriteInfoRequest = (LPFN_GETMMMOJOWRITEINFOREQUEST)GetProcAddress(hMMMojo, "GetMMMojoWriteInfoRequest");
			g_mmmojo.lpRemoveMMMojoWriteInfo = (LPFN_REMOVEMMMOJOWRITEINFO)GetProcAddress(hMMMojo, "RemoveMMMojoWriteInfo");
			g_mmmojo.lpGetMMMojoWriteInfoAttach = (LPFN_GETMMMOJOWRITEINFOATTACH)GetProcAddress(hMMMojo, "GetMMMojoWriteInfoAttach");
			g_mmmojo.lpSetMMMojoWriteInfoMessagePipe = (LPFN_SETMMMOJOWRITEINFOMESSAGEPIPE)GetProcAddress(hMMMojo, "SetMMMojoWriteInfoMessagePipe");
			g_mmmojo.lpSetMMMojoWriteInfoResponseSync = (LPFN_SETMMMOJOWRITEINFORESPONSESYNC)GetProcAddress(hMMMojo, "SetMMMojoWriteInfoResponseSync");
			g_mmmojo.lpSendMMMojoWriteInfo = (LPFN_SENDMMMOJOWRITEINFO)GetProcAddress(hMMMojo, "SendMMMojoWriteInfo");
			g_mmmojo.lpSwapMMMojoWriteInfoCallback = (LPFN_SWAPMMMOJOWRITEINFOCALLBACK)GetProcAddress(hMMMojo, "SwapMMMojoWriteInfoCallback");
			g_mmmojo.lpSwapMMMojoWriteInfoMessage = (LPFN_SWAPMMMOJOWRITEINFOMESSAGE)GetProcAddress(hMMMojo, "SwapMMMojoWriteInfoMessage");

			if (g_mmmojo.lpInitializeMMMojo == NULL)
			{
				_last_err = std::format(L"MMMojoDll GetProcAddress Err:[{}]\n", GetLastError());
				return false;
			}
		}

		if (g_ocr_manager.MMMojoEnvPtr == NULL)
		{
			g_mmmojo.lpInitializeMMMojo(0, 0);

			g_ocr_manager.MMMojoEnvPtr = g_mmmojo.lpCreateMMMojoEnvironment();
			if (g_ocr_manager.MMMojoEnvPtr == NULL)
			{
				_last_err = L"CreateMMMojoEnvironment Err\n";
				return false;
			}

			if (g_ocr_manager.vTable->OnReadPush == NULL)
			{
				_last_err = L"vTable->OnReadPush is invaild!\n";
				return false;
			}
			//设置OCR的回调函数
			g_mmmojo.lpSetMMMojoEnvironmentCallbacks(g_ocr_manager.MMMojoEnvPtr, 0, (DWORD64*)&g_ocr_manager);

			for (size_t i = 1; i <= 8; i++)
			{
				g_mmmojo.lpSetMMMojoEnvironmentCallbacks(g_ocr_manager.MMMojoEnvPtr, i, g_callbacks[i - 1]);
			}

			g_mmmojo.lpSetMMMojoEnvironmentInitParams(g_ocr_manager.MMMojoEnvPtr, 0, 1);
			g_mmmojo.lpSetMMMojoEnvironmentInitParams(g_ocr_manager.MMMojoEnvPtr, 2, (DWORD64)g_ocr_manager.ExePath.c_str());

			std::filesystem::path exe_path = g_ocr_manager.ExePath;
			std::filesystem::path usr_lib_path = g_usr_lib_path;
			if (g_usr_lib_path.empty()) usr_lib_path = exe_path.parent_path().parent_path().parent_path();
			//判断usr_lib_path存不存在mmmojo_64.dll不存在则报错
			if (!std::filesystem::exists(usr_lib_path / "mmmojo_64.dll"))
			{
				_last_err = L"Missing mmmojo_64.dll required to start OCR";
				return false;
			}

			g_mmmojo.lpAppendMMSubProcessSwitchNative(g_ocr_manager.MMMojoEnvPtr, "user-lib-dir", (wchar_t*)usr_lib_path.wstring().c_str());

			g_mmmojo.lpStartMMMojoEnvironment(g_ocr_manager.MMMojoEnvPtr);
		}

		g_ocr_manager.isEnvInit = 2;
		_last_err = L"ERR_OK";
		return true;
	}

	void OCRdoUnInit()
	{
		//QQ在启动TencentOCR.exe后会设置一个Timer, 过几分钟自动shutdown ocr环境
		if (g_mmmojo.hMMMojo != NULL && g_ocr_manager.MMMojoEnvPtr != NULL)
		{
			g_mmmojo.lpStopMMMojoEnvironment(g_ocr_manager.MMMojoEnvPtr);
			g_mmmojo.lpRemoveMMMojoEnvironment(g_ocr_manager.MMMojoEnvPtr);
			//g_mmmojo.lpShutdownMMMojo();//这个函数其实QQ没调用 也不能调
		}
		g_ocr_manager.isSending = 0;
		g_ocr_manager.MMMojoEnvPtr = NULL;

		if (g_ocr_manager.ExePath.empty())
			g_ocr_manager.isEnvInit = 0;
		else
			g_ocr_manager.isEnvInit = 1;

		is_remoteconnect_singal = false;
	}

	bool SendOCRTask(long long task_id, std::string pic_path)
	{
		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = L"WeChatOCR can't run on x86 32bit!";
			return false;
		}

		g_id_path[task_id] = pic_path;//记录ID和图片路径的关系

		ocr_protobuf::OcrRequest ocr_request;
		ocr_request.set_unknow(0);
		ocr_request.set_task_id(task_id);
		ocr_protobuf::OcrRequest::PicPaths* pp = new ocr_protobuf::OcrRequest::PicPaths();
		pp->add_pic_path(pic_path);
		ocr_request.set_allocated_pic_path(pp);

		std::string data_;
		ocr_request.SerializeToString(&data_);

		DWORD64* write_info = g_mmmojo.lpCreateMMMojoWriteInfo(1, 0, 1);
		if (write_info == NULL)
		{
			_last_err = L"CreateMMMojoWriteInfo Err";
			return false;
		}

		DWORD64* request = g_mmmojo.lpGetMMMojoWriteInfoRequest(write_info, data_.size());//返回protobuf指针 vector格式

		memmove(request, data_.c_str(), data_.size());

		//在Send之前, 必须等待OCR进程启动
		std::unique_lock<std::mutex> lock(connect_mutex);
		connect_con_var.wait(lock, []() { return is_remoteconnect_singal; });
		g_mmmojo.lpSendMMMojoWriteInfo(g_ocr_manager.MMMojoEnvPtr, write_info);

		_last_err = L"ERR_OK";
		return true;
	}

	void SetUsrReadPushCallbck(LPFN_ONUSRREADPUSH callback)
	{
		g_usr_readpush = callback;
	}

	bool HasDisconnectSignal()
	{
		return is_disconnect_signal;
	}

	//以下为自己实现的虚表函数
	DWORD64* OnThrowException(DWORD64 arg1)
	{
		return 0;
	}

	int OnReadPush(DWORD64 arg_this, DWORD64 arg1, DWORD64* arg2)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnReadPush: this:0x{:08X} ARG1:{}, ARG2:0x{:08X}\n", (DWORD64)pThis, arg1, (DWORD64)arg2);

		DWORD64 request_size = 0, attach_size = 0;
		DWORD64* read_request = g_mmmojo.lpGetMMMojoReadInfoRequest(arg2, &request_size);
		DWORD64* read_attach = g_mmmojo.lpGetMMMojoReadInfoAttach(arg2, &attach_size);

		ocr_protobuf::OcrResponse ocr_response;
		ocr_response.ParseFromArray(read_request, request_size);

		if (ocr_response.type() == 1)//WeChatOCR启动成功
		{
			if (g_usr_readpush != NULL) g_usr_readpush("", ocr_response);//调用用户函数
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}

		int32_t task_id_ = ocr_response.task_id();
		std::string pic_path;
		
		if (g_id_path.count(task_id_) == 0)
		{
			std::cout << "[!] OnReadPush: Get PicPath From ID_PATH_MAP Failed!\n";
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}
		pic_path = g_id_path[task_id_];
	
		std::cout << std::format("[*] OnReadPush type:{} task id:{} errCode:{}\n", ocr_response.type(), task_id_, ocr_response.err_code());
		std::cout << std::format("[*] OnReadPush TaskId:{} -> PicPath:{}\n", task_id_, pic_path);
		if (ocr_response.type() == 0)
		{
			std::cout << "[*] OcrResult:\n[\n";
			for (int i = 0; i < ocr_response.ocr_result().single_result_size(); i++)
			{
				ocr_protobuf::OcrResponse::OcrResult::SingleResult single_result = ocr_response.ocr_result().single_result(i);
				std::cout << std::format("\tRECT:[lx:{}, ly:{}, rx:{}, ry:{}]\n", single_result.lx(), single_result.ly(), single_result.rx(), single_result.ry());
				std::string utf8str = single_result.single_str_utf8();
				printf("\tUTF8STR:[");
				for (size_t j = 0; j < utf8str.size(); j++)
				{
					printf("%02X ", (BYTE)utf8str[j]);
				}
				puts("]");
			}
			puts("]");
		}

		if (g_usr_readpush != NULL) g_usr_readpush(pic_path, ocr_response);//调用用户函数

		g_mmmojo.lpRemoveMMMojoReadInfo(arg2);

		//删除id与pic_path的map
		SetTaskIdIdle(task_id_);
		g_id_path.erase(task_id_);
		return 0;
	}

	void OnReserved_(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2)
	{
		;
	}

	int OnRemoteConnect(DWORD64 arg_this, DWORD64 arg1)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnRemoteConnect: this: 0x{:08X}\n", (DWORD64)pThis);

		std::lock_guard<std::mutex> lock(connect_mutex);
		is_remoteconnect_singal = true;
		connect_con_var.notify_all();

		return 0;
	}

	int __stdcall OnRemoteDisconnect(DWORD64 arg_this)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnRemoteDisconnect: this: 0x{:08X}\n", (DWORD64)pThis);
		is_disconnect_signal = true;
		return 0;
	}

	int __stdcall OnRemoteProcessLaunched(DWORD64 arg_this)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnRemoteProcessLaunched: this: 0x{:08X}\n", (DWORD64)pThis);
		return 0;
	}

	void __stdcall OnRemoteProcessLaunchFailed(DWORD64 arg_this, DWORD64 arg1)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnRemoteProcessLaunchFailed: this: 0x{:08X}\n", (DWORD64)pThis);
	}

	int __stdcall OnRemoteMojoError(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		std::cout << std::format("[*] OnRemoteMojoError: this: 0x{:08X}\n", (DWORD64)pThis);
		return 0;
	}
}
#endif // NEED_WECHATOCR

