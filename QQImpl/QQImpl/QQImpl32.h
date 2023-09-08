#pragma once
//通过逆向分析实现一些QQ的IPC功能 

#include <string>
#include <Windows.h>

#ifdef NEED_WECHATOCR
#include "ocr_protobuf.pb.h"
#endif

namespace qqimpl
{
	//QQLAUNCHINFO结构体是逆向分析出来的对应类, 实际调用中会复制一份到pIMojoIpc, 也就是它是pIMojoIpc类的一个成员变量
	// {实际位置为 (*(DWORD*)(((DWORD*)(*(DWORD*)(*(pIMojoIpc + 29)))) + 3))} (32位下)
	//(有的位置可能是错误的, 但是能正常使用)
	struct QQLAUNCHINFO
	{
		DWORD Reserved_0;		// + 00	应该是虚表
		DWORD Reserved_1;		// + 04
		DWORD Reserved_2;		// + 08
		DWORD Reserved_3; 		// + 12 添加命令行参数用的
		DWORD Reserved_4;		// + 16 命令行参数的截止地址
		DWORD* pIMojoIpc;		// + 20 COM指针
		DWORD launchResult;		// + 24 LaunchChildProcess()返回值
		DWORD PlaceHolder_0[5];	// + 28
		std::wstring ExePath;	// + 48 QQScreenShot.exe路径 <- 这里后面+56会被写入其他值 没懂但无所谓
		DWORD Reserved_18;		// + 72
		DWORD PlaceHolder_1[28];// + 76
		DWORD* ProcessClass;	// + 188 Chrome内部Process类
		DWORD Reserved_48;		// + 192
		DWORD ChildPid;			// + 196 子进程PID
		DWORD PlaceHolder_3[14];// + 200
	};//size = 256

	//QQ对Ipc一些操作的封装
	class QQIpcParentWrapper {
	public:
		QQIpcParentWrapper();
		~QQIpcParentWrapper();

		/**
		 * @brief 默认的子进程接收消息的函数.
		 * @param pArg pIMojoIpc
		 * @param msg IPC_MSG
		 * @param arg3 用不到
		 * @param arg4 可能是int 
		 * @param addition_msg 如果addition_msg_size不为0就是有addition_msg(图片路径)
		 * @param addition_msg_size strlen(addition_msg)
		 */
		static void __stdcall OnDefaultReceiveMsg(DWORD* pArg, char* msg, DWORD arg3, DWORD arg4, char* addition_msg, int addition_msg_size);

		/**
		 * @brief 初始化环境.
		 * @param exe_path 子进程EXE路径
		 * @param dll_path parent-ipc-core-x86.dll的路径 默认为运行目录下
		 * @return 成功返回true
		 */
		bool Init(std::wstring exe_path, std::wstring dll_path = L"parent-ipc-core-x86.dll");

		/**
		 * @brief 设置子进程Exe路径.
		 * @param exe_path wstring类型路径
		 * @param 成功返回true
		 */
		bool SetExePath(std::wstring exe_path);

		/**
		 * @brief 获取QQ截图信息结构体.
		 * @return struct QQLAUNCHINFO*
		 */
		struct QQLAUNCHINFO* GetInfoPtr();

		/**
		 * @brief 设置dll内部的日志等级.
		 * * @param level 0-4 5则关闭
		 */
		void SetLogLevel(int level);

		/**
		 * @brief 获取上一次的错误信息字符串.
		 * @return 错误信息 
		 */
		std::wstring GetLastErrStr();

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
		 * @param 用户自定义接收消息的函数 必须为6个参数且为__stdcall
		 * @return 子进程PID 失败返回0
		 */
		UINT LaunchChildProcess(void* callback = NULL);

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
		 * @return 成功返回true
		 */
		bool ReLaunchChildProcess();


	private:
		HMODULE m_ipc_dll;
		int m_child_pid;
		DWORD* m_ptr_IMojoIpc;
		std::wstring m_exe_path;
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
		bool Init(std::wstring dll_path = L"child-ipc-core-x86.dll");

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
		DWORD* m_ptr_IMojoIpc;// +00 虚表 +04 接收IPC MSG的函数地址 +12 日志等级 +16 &(COM指针)

		std::wstring _last_err;
	};

#ifdef NEED_WECHATOCR
	namespace qqocrwrapper
	{
		//QQOCRManager结构体是逆向分析出来的对应类
		struct OCRVTABLE
		{
			DWORD* ThrowException;
			DWORD* OnReadPush;
			DWORD* Reserved_2;
			DWORD* Reserved_3;
			DWORD* OnRemoteConnect;
			DWORD* OnRemoteDisconnect;
			DWORD* OnRemoteProcessLaunched;
			DWORD* OnRemoteProcessLaunchFailed;
			DWORD* OnRemoteMojoError;
		};

		struct QQOCRManager
		{
			OCRVTABLE* vTable;		// + 00 虚表
			DWORD Reserved_1;		// + 04
			DWORD Reserved_2;		// + 08
			DWORD isSending;		// + 12	是否正在发送OCR Task (1 = 是) 
			DWORD isEnvInit;		// + 16	是否初始化了环境(包括路径和MMMojo) (0 = 未初始化 1 = 只有路径 2 = 全部初始化)
			DWORD* MMMojoEnvPtr;	// + 20 mmmojo!CreateMMMOjoEnvironment()
			std::wstring ExePath;	// + 24 TencentOCR.exe路径
			DWORD isArch64;			// + 48 是否是64位架构 32位架构不调用TencentOCR
			DWORD Reserved_13;		// + 52
			DWORD Reserved_14;		// + 56
			DWORD PlaceHolder[11];	// + 60 占位符
			std::string PicPath;	// + 104 要识别的图片路径
		};//size = 128

		typedef DWORD(__cdecl* LPFN_INITIALIZEMMMOJO)(DWORD, DWORD);
		typedef DWORD(__cdecl* LPFN_SHUTDOWNMMMOJO)();
		typedef DWORD* (__cdecl* LPFN_CREATEMMMOJOENVIRONMENT)();
		typedef DWORD* (__cdecl* LPFN_SETMMMOJOENVIRONMENTCALLBACKS)(DWORD*, int, DWORD*);//arg: 第一个参数都是 创建环境时返回的类指针, 序号, 函数指针
		typedef DWORD(__cdecl* LPFN_SETMMMOJOENVIRONMENTINITPARAMS)(DWORD*, DWORD, DWORD);
		typedef void(__cdecl* LPFN_APPENDMMSUBPROCESSSWITCHNATIVE)(DWORD*, const char*, wchar_t*);//arg: "user-lib-dir", 工作目录
		typedef DWORD(__cdecl* LPFN_STARTMMMOJOENVIRONMENT)(DWORD*);
		typedef DWORD(__cdecl* LPFN_STOPMMMOJOENVIRONMENT)(DWORD*);
		typedef DWORD(__cdecl* LPFN_REMOVEMMMOJOENVIRONMENT)(DWORD*);
		typedef DWORD* (__cdecl* LPFN_GETMMMOJOREADINFOREQUEST)(DWORD*, DWORD*);
		typedef DWORD* (__cdecl* LPFN_GETMMMOJOREADINFOATTACH)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_REMOVEMMMOJOREADINFO)(DWORD*);
		typedef DWORD(__cdecl* LPFN_GETMMMOJOREADINFOMETHOD)(DWORD*);
		typedef DWORD(__cdecl* LPFN_GETMMMOJOREADINFOSYNC)(DWORD*);
		typedef DWORD* (__cdecl* LPFN_CREATEMMMOJOWRITEINFO)(DWORD, DWORD, DWORD);
		typedef DWORD* (__cdecl* LPFN_GETMMMOJOWRITEINFOREQUEST)(DWORD*, DWORD);
		typedef DWORD(__cdecl* LPFN_REMOVEMMMOJOWRITEINFO)(DWORD*);
		typedef DWORD* (__cdecl* LPFN_GETMMMOJOWRITEINFOATTACH)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_SETMMMOJOWRITEINFOMESSAGEPIPE)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_SETMMMOJOWRITEINFORESPONSESYNC)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_SENDMMMOJOWRITEINFO)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_SWAPMMMOJOWRITEINFOCALLBACK)(DWORD*, DWORD*);
		typedef DWORD(__cdecl* LPFN_SWAPMMMOJOWRITEINFOMESSAGE)(DWORD*, DWORD*);

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
		 * @param exe_path TencentOCR.exe路径
		 * @param dll_path mmmojo,dll的路径 默认为运行目录下
		 * @param need_timer 是否设置定时器结束OCR进程
		 */
		void InitManager(std::wstring exe_path = L"", std::wstring dll_path = L"mmmojo.dll", bool need_timer = true);

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

		typedef void(__stdcall* LPFN_ONUSRREADPUSH)(ocr_protobuf::OcrResponse ocr_response);

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
