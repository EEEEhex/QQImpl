#pragma once
/*****************************************************************//**
 * @file	QQOcr.h
 * @brief	此模块用于实现调用TencentOCR/WeChatOCR 并DLL导出
 * @version 2.0
 * 
 * @author	0xEEEE
 * @date	2023.10.16
 *********************************************************************/

#include <string>
#include <Windows.h>

//#include "ocr_protobuf.pb.h" 改为导出序列化数据

#ifdef QQOCR_EXPORTS
#define QQOCR_API __declspec(dllexport)
#else
#define QQOCR_API __declspec(dllimport)
#endif

namespace qqimpl
{
	namespace qqocr
	{
		/**
		 * @brief 初始化OCRManager.
		 * @param exe_path TencentOCR.exe路径
		 * @param dll_path mmmojo_64.dll的路径 默认为运行目录下
		 */
		extern "C" QQOCR_API void InitManager(char* exe_path = NULL, char* usr_lib_path = NULL);

		/**
		 * @brief 删除OCRManager.
		 */
		extern "C" QQOCR_API void UnInitManager();

		/**
		 * @brief 设置腾讯OCR Exe路径.
		 * @param exe_path char*类型路径
		 * @param 成功返回TRUE
		 */
		extern "C" QQOCR_API BOOL SetOcrExePath(const char* exe_path);

		/**
		 * @brief 设置腾讯OCR的--user-lib-dir参数的路径.
		 * @param usr_lib_path 也就是mmmojo_64.dll所在的路径
		 * @return 成功返回TRUE
		 */
		extern "C" QQOCR_API BOOL SetOcrUsrLibPath(const char* usr_lib_path);

		/**
		 * @brief 获取上一次的错误信息字符串.
		 * @return 错误信息
		 */
		extern "C" QQOCR_API const char* GetLastErrStr();

		/**
		 * @brief 封装 使用此函数发送OCR任务.
		 * @param pic_path char*类型
		 */
		extern "C" QQOCR_API BOOL DoOCRTask(const char* pic_path);

		/**
		 * @brief 初始化MMMojo环境.
		 * @return 成功返回true
		 */
		extern "C" QQOCR_API BOOL OCRdoInit();

		/**
		 * @brief 卸载OCR环境.
		 */
		extern "C" QQOCR_API void OCRdoUnInit();

		/**
		 * @brief 发送OCR Task [最好调用DoOCRTask].
		 * @param task_id 任务序号
		 * @param pic_path 要识别的图片路径
		 * @return 成功返回true
		 */
		extern "C" QQOCR_API BOOL SendOCRTask(long long task_id, const char* pic_path);

		typedef void(* LPFN_ONUSRREADPUSH)(const char* pic_path, void* ocr_response_serialize, int serialize_size);

		/**
		 * @brief 设置用户读取OCR结果的回调函数.
		 * @param callback 将OcrResponse作为参数
		 */
		extern "C" QQOCR_API void SetUsrReadPushCallback(LPFN_ONUSRREADPUSH callback);

		/**
		 * @brief 是否调用了Connect回调函数.
		 * @return 调用了返回TRUE
		 */
		extern "C" QQOCR_API BOOL HasConnectSignal();

		/**
		 * @brief 是否调用了Disconnect回调函数.
		 * @return 调用了返回TRUE
		 */
		extern "C" QQOCR_API BOOL HasDisconnectSignal();
	}
}