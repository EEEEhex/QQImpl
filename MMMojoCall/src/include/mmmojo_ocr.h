/*****************************************************************//**
 * @file   mmmojo_ocr.h
 * @brief  在调用XPlugin的基础上对调用WeChatOCR的方法进行封装
 * 
 * @author 0xEEEE
 * @date   2023.10.29
 *********************************************************************/
#pragma once

#include "mojo_call_export.h"
#include "mmmojo_call.h"

#include "ocr_protobuf.pb.h"

#include <mutex>
#include <Windows.h>

namespace qqimpl
{
namespace mmmojocall
{
	class MMMOJOCALL_API OCRManager : public XPluginManager
	{
	public:
		OCRManager();
		~OCRManager();

		/**
		 * @brief 设置--user-lib-dir命令行参数.
		 * @param usr_lib_dir 即微信mmmojo(_64).dll所在目录
		 * @return 成功返回true 
		 */
		bool SetUsrLibDir(const char* usr_lib_dir);

		/**
		 * @brief 开放给用户的ReadOnPush回调函数的接口.
		 * @param pic_path 图片路径
		 * @param serialized_data 序列化后的数据
		 * @param data_size 数据大小
		 */
		typedef void (*LPFN_OCRREADONPUSH)(const char* pic_path, const void* serialized_data, int data_size);

		/**
		 * @brief 设置接收结果的回调函数.
		 * @param pfunc 应为LPFN_OCRREADONPUSH类型
		 * @return 成功返回true 
		 */
		void SetReadOnPush(LPFN_OCRREADONPUSH pfunc);

		/**
		 * @brief 启动MMMojo环境以及WeChatOCR.exe程序.
		 * @return 成功返回true
		 */
		bool StartWeChatOCR();

		/**
		 * @brief 销毁MMMojo环境以及WeChatOCR.exe程序. 
		 */
		void KillWeChatOCR();

		/**
		 * @brief 发送一次OCR请求.
		 * @param pic_path 图片路径
		 * @return 成功返回true 
		 */
		bool DoOCRTask(const char* pic_path);

		/**
		 * @brief 设置连接状态 [只有为true才能调用DoOCRTask 不需要手动调用].
		 * @param true为连接上
		 */
		void SetConnectState(bool connect);

		/**
		 * @brief 调用用户设置的回调函数.
		 * @param requst_id RequestIdOCR
		 * @param serialized_data pb序列化后的数据
		 * @param data_size pb数据的大小
		 */
		void CallUsrCallback(int request_id, const void* serialized_data, int data_size);

	private:
		bool SendOCRTask(uint32_t task_id, std::string pic_path);

		int GetIdleTaskId();
		bool SetTaskIdIdle(int id);

	private:
		#define OCR_MAX_TASK_ID 32			//最大同时发送任务ID, 默认32, 一定要<=32
		BYTE m_task_id[OCR_MAX_TASK_ID];
		std::map<int, std::string> m_id_path;
		std::mutex m_task_mutex;			//用于互斥获取TASK ID
		std::mutex m_connect_mutex;			//mutex和condition_variable用于只有在回调了RemoteConnect也就是Connect上后再发送OCR任务
		std::condition_variable m_connect_con_var;
		std::string m_usr_lib_dir;
		bool m_wechatocr_running;			//WeChatOCR.exe是否正在运行
		bool m_is_arch64;					//是否是64位系统
		bool m_connect_state;
		LPFN_OCRREADONPUSH m_usr_callback;	//用户设置的获取OCR结果的回调函数
	};
}
}
