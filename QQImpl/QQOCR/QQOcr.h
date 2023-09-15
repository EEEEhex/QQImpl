#pragma once
/*****************************************************************//**
 * @file	QQOcr.h
 * @brief	此模块用于实现调用TencentOCR/WeChatOCR
 * @version 2.0
 * 
 * @author	0xEEEE
 * @date	2023.9.13
 *********************************************************************/

#include <string>
#include <Windows.h>

#include "ocr_protobuf.pb.h"

namespace qqimpl
{
	namespace qqocr
	{
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

		/**
		 * @brief 初始化OCRManager.
		 * @param exe_path TencentOCR.exe路径
		 * @param dll_path mmmojo_64.dll的路径 默认为运行目录下
		 */
		void InitManagerA(std::string exe_path = "", std::string usr_lib_path = "mmmojo_64.dll");

		/**
		 * @brief 初始化OCRManager.
		 * @param exe_path TencentOCR.exe路径
		 * @param dll_path mmmojo,dll的路径 默认为运行目录下
		 */
		void InitManager(std::wstring exe_path = L"",  std::wstring usr_lib_path = L"mmmojo_64.dll");

		/**
		 * @brief 删除OCRManager.
		 */
		void UnInitManager();

		/**
		 * @brief 设置腾讯OCR Exe路径.
		 * @param exe_path string类型路径
		 * @param 成功返回true
		 */
		bool SetOcrExePathA(std::string exe_path);

		/**
		 * @brief 设置腾讯OCR Exe路径.
		 * @param exe_path string类型路径
		 * @param 成功返回true
		 */
		bool SetOcrExePath(std::wstring exe_path);

		/**
		 * @brief 设置腾讯OCR的--user-lib-dir参数的路径.
		 * @param usr_lib_path 也就是mmmojo_64.dll存在的路径
		 */
		bool SetOcrUsrLibPathA(std::string usr_lib_path);

		/**
		 * @brief 设置腾讯OCR的--user-lib-dir参数的路径.
		 * @param usr_lib_path 也就是mmmojo_64.dll所在的路径
		 */
		bool SetOcrUsrLibPath(std::wstring usr_lib_path);

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
		 * @brief 封装 使用此函数发送OCR任务.
		 * @param pic_path string类型
		 */
		bool DoOCRTaskA(std::string pic_path);

		/**
		 * @brief 封装 使用此函数发送OCR任务.
		 * @param pic_path wstring类型
		 */
		bool DoOCRTask(std::wstring pic_path);

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
		 * @brief 发送OCR Task [最好调用DoOCRTask].
		 * @param task_id 任务序号
		 * @param pic_path 要识别的图片路径
		 * @return 成功返回true
		 */
		bool SendOCRTask(long long task_id, std::string pic_path);

		typedef void(* LPFN_ONUSRREADPUSH)(std::string pic_path, ocr_protobuf::OcrResponse ocr_response);

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
}