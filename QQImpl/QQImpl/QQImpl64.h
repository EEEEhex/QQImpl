#pragma once
//通过逆向分析实现一些QQ的IPC功能

#include <string>
#include <Windows.h>

#define NEED_WECHATOCR

#ifdef NEED_WECHATOCR
#include "ocr_protobuf.pb.h"
#endif

namespace qqimpl
{
	struct LAUNCHINFO
	{
		DWORD64 child_pid;		// [out]
		std::string exe_path_a; // [in]
		std::wstring exe_path_w;// [in]
		std::string magic_name;	// [in]
		bool is_alive;			// [out] 一开始要初始化为false

		//待定
	};//用于接收IPC MSG函数的第一个参数

	//QQ对Ipc一些操作的封装
	class QQIpcParentWrapper {
	public:
		QQIpcParentWrapper();
		~QQIpcParentWrapper();

		/**
		 * @brief 默认的子进程接收消息的函数.
		 * @param pArg LAUNCHINFO*
		 * @param msg IPC_MSG
		 * @param arg3 用不到 可能是版本号之类的
		 * @param addition_msg 如果addition_msg_size不为0就是有addition_msg(图片路径)
		 * @param addition_msg_size strlen(addition_msg)
		 */
		static void OnDefaultReceiveMsg(DWORD64* pArg, char* msg, DWORD64 arg3, char* addition_msg, int addition_msg_size);

		/**
		 * @brief 初始化环境.
		 * @param info 必须为全局变量
		 * @param dll_path parent-ipc-core-x86.dll的路径 默认为运行目录下
		 * @return 成功返回true
		 */
		bool InitEnv(std::wstring dll_path = L"parent-ipc-core-x64.dll");

		/**
		 * @brief 设置dll内部的日志等级.
		 * * @param level 0-4 5则关闭 [弃用]
		 */
		void SetLogLevel(int level);

		/**
		 * @brief 获取上一次的错误信息字符串.
		 * @return 错误信息 
		 */
		std::wstring GetLastErrStr();

		/**
		 * @biref 设置当前的启动信息.
		 * @param LAUNCHINFO* 如果需要在接收MSG回调函数中使用此信息则此变量必须为在运行期间不会被销毁的变量
		 * @return 成功返回true
		 */
		bool SetCurLaunchInfo(LAUNCHINFO* info);

		/**
		 * @brief 对pIMojoIpc->InitParentLog的封装.
		 * @param level 日志等级
		 * @param LogCallback为空
		 */
		void InitLog(int level = 0, void* callback = NULL);

		/**
		 * @brief 对pIMojoIpc->InitParentIpc()的封装.
		 */
		void InitParentIpc();

		/**
		 * @brief 对pIMojoIpc->LaunchChildProcess的封装.
		 * @param 用户自定义接收消息的函数 必须为5个参数
		 * @param cmdlines 要添加的命令行参数 (char*)[]
		 * @param cmd_num  要添加的命令行参数的个数
		 * @return 子进程PID 失败返回0
		 */
		UINT LaunchChildProcess(void* callback = NULL, char** cmdlines = NULL, int cmd_num = 0);

		/**
		 * @brief 对pIMojoIpc->ConnectedToChildProcess的封装.
		 * @return 成功返回true
		 */
		bool ConnectedToChildProcess();

		/**
		 * @brief 对pIMojoIpc->SendMessageUsingBufferInIPCThread的封装.
		 * @param command IPC_MSG
		 * @param addition_msg 参数
		 * @return 成功返回true
		 */
		bool SendIpcMessage(std::string command, std::string addition_msg = "");

		/**
		 * @brief 对pIMojoIpc->TerminateChildProcess的封装.
		 * @param exit_code 即chrome process_win.cc里的Process::Terminate方法
		 * @param wait The process may not end immediately due to pending I/O
		 * @return 成功返回true
		 */
		bool TerminateChildProcess(int exit_code, bool wait_);

		/**
		 * @brief 对pIMojoIpc->ReLaunchChildProcess的封装.
		 * @return 成功返回pid 失败返回-1
		 */
		DWORD64 ReLaunchChildProcess();


	private:
		LAUNCHINFO* m_info;
		HMODULE m_ipc_dll;
		DWORD64* m_ptr_IMojoIpc;
		void* m_usr_receive_callback;

		std::wstring _last_err;
	};

	class QQIpcChildWrapper
	{
	public:
		QQIpcChildWrapper();
		~QQIpcChildWrapper();

		/**
		 * @brief 获取上一次的错误信息字符串.
		 * @return 错误信息
		 */
		std::wstring GetLastErrStr();

		/**
		 * @brief 初始化环境.
		 * @param dll_path child(或parent, 因为这俩是一样的)-ipc.dll的路径 默认为运行目录下
		 * @return 成功返回true
		 */
		bool InitEnv(std::wstring dll_path = L"parent-ipc-core-x64.dll");

		/**
		 * @brief 对pIMojoIpc->InitChildIpc()的封装.
		 */
		void InitChildIpc();

		/**
		 * @brief 对pIMojoIpc->InitChildLog的封装.
		 * @param level 日志等级
		 * @param LogCallback为空
		 */
		void InitLog(int level = 0, void* callback = NULL);

		/**
		 * @brief 对pIMojoIpc->SetChildReceiveCallbackAndCOMPtr()的封装.
		 * @param callback 接收IPC消息的函数
		 */
		void SetChildReceiveCallback(void* callback = nullptr);

		/**
		 * @brief 对pIMojoIpc->ChildSendMessageUsingBuffer的封装.
		 * @param command IPC_MSG
		 * @param addition_msg 参数
		 */
		void ChildSendIpcMessage(std::string command, std::string addition_msg = "");

	private:
		HMODULE m_ipc_dll;
		DWORD64* m_ptr_IMojoIpc;// +00 虚表 +04 接收IPC MSG的函数地址 +12 日志等级 +16 &(COM指针)

		std::wstring _last_err;
	};

#ifdef NEED_WECHATOCR
	namespace qqocrwrapper
	{
		//QQOCRManager结构体是逆向分析出来的对应类
		struct OCRVTABLE
		{
			DWORD64* ThrowException;
			DWORD64* OnReadPush;
			DWORD64* Reserved_2;
			DWORD64* Reserved_3;
			DWORD64* OnRemoteConnect;
			DWORD64* OnRemoteDisconnect;
			DWORD64* OnRemoteProcessLaunched;
			DWORD64* OnRemoteProcessLaunchFailed;
			DWORD64* OnRemoteMojoError;
		};

		struct QQOCRManager
		{
			OCRVTABLE* vTable;		// + 00 虚表
			DWORD64 Reserved_1;		// + 04
			DWORD64 Reserved_2;		// + 08
			DWORD64 isSending;		// + 12	是否正在发送OCR Task (1 = 是) 
			DWORD64 isEnvInit;		// + 16	是否初始化了环境(包括路径和MMMojo) (0 = 未初始化 1 = 只有路径 2 = 全部初始化)
			DWORD64* MMMojoEnvPtr;	// + 20 mmmojo!CreateMMMOjoEnvironment()
			std::wstring ExePath;	// + 24 TencentOCR.exe路径
			DWORD64 isArch64;			// + 48 是否是64位架构 32位架构不调用TencentOCR
			DWORD64 Reserved_13;		// + 52
			DWORD64 Reserved_14;		// + 56
			DWORD64 PlaceHolder[11];	// + 60 占位符
			std::string PicPath;	// + 104 要识别的图片路径
		};//size = 128

		typedef DWORD64(__cdecl* LPFN_INITIALIZEMMMOJO)(DWORD64, DWORD64);
		typedef DWORD64(__cdecl* LPFN_SHUTDOWNMMMOJO)();
		typedef DWORD64* (__cdecl* LPFN_CREATEMMMOJOENVIRONMENT)();
		typedef DWORD64* (__cdecl* LPFN_SETMMMOJOENVIRONMENTCALLBACKS)(DWORD64*, int, DWORD64*);//arg: 第一个参数都是 创建环境时返回的类指针, 序号, 函数指针
		typedef DWORD64(__cdecl* LPFN_SETMMMOJOENVIRONMENTINITPARAMS)(DWORD64*, DWORD64, DWORD64);
		typedef void(__cdecl* LPFN_APPENDMMSUBPROCESSSWITCHNATIVE)(DWORD64*, const char*, wchar_t*);//arg: "user-lib-dir", 工作目录
		typedef DWORD64(__cdecl* LPFN_STARTMMMOJOENVIRONMENT)(DWORD64*);
		typedef DWORD64(__cdecl* LPFN_STOPMMMOJOENVIRONMENT)(DWORD64*);
		typedef DWORD64(__cdecl* LPFN_REMOVEMMMOJOENVIRONMENT)(DWORD64*);
		typedef DWORD64* (__cdecl* LPFN_GETMMMOJOREADINFOREQUEST)(DWORD64*, DWORD64*);
		typedef DWORD64* (__cdecl* LPFN_GETMMMOJOREADINFOATTACH)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_REMOVEMMMOJOREADINFO)(DWORD64*);
		typedef DWORD64(__cdecl* LPFN_GETMMMOJOREADINFOMETHOD)(DWORD64*);
		typedef DWORD64(__cdecl* LPFN_GETMMMOJOREADINFOSYNC)(DWORD64*);
		typedef DWORD64* (__cdecl* LPFN_CREATEMMMOJOWRITEINFO)(DWORD64, DWORD64, DWORD64);
		typedef DWORD64* (__cdecl* LPFN_GETMMMOJOWRITEINFOREQUEST)(DWORD64*, DWORD64);
		typedef DWORD64(__cdecl* LPFN_REMOVEMMMOJOWRITEINFO)(DWORD64*);
		typedef DWORD64* (__cdecl* LPFN_GETMMMOJOWRITEINFOATTACH)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_SETMMMOJOWRITEINFOMESSAGEPIPE)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_SETMMMOJOWRITEINFORESPONSESYNC)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_SENDMMMOJOWRITEINFO)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_SWAPMMMOJOWRITEINFOCALLBACK)(DWORD64*, DWORD64*);
		typedef DWORD64(__cdecl* LPFN_SWAPMMMOJOWRITEINFOMESSAGE)(DWORD64*, DWORD64*);

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

		/**
		 * @brief 初始化OCRManager.
		 * @param exe_path WeChatOCR.exe路径
		 * @param dll_path mmmojo_64.dll的路径 默认为运行目录下
		 */
		void InitManager(std::wstring exe_path = L"", std::wstring dll_path = L"mmmojo_64.dll");

		/**
		 * @brief 删除OCRManager.
		 */
		void UnInitManager();

		/**
		 * @brief 设置腾讯OCR Exe路径.
		 * @param exe_path wstring类型路径
		 * @param 成功返回true
		 */
		bool SetOcrExePath(std::wstring exe_path);

		/**
		 * @brief 设置腾讯OCR的--user-lib-dir参数的路径.
		 * @param usr_lib_path 也就是mmmojo_64.dll存在的路径
		 */
		bool SetOcrUsrLibPath(std::wstring usr_lib_path);

		/**
		 * @brief 获取上一次的错误信息字符串.
		 * @return 错误信息
		 */
		std::wstring GetLastErrWStr();

		/**
		 * @brief 封装.
		 * @param pic_path string类型
		 */
		bool DoOCRTask(std::string pic_path);

		/**
		 * @brief 初始化MMMojo环境.
		 * @return 成功返回true
		 */
		bool OCRdoInit();

		/**
		 * @brief 卸载OCR环境.
		 */
		void OCRdoUnInit();

		/**
		 * @brief 发送OCR Task.
		 * @param task_id 任务序号
		 * @param pic_path 要识别的图片路径
		 * @return 成功返回true
		 */
		bool SendOCRTask(long long task_id, std::string pic_path);

		typedef void(__stdcall* LPFN_ONUSRREADPUSH)(std::string pic_path, ocr_protobuf::OcrResponse ocr_response);

		/**
		 * @brief 设置用户读取OCR结果的回调函数.
		 * @param callback 将OcrResponse作为参数
		 */
		void SetUsrReadPushCallbck(LPFN_ONUSRREADPUSH callback);

		/**
		 * @brief 是否调用了Disconnect回调函数.
		 * @return 调用了返回true
		 */
		bool HasDisconnectSignal();
	}
#endif
};
