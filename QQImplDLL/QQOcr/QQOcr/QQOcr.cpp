#include "QQOcr.h"

#include "ocr_protobuf.pb.h"

#ifdef _WIN64
#include "QQOcrAsm.h"
#endif // _WIN64

namespace qqimpl
{
namespace qqocr
{
	//以下为char与wchar的转换
	std::wstring Utf8ToUnicode(std::string utf8_str)
	{
		if (utf8_str.empty())
			return std::wstring();

		const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8_str.at(0), (int)utf8_str.size(), nullptr, 0);
		if (size_needed <= 0)
		{
			throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
		}

		std::wstring result(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &utf8_str.at(0), (int)utf8_str.size(), &result.at(0), size_needed);
		return result;
	}

	std::string UnicodeToUtf8(std::wstring utf16_str) 
	{
		if (utf16_str.empty())
			return std::string();

		const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &utf16_str.at(0), (int)utf16_str.size(), nullptr, 0, nullptr, nullptr);
		if (size_needed <= 0)
		{
			throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
		}

		std::string result(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &utf16_str.at(0), (int)utf16_str.size(), &result.at(0), size_needed, nullptr, nullptr);
		return result;
	}

	//QQOCRManager结构体是逆向分析出来的对应类
	struct OCRVTABLE
	{
		DWORD_PTR* ThrowException;
		DWORD_PTR* OnReadPush;
		DWORD_PTR* Reserved_2;
		DWORD_PTR* Reserved_3;
		DWORD_PTR* OnRemoteConnect;
		DWORD_PTR* OnRemoteDisconnect;
		DWORD_PTR* OnRemoteProcessLaunched;
		DWORD_PTR* OnRemoteProcessLaunchFailed;
		DWORD_PTR* OnRemoteMojoError;
	};

	//其实可以不使用此结构体, 只是为了对应逆向出来的原逻辑
	struct QQOCRManager
	{
		OCRVTABLE* vTable;			// + 00 虚表
		DWORD_PTR Reserved_1;		// + 04
		DWORD_PTR Reserved_2;		// + 08
		DWORD_PTR isSending;		// + 12	是否正在发送OCR Task (1 = 是) 
		DWORD_PTR isEnvInit;		// + 16	是否初始化了环境(包括路径和MMMojo) (0 = 未初始化 1 = 只有路径 2 = 全部初始化)
		DWORD_PTR* MMMojoEnvPtr;	// + 20 mmmojo!CreateMMMOjoEnvironment()
		std::wstring ExePath;		// + 24 TencentOCR.exe路径
		DWORD_PTR isArch64;			// + 48 是否是64位架构 32位架构不调用TencentOCR
		DWORD_PTR Reserved_13;		// + 52
		DWORD_PTR Reserved_14;		// + 56
		DWORD_PTR PlaceHolder[11];	// + 60 占位符
		std::string PicPath;		// + 104 要识别的图片路径
	};//size = 128 (32位下)

	typedef DWORD_PTR(__cdecl* LPFN_INITIALIZEMMMOJO)(DWORD_PTR, DWORD_PTR);
	typedef DWORD_PTR(__cdecl* LPFN_SHUTDOWNMMMOJO)();
	typedef DWORD_PTR* (__cdecl* LPFN_CREATEMMMOJOENVIRONMENT)();
	typedef DWORD_PTR* (__cdecl* LPFN_SETMMMOJOENVIRONMENTCALLBACKS)(DWORD_PTR*, int, DWORD_PTR*);//arg: 第一个参数都是 创建环境时返回的类指针, 序号, 函数指针
	typedef DWORD_PTR(__cdecl* LPFN_SETMMMOJOENVIRONMENTINITPARAMS)(DWORD_PTR*, DWORD_PTR, DWORD_PTR);
	typedef void(__cdecl* LPFN_APPENDMMSUBPROCESSSWITCHNATIVE)(DWORD_PTR*, const char*, wchar_t*);//arg: "user-lib-dir", 工作目录
	typedef DWORD_PTR(__cdecl* LPFN_STARTMMMOJOENVIRONMENT)(DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_STOPMMMOJOENVIRONMENT)(DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_REMOVEMMMOJOENVIRONMENT)(DWORD_PTR*);
	typedef DWORD_PTR* (__cdecl* LPFN_GETMMMOJOREADINFOREQUEST)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR* (__cdecl* LPFN_GETMMMOJOREADINFOATTACH)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_REMOVEMMMOJOREADINFO)(DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_GETMMMOJOREADINFOMETHOD)(DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_GETMMMOJOREADINFOSYNC)(DWORD_PTR*);
	typedef DWORD_PTR* (__cdecl* LPFN_CREATEMMMOJOWRITEINFO)(DWORD_PTR, DWORD_PTR, DWORD_PTR);
	typedef DWORD_PTR* (__cdecl* LPFN_GETMMMOJOWRITEINFOREQUEST)(DWORD_PTR*, DWORD_PTR);
	typedef DWORD_PTR(__cdecl* LPFN_REMOVEMMMOJOWRITEINFO)(DWORD_PTR*);
	typedef DWORD_PTR* (__cdecl* LPFN_GETMMMOJOWRITEINFOATTACH)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_SETMMMOJOWRITEINFOMESSAGEPIPE)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_SETMMMOJOWRITEINFORESPONSESYNC)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_SENDMMMOJOWRITEINFO)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_SWAPMMMOJOWRITEINFOCALLBACK)(DWORD_PTR*, DWORD_PTR*);
	typedef DWORD_PTR(__cdecl* LPFN_SWAPMMMOJOWRITEINFOMESSAGE)(DWORD_PTR*, DWORD_PTR*);

	struct MMMojoDLL
	{
		HMODULE	hMMMojo;
		LPFN_INITIALIZEMMMOJO				lpInitializeMMMojo;
		LPFN_SHUTDOWNMMMOJO					lpShutdownMMMojo;
		LPFN_CREATEMMMOJOENVIRONMENT		lpCreateMMMojoEnvironment;
		LPFN_SETMMMOJOENVIRONMENTCALLBACKS	lpSetMMMojoEnvironmentCallbacks;
		LPFN_SETMMMOJOENVIRONMENTINITPARAMS	lpSetMMMojoEnvironmentInitParams;
		LPFN_APPENDMMSUBPROCESSSWITCHNATIVE	lpAppendMMSubProcessSwitchNative;
		LPFN_STARTMMMOJOENVIRONMENT			lpStartMMMojoEnvironment;
		LPFN_STOPMMMOJOENVIRONMENT			lpStopMMMojoEnvironment;
		LPFN_REMOVEMMMOJOENVIRONMENT		lpRemoveMMMojoEnvironment;
		LPFN_GETMMMOJOREADINFOREQUEST		lpGetMMMojoReadInfoRequest;
		LPFN_GETMMMOJOREADINFOATTACH		lpGetMMMojoReadInfoAttach;
		LPFN_REMOVEMMMOJOREADINFO			lpRemoveMMMojoReadInfo;
		LPFN_GETMMMOJOREADINFOMETHOD		lpGetMMMojoReadInfoMethod;
		LPFN_GETMMMOJOREADINFOSYNC			lpGetMMMojoReadInfoSync;
		LPFN_CREATEMMMOJOWRITEINFO			lpCreateMMMojoWriteInfo;
		LPFN_GETMMMOJOWRITEINFOREQUEST		lpGetMMMojoWriteInfoRequest;
		LPFN_REMOVEMMMOJOWRITEINFO			lpRemoveMMMojoWriteInfo;
		LPFN_GETMMMOJOWRITEINFOATTACH		lpGetMMMojoWriteInfoAttach;
		LPFN_SETMMMOJOWRITEINFOMESSAGEPIPE	lpSetMMMojoWriteInfoMessagePipe;
		LPFN_SETMMMOJOWRITEINFORESPONSESYNC	lpSetMMMojoWriteInfoResponseSync;
		LPFN_SENDMMMOJOWRITEINFO			lpSendMMMojoWriteInfo;
		LPFN_SWAPMMMOJOWRITEINFOCALLBACK	lpSwapMMMojoWriteInfoCallback;
		LPFN_SWAPMMMOJOWRITEINFOMESSAGE		lpSwapMMMojoWriteInfoMessage;
	};


#ifdef _WIN64
	DWORD64* OnThrowException(DWORD64 arg1);
	int OnReadPush(DWORD64 arg_this, DWORD64 arg1, DWORD64* arg2);
	void OnReserved_(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2);
	int OnRemoteConnect(DWORD64 arg_this, DWORD64 arg1);
	int OnRemoteDisconnect(DWORD64 arg_this);
	int OnRemoteProcessLaunched(DWORD64 arg_this);
	void OnRemoteProcessLaunchFailed(DWORD64 arg_this, DWORD64 arg1);
	int OnRemoteMojoError(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2);
#else
	int OnReadPushTransfer(DWORD arg1, DWORD arg2, DWORD arg3);
	int OnReserved2Transfer(DWORD arg1, DWORD arg2, DWORD arg3);
	int OnReserved3Transfer(DWORD arg1, DWORD arg2, DWORD arg3);
	int OnRemoteConnectTransfer(DWORD arg1, DWORD arg2);
	int OnRemoteDisconnectTransfer(DWORD arg1);
	int OnRemoteProcessLaunchedTransfer(DWORD arg1);
	int OnRemoteProcessLaunchFailedTransfer(DWORD arg1, DWORD arg2);
	int OnRemoteMojoErrorTransfer(DWORD arg1, DWORD arg2, DWORD  arg3);
	DWORD* __stdcall OnThrowException(DWORD arg1);
	int __stdcall OnReadPush(DWORD arg1, DWORD* arg2);
	void __stdcall OnReserved_(DWORD arg1, DWORD arg2);
	int __stdcall OnRemoteConnect(DWORD arg1);
	int __stdcall OnRemoteDisconnect();
	int __stdcall OnRemoteProcessLaunched();
	void __stdcall OnRemoteProcessLaunchFailed(DWORD arg1);
	int __stdcall OnRemoteMojoError(DWORD arg1, DWORD arg2);
#endif 

	std::string _last_err;
	DWORD_PTR* g_callbacks[8] = { (DWORD_PTR*)OnReadPushTransfer, (DWORD_PTR*)OnReserved2Transfer, (DWORD_PTR*)OnReserved3Transfer, (DWORD_PTR*)OnRemoteConnectTransfer,
						(DWORD_PTR*)OnRemoteDisconnectTransfer, (DWORD_PTR*)OnRemoteProcessLaunchedTransfer, (DWORD_PTR*)OnRemoteProcessLaunchFailedTransfer,
						(DWORD_PTR*)OnRemoteMojoErrorTransfer };

	qqimpl::qqocr::QQOCRManager		g_ocr_manager;//因为不是去调用, 而是仿写, 所以这个结构体由自己保存
	MMMojoDLL						g_mmmojo;

	#define MAX_TASK_ID 32
	BYTE g_task_id[MAX_TASK_ID] = { 0 };//最多同时发MAX_TASK_ID个task (1 - MAX_TASK_ID), 0代表空闲 1代表占用
	std::map<int, std::string> g_id_path;//TASK_ID和对应的路径, protobuf里没有图片路径的信息, 只能自己记录一下了 在SendOCRTask函数里记录
	std::mutex task_mutex;//用于互斥获取TASK ID

	qqimpl::qqocr::LPFN_ONUSRREADPUSH g_usr_readpush = NULL;//void UsrReadPush(char* pic_path, ocr_protobuf::OcrResponse ocr_response);
	std::wstring g_usr_lib_path;

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

	void InitManager(char* exe_path, char* usr_lib_path)
	{
		g_ocr_manager.vTable = (qqimpl::qqocr::OCRVTABLE*)calloc(sizeof(qqimpl::qqocr::OCRVTABLE), sizeof(BYTE));

		g_ocr_manager.vTable->ThrowException = (DWORD_PTR*)OnThrowException;
		g_ocr_manager.vTable->OnReadPush = (DWORD_PTR*)OnReadPush;
		g_ocr_manager.vTable->Reserved_2 = (DWORD_PTR*)OnReserved_;
		g_ocr_manager.vTable->Reserved_3 = (DWORD_PTR*)OnReserved_;
		g_ocr_manager.vTable->OnRemoteConnect = (DWORD_PTR*)OnRemoteConnect;
		g_ocr_manager.vTable->OnRemoteDisconnect = (DWORD_PTR*)OnRemoteDisconnect;
		g_ocr_manager.vTable->OnRemoteMojoError = (DWORD_PTR*)OnRemoteMojoError;
		g_ocr_manager.vTable->OnRemoteProcessLaunched = (DWORD_PTR*)OnRemoteProcessLaunched;
		g_ocr_manager.vTable->OnRemoteProcessLaunchFailed = (DWORD_PTR*)OnRemoteProcessLaunchFailed;

		g_ocr_manager.isEnvInit = 0;
		g_ocr_manager.MMMojoEnvPtr = NULL;

		//判断是否为64位系统
		SYSTEM_INFO si; GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			g_ocr_manager.isArch64 = true;
		else
			g_ocr_manager.isArch64 = false;

		if ((exe_path != NULL) && (strlen(exe_path) != 0)) SetOcrExePath(exe_path);
		if (usr_lib_path != NULL && (strlen(usr_lib_path) != 0)) SetOcrUsrLibPath(usr_lib_path);
		_last_err = "ERR_OK";
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
		_last_err = "ERR_OK";
	}

	BOOL SetOcrExePath(const char* exe_path)
	{
		if ((exe_path == NULL) || (strlen(exe_path) == 0))
		{
			_last_err = "Arg ExePath is Invaild!";
			return FALSE;
		}

		g_ocr_manager.ExePath = Utf8ToUnicode(exe_path);
		g_ocr_manager.isEnvInit = 1;
		_last_err = "ERR_OK";
		return TRUE;
	}

	BOOL SetOcrUsrLibPath(const char* usr_lib_path)
	{
		if ((usr_lib_path == NULL) || (strlen(usr_lib_path) == 0))
		{
			_last_err = "Arg UsrLibPath is Invaild!";
			return FALSE;
		}

		g_usr_lib_path = Utf8ToUnicode(usr_lib_path);
		return TRUE;
	}

	const char* GetLastErrStr()
	{
		return _last_err.c_str();
	}

	BOOL DoOCRTask(const char* pic_path)
	{
		if (g_ocr_manager.isEnvInit < 1)
		{
			_last_err = "OCR Env Doesn't Init!";
			return FALSE;
		}

		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = "TencentOCR can't run on x86 32bit!";
			return FALSE;
		}

		g_ocr_manager.PicPath = pic_path;

		if (g_mmmojo.hMMMojo == NULL || g_ocr_manager.MMMojoEnvPtr == NULL)
		{
			if (!OCRdoInit())
				return FALSE;
		}

		int id = GetIdleTaskId();
		if (id == 0)
		{
			_last_err = "No Idle TASK ID";
			return FALSE;
		}
		if (!SendOCRTask(id, pic_path))
		{
			SetTaskIdIdle(id);
			_last_err = "SendOCRTask Err";
			return FALSE;
		}

		_last_err = "ERR_OK";
		return TRUE;
	}

	BOOL OCRdoInit()
	{
		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = "TencentOCR can't run on x86 32bit!";
			return FALSE;
		}

		if (g_ocr_manager.ExePath.empty() || g_usr_lib_path.empty())
		{
			_last_err = "OCRdoInit Err: Exe or DLL Path is Empty!";
			return FALSE;
		}

		if (g_mmmojo.hMMMojo == NULL)
		{
			std::wstring mmmojo_dll_path = g_usr_lib_path, dll_name;
#ifdef _WIN64
			dll_name = L"mmmojo_64.dll";
#else
			dll_name = L"mmmojo.dll";
#endif // _WIN64

			if ((mmmojo_dll_path.back() == L'\\') || (mmmojo_dll_path.back() == L'/'))
				mmmojo_dll_path += dll_name;
			else
				mmmojo_dll_path += (L"\\" + dll_name);

			g_mmmojo.hMMMojo = LoadLibraryW(mmmojo_dll_path.c_str());
			HMODULE hMMMojo = g_mmmojo.hMMMojo;
			if (hMMMojo == NULL)
			{
				_last_err = "LoadMMMojoDll Err";
				return FALSE;
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
				_last_err = "MMMojoDll GetProcAddress Err";
				return FALSE;
			}
		}

		if (g_ocr_manager.MMMojoEnvPtr == NULL)
		{
			g_mmmojo.lpInitializeMMMojo(0, 0);

			g_ocr_manager.MMMojoEnvPtr = g_mmmojo.lpCreateMMMojoEnvironment();
			if (g_ocr_manager.MMMojoEnvPtr == NULL)
			{
				_last_err = "CreateMMMojoEnvironment Err";
				return FALSE;
			}

			if (g_ocr_manager.vTable->OnReadPush == NULL)
			{
				_last_err = "vTable->OnReadPush is invaild!";
				return FALSE;
			}
			//设置OCR的回调函数
			g_mmmojo.lpSetMMMojoEnvironmentCallbacks(g_ocr_manager.MMMojoEnvPtr, 0, (DWORD_PTR*)&g_ocr_manager);

			for (size_t i = 1; i <= 8; i++)
			{
				g_mmmojo.lpSetMMMojoEnvironmentCallbacks(g_ocr_manager.MMMojoEnvPtr, i, g_callbacks[i - 1]);
			}

			g_mmmojo.lpSetMMMojoEnvironmentInitParams(g_ocr_manager.MMMojoEnvPtr, 0, 1);
			g_mmmojo.lpSetMMMojoEnvironmentInitParams(g_ocr_manager.MMMojoEnvPtr, 2, (DWORD_PTR)g_ocr_manager.ExePath.c_str());

			if (g_usr_lib_path.empty())
			{
				_last_err = "Usr Lib Path is Empty!";
				return FALSE;
			}

			g_mmmojo.lpAppendMMSubProcessSwitchNative(g_ocr_manager.MMMojoEnvPtr, "user-lib-dir", (wchar_t*)g_usr_lib_path.c_str());

			g_mmmojo.lpStartMMMojoEnvironment(g_ocr_manager.MMMojoEnvPtr);
		}

		g_ocr_manager.isEnvInit = 2;
		_last_err = "ERR_OK";
		return TRUE;
	}


	void OCRdoUnInit()
	{
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
		is_disconnect_signal = false;
	}

	BOOL SendOCRTask(long long task_id, const char* pic_path) 
	{
		if (g_ocr_manager.isArch64 == false)
		{
			_last_err = "WeChatOCR can't run on x86 32bit!";
			return FALSE;
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

		DWORD_PTR* write_info = g_mmmojo.lpCreateMMMojoWriteInfo(1, 0, 1);
		if (write_info == NULL)
		{
			_last_err = "CreateMMMojoWriteInfo Err";
			return FALSE;
		}

		DWORD_PTR* request = g_mmmojo.lpGetMMMojoWriteInfoRequest(write_info, data_.size());//返回protobuf指针 vector格式

		memmove(request, data_.c_str(), data_.size());

		//在Send之前, 必须等待OCR进程启动
		std::unique_lock<std::mutex> lock(connect_mutex);
		connect_con_var.wait(lock, []() { return is_remoteconnect_singal; });
		g_mmmojo.lpSendMMMojoWriteInfo(g_ocr_manager.MMMojoEnvPtr, write_info);

		_last_err = "ERR_OK";
		return TRUE;
	}

	void SetUsrReadPushCallback(qqimpl::qqocr::LPFN_ONUSRREADPUSH callback)
	{
		g_usr_readpush = callback;
	}

	BOOL HasConnectSignal()
	{
		return is_remoteconnect_singal;
	}

	BOOL HasDisconnectSignal()
	{
		return is_disconnect_signal;
	}

	//以下为自己实现的虚表函数
#ifdef _WIN64
	DWORD64* OnThrowException(DWORD64 arg1)
	{
		return 0;
	}

	int OnReadPush(DWORD64 arg_this, DWORD64 arg1, DWORD64* arg2)
	{
		QQOCRManager* pThis = (QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnReadPush: this:0x{:08X} ARG1:{}, ARG2:0x{:08X}\n", (DWORD64)pThis, arg1, (DWORD64)arg2);

		DWORD64 request_size = 0, attach_size = 0;
		DWORD64* read_request = g_mmmojo.lpGetMMMojoReadInfoRequest(arg2, &request_size);
		DWORD64* read_attach = g_mmmojo.lpGetMMMojoReadInfoAttach(arg2, &attach_size);

		if (request_size <= 6)//此时ocr_response.type == 1 即 WeChatOCR启动成功
		{
			if (g_usr_readpush != NULL) 
				g_usr_readpush("", read_request, request_size);//调用用户函数
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}

		ocr_protobuf::OcrResponse ocr_response;
		ocr_response.ParseFromArray(read_request, request_size);

		int32_t task_id_ = ocr_response.task_id();
		std::string pic_path;

		if (g_id_path.count(task_id_) == 0)
		{
			//[!] OnReadPush: Get PicPath From ID_PATH_MAP Failed!\;
			_last_err = "Get PicPath From ID_PATH_MAP Failed!";
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}
		pic_path = g_id_path[task_id_];

		if (ocr_response.type() == 0)
		{
			//解析
			if (g_usr_readpush != NULL) //调用用户函数
				g_usr_readpush(pic_path.c_str(), read_request, request_size);


			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);

			//删除id与pic_path的map
			SetTaskIdIdle(task_id_);
			g_id_path.erase(task_id_);
		}

		return 0;
	}

	void OnReserved_(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2)
	{
		;
	}

	int OnRemoteConnect(DWORD64 arg_this, DWORD64 arg1)
	{
		qqimpl::qqocr::QQOCRManager* pThis = (qqimpl::qqocr::QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnRemoteConnect: this: 0x{:08X}\n", (DWORD64)pThis);

		std::lock_guard<std::mutex> lock(connect_mutex);
		is_remoteconnect_singal = true;
		connect_con_var.notify_all();

		return 0;
	}

	int __stdcall OnRemoteDisconnect(DWORD64 arg_this)
	{
		qqimpl::qqocr::QQOCRManager* pThis = (qqimpl::qqocr::QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnRemoteDisconnect: this: 0x{:08X}\n", (DWORD64)pThis);
		is_disconnect_signal = true;
		return 0;
	}

	int __stdcall OnRemoteProcessLaunched(DWORD64 arg_this)
	{
		qqimpl::qqocr::QQOCRManager* pThis = (qqimpl::qqocr::QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnRemoteProcessLaunched: this: 0x{:08X}\n", (DWORD64)pThis);
		return 0;
	}

	void __stdcall OnRemoteProcessLaunchFailed(DWORD64 arg_this, DWORD64 arg1)
	{
		qqimpl::qqocr::QQOCRManager* pThis = (qqimpl::qqocr::QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnRemoteProcessLaunchFailed: this: 0x{:08X}\n", (DWORD64)pThis);
	}

	int __stdcall OnRemoteMojoError(DWORD64 arg_this, DWORD64 arg1, DWORD64 arg2)
	{
		qqimpl::qqocr::QQOCRManager* pThis = (qqimpl::qqocr::QQOCRManager*)arg_this;

		//std::cout << std::format("[*] OnRemoteMojoError: this: 0x{:08X}\n", (DWORD64)pThis);
		return 0;
	}
#else
	DWORD* __stdcall OnThrowException(DWORD arg1)
	{
		return 0;
	}

	//其实是__thiscall
	int __stdcall OnReadPush(DWORD arg1, DWORD* arg2)
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		DWORD_PTR request_size = 0, attach_size = 0;
		DWORD_PTR* read_request = g_mmmojo.lpGetMMMojoReadInfoRequest(arg2, &request_size);
		DWORD_PTR* read_attach = g_mmmojo.lpGetMMMojoReadInfoAttach(arg2, &attach_size);

		ocr_protobuf::OcrResponse ocr_response;
		ocr_response.ParseFromArray(read_request, request_size);

		if (ocr_response.type() == 1)//WeChatOCR启动成功
		{
			if (g_usr_readpush != NULL) g_usr_readpush("", read_request, request_size);//调用用户函数
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}

		int32_t task_id_ = ocr_response.task_id();
		std::string pic_path;

		if (g_id_path.count(task_id_) == 0)
		{
			//OnReadPush: Get PicPath From ID_PATH_MAP Failed!
			g_mmmojo.lpRemoveMMMojoReadInfo(arg2);
			return 0;
		}
		pic_path = g_id_path[task_id_];

		if (ocr_response.type() == 0)
		{
			//解析
		}

		if (g_usr_readpush != NULL) //调用用户函数
			g_usr_readpush(pic_path.c_str(), read_request, request_size);

		g_mmmojo.lpRemoveMMMojoReadInfo(arg2);

		//删除id与pic_path的map
		SetTaskIdIdle(task_id_);
		g_id_path.erase(task_id_);
		return 0;
	}

	void __stdcall OnReserved_(DWORD arg1, DWORD arg2)
	{
		;
	}

	//其实是__thiscall
	int __stdcall OnRemoteConnect(DWORD arg1)
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		//std::cout << std::format("[*] OnRemoteConnect: this: 0x{:08X}\n", (DWORD64)pThis);

		std::lock_guard<std::mutex> lock(connect_mutex);
		is_remoteconnect_singal = true;
		connect_con_var.notify_all();

		return 0;
	}

	int __stdcall OnRemoteDisconnect()
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		//std::cout << std::format("[*] OnRemoteDisconnect: this: 0x{:08X}\n", (DWORD)pThis);
		is_disconnect_signal = true;
		return 0;
	}

	int __stdcall OnRemoteProcessLaunched()
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		//std::cout << std::format("[*] OnRemoteProcessLaunched: this: 0x{:08X}\n", (DWORD)pThis);
		return 0;
	}

	void __stdcall OnRemoteProcessLaunchFailed(DWORD arg1)
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		//std::cout << std::format("[*] OnRemoteProcessLaunchFailed: this: 0x{:08X}\n", (DWORD)pThis);
	}

	int __stdcall OnRemoteMojoError(DWORD arg1, DWORD arg2)
	{
		qqimpl::qqocr::QQOCRManager* pThis = NULL;
		__asm mov pThis, ecx

		//std::cout << std::format("[*] OnRemoteMojoError: this: 0x{:08X}\n", (DWORD)pThis);
		return 0;
	}

	//以下为wrapper.node + 0x2919D30 - wrapper.node + 0x2919D70
	_declspec(naked) int OnReadPushTransfer(DWORD arg1, DWORD arg2, DWORD arg3)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x10]
			push[ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0x4]
			pop ebp
			ret
		}
	}

	_declspec(naked) int OnReserved2Transfer(DWORD arg1, DWORD arg2, DWORD arg3)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x10]
			push[ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0x8]
			pop ebp
			ret
		}
	}

	_declspec(naked) int OnReserved3Transfer(DWORD arg1, DWORD arg2, DWORD arg3)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x10]
			push[ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0xC]
			pop ebp
			ret
		}
	}

	_declspec(naked) int OnRemoteConnectTransfer(DWORD arg1, DWORD arg2)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0x10]
			pop ebp
			ret
		}
	}

	_declspec(naked) int OnRemoteDisconnectTransfer(DWORD arg1)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x8]
			mov eax, [ecx]
			pop ebp
			jmp dword ptr[eax + 0x14]
		}
	}

	_declspec(naked) int OnRemoteProcessLaunchedTransfer(DWORD arg1)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x8]
			mov eax, [ecx]
			pop ebp
			jmp dword ptr[eax + 0x18]
		}
	}

	_declspec(naked) int OnRemoteProcessLaunchFailedTransfer(DWORD arg1, DWORD arg2)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0x1C]
			pop ebp
			ret
		}
	}

	_declspec(naked) int OnRemoteMojoErrorTransfer(DWORD arg1, DWORD arg2, DWORD  arg3)
	{
		_asm
		{
			push ebp
			mov ebp, esp
			mov ecx, [ebp + 0x10]
			push[ebp + 0xC]
			push[ebp + 0x8]
			mov eax, [ecx]
			call dword ptr[eax + 0x20]
			pop ebp
			ret
		}
	}
#endif // _WIN64
}
}

