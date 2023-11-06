#include "qq_ipc.h"

#include "utils.h"
#include <iostream>

#ifdef _WIN64
#include "qq_ipc_asm.h"
#endif

/*
HRESULT DllGetClassObject(
	[in]  REFCLSID rclsid,
	[in]  REFIID   riid,
	[out] LPVOID* ppv);
*/
typedef HRESULT(WINAPI* LPFN_DLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID*);

//以下为Parent IPC的实现
qqimpl::qqipc::QQIpcParentWrapper::QQIpcParentWrapper()
{
	m_ipc_dll = 0;
	m_ptr_IMojoIpc = NULL;
	_last_err = "ERR_OK";
}

qqimpl::qqipc::QQIpcParentWrapper::~QQIpcParentWrapper()
{
}

#ifdef _WIN64
void qqimpl::qqipc::QQIpcParentWrapper::OnDefaultReceiveMsg(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size)
{
	std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
#ifdef _DEBUG
	std::cout << "[ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
	std::cout << "[ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
	if (addition_msg_size != 0)
	{
		std::cout << "[ " << __FUNCTION__ << "] | The Addition Msg: ";
		for (size_t i = 0; i < addition_msg_size; i++)
			printf("0x%02X ", (byte)(addition_msg[i]));
		puts("");
	}
#endif
}
#else
void __stdcall qqimpl::qqipc::QQIpcParentWrapper::OnDefaultReceiveMsg(void* pArg, char* msg, int arg3, int arg4, char* addition_msg, int addition_msg_size)
{
	std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
#ifdef _DEBUG
	std::cout << "[ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
	std::cout << "[ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
	if (addition_msg_size != 0)
	{
		std::cout << "[ " << __FUNCTION__ << "] | The Addition Msg: ";
		for (size_t i = 0; i < addition_msg_size; i++)
			printf("0x%02X ", (byte)(addition_msg[i]));
		puts("");
	}
#endif
}

void* qqimpl::qqipc::QQIpcParentWrapper::GetCallbackArg()
{
	DWORD* pIMojoIpc = (DWORD*)(this->m_ptr_IMojoIpc);
	if (pIMojoIpc == NULL)
	{
		_last_err = "IMojoIpc is not init!";
		return nullptr;
	}

	return (void*)(*(DWORD*)(((DWORD*)(*(DWORD*)(*(pIMojoIpc + 29)))) + 3));
}
#endif // _WIN64


bool qqimpl::qqipc::QQIpcParentWrapper::InitEnv(const char* dll_path)
{
	if ((dll_path == NULL) || (strlen(dll_path) == 0))
	{
#ifdef _WIN64
		dll_path = "parent-ipc-core-x64.dll";
#else
		dll_path = "parent-ipc-core-x86.dll";
#endif
	}

	m_ipc_dll = LoadLibraryA(dll_path);
	if (m_ipc_dll == NULL)
	{

		_last_err = "ParentIpc LoadDll Err";
		return false;
	}

	LPFN_DLLGETCLASSOBJECT lpfnDllGetClassObject = (LPFN_DLLGETCLASSOBJECT)GetProcAddress(m_ipc_dll, "DllGetClassObject");
	if (!lpfnDllGetClassObject)
	{
		_last_err = "ParentIpc Can't Get \'DllGetClassObject\' Addr";
		return false;
	}

	IID clsid = { 0x14D5685E, 0x0574, 0x4F1F,{ 0xB6, 0xA4, 0x9B, 0xE9, 0xC3, 0xBE, 0x0C, 0x0D } };
	//IID m_iid = { 0x00000001, 0x0000, 0x00000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };//IID_IClassFactory
	IID riid = { 0x8504298A, 0xA491, 0x4B40,{ 0xAA, 0xC4, 0xB2, 0x60, 0x8D, 0xAC, 0x40, 0x45} };

	IClassFactory* pIClassFactory = NULL;
	if (lpfnDllGetClassObject(clsid, IID_IClassFactory, (LPVOID*)(&pIClassFactory)) != S_OK)
	{
		_last_err = "ParentIpc lpfnDllGetClassObject Err";
		return false;
	}

	DWORD_PTR* pIMojoIpc = NULL;
	pIClassFactory->CreateInstance(NULL, riid, (void**)&pIMojoIpc);
	pIClassFactory->Release();
	if (pIMojoIpc == NULL)
	{
		_last_err = "pIClassFactory->CreateInstance Err";
		return false;
	}

	this->m_ptr_IMojoIpc = pIMojoIpc;

	_last_err = "ERR_OK";
	return true;
}

const char* qqimpl::qqipc::QQIpcParentWrapper::GetLastErrStr()
{
	return _last_err.c_str();
}

//把有直接操作内存的和内联汇编的代码片段的编译优化关掉
#pragma optimize( "", off )
void qqimpl::qqipc::QQIpcParentWrapper::SetLogLevel(int level)//已被弃用
{
	if (m_ipc_dll == NULL) return;
	if (level < 0 || level > 5) level = 5;

	DWORD_PTR offset = 0;
#ifdef _WIN64
	offset = 0x18DAE8;
#else
	offset = 0x150034;
#endif // _WIN64

	DWORD_PTR lpLogLevel_ = (DWORD_PTR)(m_ipc_dll + offset);//待定
	*(DWORD_PTR*)lpLogLevel_ = level;
}

void qqimpl::qqipc::QQIpcParentWrapper::InitLog(int level, void* callback)
{
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc,
		dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 4), logLevel = level;

#ifdef _WIN64
	InitLogAsm((DWORD_PTR)pIMojoIpc, (DWORD_PTR)callback, logLevel, dwFunc);
#else
	_asm
	{
		push logLevel
		push callback
		push pIMojoIpc
		call dwFunc
	}
#endif 

	_last_err = "ERR_OK";
}

void qqimpl::qqipc::QQIpcParentWrapper::InitParentIpc()
{
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc,
		dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 3);//pIMojoIpc->InitParentIpc()

#ifdef _WIN64
	InitParentIpcAsm((DWORD_PTR)pIMojoIpc, dwFunc);
#else
	_asm
	{
		push pIMojoIpc
		call dwFunc
	}
#endif // _WIN64

	_last_err = "ERR_OK";
}

int qqimpl::qqipc::QQIpcParentWrapper::LaunchChildProcess(const char* file_path, callback_ipc callback, void* cb_arg, char** cmdlines, int cmd_num)
{
	//pIMojoIpc->LaunchChildProcess(wchar_t *path, DWORD 0,DWORD 0, LPVOID callback, LPVOID Launch函数的参数);//arg4: wrapper.node + 0x97D1B0
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc,
		dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 5);

	int child_pid = 0;
	void* lpArg = cb_arg, *lpCallback = callback;
	std::wstring file_path_w = utils::Utf8ToUnicode(file_path);
	const wchar_t* wzChildPath = file_path_w.c_str();
	if (callback == nullptr)
		lpCallback = (void*)OnDefaultReceiveMsg;

#ifdef _WIN64
	child_pid = LaunchChildProcessAsm((DWORD_PTR)pIMojoIpc, (DWORD_PTR)wzChildPath, cmd_num, (DWORD_PTR)cmdlines, (DWORD_PTR)lpCallback, (DWORD_PTR)cb_arg, dwFunc);
#else
	_asm
	{
		push lpArg
		push lpCallback
		push 0
		push 0
		push wzChildPath
		push pIMojoIpc
		call dwFunc
		mov child_pid, eax
	}//成功返回PID 失败返回0
#endif

	if (child_pid <= 0)
	{
		_last_err = "pIMojoIpc->LaunchChildProcess Internal Error";
		return 0;
	}

	_last_err = "ERR_OK";
	return child_pid;
}

bool qqimpl::qqipc::QQIpcParentWrapper::ConnectedToChildProcess(int pid)
{
	if (pid <= 0)
	{
		_last_err = "ChildPid is not valid!";
		return false;
	}

	//pIMojoIpc->ConnectedToChildProcess(DWORD64 child_pid);
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc,
		dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 6), dwPid = pid, call_result = 0;

#ifdef _WIN64
	call_result = ConnectedToChildProcessAsm((DWORD_PTR)pIMojoIpc, dwPid, dwFunc);
#else
	_asm
	{
		push dwPid
		push pIMojoIpc
		call dwFunc
		mov call_result, eax
	}
#endif

	if (call_result == E_FAIL)
	{
		_last_err = "pIMojoIpc->ConnectedToChildProcess Internal Error";
		return false;
	}
	_last_err = "ERR_OK";
	return true;
}

bool qqimpl::qqipc::QQIpcParentWrapper::SendIpcMessage(int pid, const char* command, const char* addition_msg, int addition_msg_size)
{
	//pIMojoIpc->SendMessageUsingBufferInIPCThread(DWORD child_pid, char* command, DWORD 0, DWORD 0, char* addition_msg, int addition_msg_size);
	//pIMojoIpc->SendIpcMessage(DWORD64 child_pid, char* msg_ptr, DWORD64 arg4, char* addition_ptr, DWORD64 addition_len);
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 7), 
			addition_len = 0, dwPid = pid, call_result = 0;

#ifdef _WIN64
	call_result = SendIpcMessageAsm((DWORD_PTR)pIMojoIpc, dwPid, (DWORD_PTR)command, 1, (DWORD_PTR)addition_msg, addition_msg_size, dwFunc);
#else
	_asm
	{
		push addition_msg_size
		push addition_msg
		push 0
		push 0
		push command
		push dwPid
		push pIMojoIpc
		call dwFunc
		mov call_result, eax
	}
#endif // _WIN64

	//失败会返回0x80004005 E_FAIL
	if (call_result == E_FAIL)
	{
		_last_err = "pIMojoIpc->SendMessageUsingBufferInIPCThread Internal Error";
		return false;
	}
	_last_err = "ERR_OK";
	return true;
}

bool qqimpl::qqipc::QQIpcParentWrapper::TerminateChildProcess(int pid, int exit_code, bool wait_)
{
	//pIMojoIpc->TerminateChildProcess(DWORD child_pid, int exit_code, bool wait);
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 8), 
			dwPid = pid, call_result = 0;
	DWORD_PTR arg_wait = (DWORD_PTR)wait_;

#ifdef _WIN64
	call_result = TerminateChildProcessAsm((DWORD_PTR)pIMojoIpc, dwPid, exit_code, arg_wait, dwFunc);
#else
	_asm
	{
		push arg_wait
		push exit_code
		push dwPid
		push pIMojoIpc
		call dwFunc
		mov call_result, eax
	}
#endif // _WIN64

	if (call_result == E_FAIL)
	{
		_last_err = "pIMojoIpc->TerminateChildProcess Internal Error";
		return false;
	}
	_last_err = "ERR_OK";
	return true;
}

int qqimpl::qqipc::QQIpcParentWrapper::ReLaunchChildProcess(int pid)
{
	//pIMojoIpc->ReLaunchChildProcess(DWORD child_pid);
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 9), 
			dwPid = pid, call_result = 0;

#ifdef _WIN64
	call_result = ReLaunchChildProcessAsm((DWORD_PTR)pIMojoIpc, dwPid, dwFunc);
#else
	_asm
	{
		push dwPid
		push pIMojoIpc
		call dwFunc
		mov call_result, eax
	}
#endif // _WIN64

	if (call_result == E_FAIL)
	{
		_last_err = "pIMojoIpc->ReLaunchChildProcess Internal Error";
		return 0;
	}
	_last_err = "ERR_OK";
	return call_result;
}
#pragma optimize( "", on )

//以下为Child IPC的实现
qqimpl::qqipc::QQIpcChildWrapper::QQIpcChildWrapper()
{
	m_ipc_dll = 0;
	m_ptr_IMojoIpc = NULL;
	_last_err = "ERR_OK";
}

qqimpl::qqipc::QQIpcChildWrapper::~QQIpcChildWrapper()
{
}

const char* qqimpl::qqipc::QQIpcChildWrapper::GetLastErrStr()
{
	return this->_last_err.c_str();
}

bool qqimpl::qqipc::QQIpcChildWrapper::InitEnv(const char* dll_path)
{	
	if ((dll_path == NULL) || (strlen(dll_path) == 0))
	{
#ifdef _WIN64
		dll_path = "parent-ipc-core-x64.dll";
#else
		dll_path = "parent-ipc-core-x86.dll";
#endif
	}

	m_ipc_dll = LoadLibraryA(dll_path);
	if (m_ipc_dll == NULL)
	{
		_last_err = "ChildIpc LoadDll Err";
		return false;
	}

	LPFN_DLLGETCLASSOBJECT lpfnDllGetClassObject = (LPFN_DLLGETCLASSOBJECT)GetProcAddress(m_ipc_dll, "DllGetClassObject");
	if (!lpfnDllGetClassObject)
	{
		_last_err = "Can't Get \'DllGetClassObject\' Addr";
		return false;
	}

	IID clsid = { 0x14D5685E, 0x0574, 0x4F1F,{ 0xB6, 0xA4, 0x9B, 0xE9, 0xC3, 0xBE, 0x0C, 0x0D } };
	//IID m_iid = { 0x00000001, 0x0000, 0x00000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };//IID_IClassFactory
	IID riid = { 0x2A106583, 0xA3C4, 0x43AF,{ 0x85, 0xA0, 0x57, 0x81, 0x74, 0x89, 0x36, 0x1A} };

	IClassFactory* pIClassFactory = NULL;
	if (lpfnDllGetClassObject(clsid, IID_IClassFactory, (LPVOID*)(&pIClassFactory)) != S_OK)
	{
		_last_err = "lpfnDllGetClassObject Err:[{}]";
		return false;
	}

	DWORD_PTR* pIMojoIpc = NULL;
	pIClassFactory->CreateInstance(NULL, riid, (void**)&pIMojoIpc);
	pIClassFactory->Release();
	if (pIMojoIpc == NULL)
	{
		_last_err = "pIClassFactory->CreateInstance Err";
		return false;
	}

	this->m_ptr_IMojoIpc = pIMojoIpc;

	_last_err = "ERR_OK";
	return true;
}

#pragma optimize( "", off )
void qqimpl::qqipc::QQIpcChildWrapper::InitChildIpc()
{
	//pIMojoIpc->InitChildIpc()
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 3);

#ifdef _WIN64
	InitChildIpcAsm((DWORD_PTR)pIMojoIpc, dwFunc);
#else
	_asm
	{
		push pIMojoIpc
		call dwFunc
	}
#endif // _WIN64

	_last_err = "ERR_OK";
}

void qqimpl::qqipc::QQIpcChildWrapper::InitLog(int level, void* callback)
{
	//pIMojoIpc->InitChildLog()
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 4);

#ifdef _WIN64
	InitLogAsm((DWORD_PTR)pIMojoIpc, level, (DWORD_PTR)callback, dwFunc);
#else
	_asm
	{
		push pIMojoIpc
		call dwFunc
	}
#endif // _WIN64

	_last_err = "ERR_OK";
}

void qqimpl::qqipc::QQIpcChildWrapper::SetChildReceiveCallback(callback_ipc callback)
{
	//pIMojoIpc->SetChildReceiveCallbackAndCOMPtr()
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 5);
	DWORD_PTR ipc_ptr_addr = (DWORD_PTR)(&(this->m_ptr_IMojoIpc));//这个参数要是一个全局变量, 这里用类成员来代替
	DWORD_PTR* lpCallback = (DWORD_PTR*)(callback);

#ifdef _WIN64
	SetChildReceiveCallbackAndCOMPtrAsm((DWORD_PTR)pIMojoIpc, (DWORD_PTR)lpCallback, ipc_ptr_addr, dwFunc);
#else
	_asm
	{
		push ipc_ptr_addr
		push lpCallback
		push pIMojoIpc
		call dwFunc
	}
#endif // _WIN64

	_last_err = "ERR_OK";
}

void qqimpl::qqipc::QQIpcChildWrapper::SendIpcMessage(const char* command, const char* addition_msg, int addition_msg_size)
{
	//pIMojoIpc->ChildSendMessageUsingBuffer(char* command, DWORD 0, DWORD 0, char* addition_msg, int addition_msg_size);
	DWORD_PTR* pIMojoIpc = this->m_ptr_IMojoIpc, dwFunc = *((DWORD_PTR*)(*pIMojoIpc) + 7);

#ifdef _WIN64
	ChildSendIpcMessageAsm((DWORD_PTR)pIMojoIpc, (DWORD64)command, 0, (DWORD_PTR)addition_msg, addition_msg_size, dwFunc);
#else
	_asm
	{
		push addition_msg_size
		push addition_msg
		push 0
		push 0
		push command
		push pIMojoIpc
		call dwFunc
	}
#endif // _WIN64

	_last_err = "ERR_OK";
}
#pragma optimize( "", on )