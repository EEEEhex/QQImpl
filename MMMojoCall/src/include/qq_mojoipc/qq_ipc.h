#pragma once
/*****************************************************************//**
 * @file	QQIpc.h
 * @brief	此模块用于实现QQ Mojo IPC
 * @version 2.0
 * 
 * @author	0xEEEE
 * @date	2023.9.13
 *********************************************************************/

//此版本分离了IPC和OCR的实现 去除了C++20标准的依赖 并将32位和64位实现合并
//不能保证所有版本的parent-ipc-core.dll可用, 接口参数可能发生变化

//通过逆向分析实现一些QQ的IPC功能 
#include <string>
//#include <functional>
#include <Windows.h>

#include "mojo_call_export.h"

namespace qqimpl
{
	namespace qqipc
	{
		//QQLAUNCHINFO结构体是逆向分析出来的一个类, 其是接收IPC消息回调函数的第一个参数, 
		//(当然也可以自己定义一个类型, 其就是被传给OnDefaultReceiveMsg函数的第一个参数),  
		//此类会被复制一份到pIMojoIpc, 也就是它是pIMojoIpc类的一个成员变量, 
		// {位置为 (*(DWORD*)(((DWORD*)(*(DWORD*)(*(pIMojoIpc + 29)))) + 3))} (32位下)
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
		//以上结构体不使用 只是逆向出来的原逻辑

#ifdef _WIN64
		typedef void (*callback_ipc)(void*, char*, int, char*, int);
#else
		//32位下必须为__stdcall
		typedef void (WINAPI* callback_ipc)(void*, char*, int, int, char*, int);
		//typedef std::function<void(void*, char*, int, int, char*, int)> callback_ipc32_func;
#endif


		//QQ对Ipc一些操作的封装
		class MMMOJOCALL_API QQIpcParentWrapper {
		public:
			QQIpcParentWrapper();
			~QQIpcParentWrapper();

#ifdef _WIN64
			/**
			 * @brief 默认的子进程接收消息的函数.
			 * @param pArg 自定义参数
			 * @param msg IPC_MSG
			 * @param arg3 用不到 可能是版本号之类的
			 * @param addition_msg 如果addition_msg_size不为0就是有addition_msg(图片路径)
			 * @param addition_msg_size strlen(addition_msg)
			 */
			static void OnDefaultReceiveMsg(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size);
#else
			/**
			 * @brief 默认的子进程接收消息的函数.
			 * @param pArg 自定义参数
			 * @param msg IPC_MSG
		 	 * @param arg3 用不到 可能是版本号之类的
		 	 * @param arg4 可能是int 
		 	 * @param addition_msg 如果addition_msg_size不为0就是有addition_msg(图片路径)
		 	 * @param addition_msg_size strlen(addition_msg)
		 	 */
			static void __stdcall OnDefaultReceiveMsg(void* pArg, char* msg, int arg3, int arg4, char* addition_msg, int addition_msg_size);

			/**
			 * @brief 获取传给接收IPC消息的回调函数的参数 [已被弃用].
			 * @return void*
			 */
			void* GetCallbackArg();
#endif 

			/**
			 * @brief 初始化环境.
			 * @param dll_path parent-ipc-core-x86/x64.dll的路径 默认为运行目录下
			 * @return 成功返回true
			 */
			bool InitEnvA(std::string dll_path = "");

			/**
			 * @brief 初始化环境.
			 * @param dll_path parent-ipc-core-x86/x64.dll的路径 默认为运行目录下
			 * @return 成功返回true
			 */
			bool InitEnv(std::wstring dll_path = L"");
				
			/**
			 * @brief 设置dll内部的日志等级 [已被弃用].
			 * @param level 0-4 5则关闭
			 */
			void SetLogLevel(int level);
		
			/**
			 * @brief 获取上一次的错误信息字符串.
			 * @return 错误信息 
			 */
			std::string GetLastErrStrA();

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
			 * @param file_path 子进程路径
			 * @param callback 用户自定义接收消息的函数
			 * @param cb_arg 传递给接收ipc消息回调函数的第一个参数
			 * @param cmdlines 要添加的命令行参数 例如const char* cmd_args[] = {"-t", "-m"};
			 * @param cmd_num  要添加的命令行参数的个数
			 * @return 子进程PID 失败返回0
			 */
			int LaunchChildProcessA(std::string file_path, callback_ipc callback = nullptr, void* cb_arg = nullptr, char** cmdlines = nullptr, int cmd_num = 0);
		
			/**
			 * @brief 对pIMojoIpc->LaunchChildProcess的封装.
			 * @param file_path 子进程路径
			 * @param callback 用户自定义接收消息的函数
			 * @param cb_arg 传递给接收ipc消息回调函数的第一个参数
			 * @param cmdlines 要添加的命令行参数 例如const char* cmd_args[] = {"-t", "-m"};
			 * @param cmd_num  要添加的命令行参数的个数
			 * @return 子进程PID 失败返回0
			 */
			int LaunchChildProcess(std::wstring file_path, callback_ipc callback = nullptr, void* cb_arg = nullptr, char** cmdlines = nullptr, int cmd_num = 0);

			/**
			 * @brief 对pIMojoIpc->ConnectedToChildProcess的封装.
			 * @param pid 子进程的pid
			 * @return 成功返回true
			 */
			bool ConnectedToChildProcess(int pid);
		
			/**
			 * @brief 对pIMojoIpc->SendMessageUsingBufferInIPCThread的封装.
			 * @param pid 子进程的pid
			 * @param command IPC_MSG
			 * @param addition_msg 参数
			 * @return 成功返回true
			 */
			bool SendIpcMessage(int pid, std::string command, std::string addition_msg = "");
		
			/**
			 * @brief 对pIMojoIpc->TerminateChildProcess的封装.
			 * @param pid 子进程的pid
			 * @param exit_code 即chrome process_win.cc里的Process::Terminate方法
			 * @param wait The process may not end immediately due to pending I/O
			 * @return 成功返回true
			 */
			bool TerminateChildProcess(int pid, int exit_code = 0, bool wait_ = true);
		
			/**
			 * @brief 对pIMojoIpc->ReLaunchChildProcess的封装.
			 * @param pid 子进程的pid
			 * @return 成功返回新的pid 失败返回0
			 */
			int ReLaunchChildProcess(int pid);
		
		private:
			HMODULE m_ipc_dll;
			DWORD_PTR* m_ptr_IMojoIpc;
		
			std::wstring _last_err;
		};
		
		class MMMOJOCALL_API QQIpcChildWrapper
		{
		public:
			QQIpcChildWrapper();
			~QQIpcChildWrapper();
		
			/**
			 * @brief 获取上一次的错误信息字符串.
			 * @return 错误信息
			 */
			std::string GetLastErrStrA();

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
			bool InitEnvA(std::string dll_path = "");

			/**
			 * @brief 初始化环境.
			 * @param dll_path child(或parent, 因为这俩是一样的)-ipc.dll的路径 默认为运行目录下
			 * @return 成功返回true
			 */
			bool InitEnv(std::wstring dll_path = L"");
		
			/**
			 * @brief 对pIMojoIpc->InitChildIpc()的封装.
			 */
			void InitChildIpc();
		
			/**
			 * @brief 对pIMojoIpc->InitChildLog的封装.
			 */
			void InitLog(int level = 0, void* callback = NULL);
		
			/**
			 * @brief 对pIMojoIpc->SetChildReceiveCallbackAndCOMPtr()的封装.
			 * @param callback 接收IPC消息的函数
			 */
			void SetChildReceiveCallback(callback_ipc callback);
		
			/**
			 * @brief 对pIMojoIpc->ChildSendMessageUsingBuffer的封装.
			 * @param command IPC_MSG
			 * @param addition_msg 参数
			 */
			void SendIpcMessage(std::string command, std::string addition_msg = "");
		
		private:
			HMODULE m_ipc_dll;
			DWORD_PTR* m_ptr_IMojoIpc;
		
			std::wstring _last_err;
		};



	}
}