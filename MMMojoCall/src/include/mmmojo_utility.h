/*****************************************************************//**
 * @file   mmmojo_utility.h
 * @brief  在调用XPlugin的基础上对调用WeChatUtility的方法进行封装
 * 
 * @author 0xEEEE
 * @date   2023.10.29
 *********************************************************************/
#pragma once

#include "mojo_call_export.h"
#include "mmmojo_call.h"

#include "utility_protobuf.pb.h"

namespace qqimpl
{
namespace mmmojocall
{
	class MMMOJOCALL_API UtilityManager : public XPluginManager
	{
	public:
		UtilityManager();
		~UtilityManager();

		/**
		 * @brief 设置--user-lib-dir命令行参数.
		 * @param usr_lib_dir 即微信mmmojo(_64).dll所在目录
		 * @return 成功返回true
		 */
		bool SetUsrLibDir(const char* usr_lib_dir);

		/**
		 * @brief 开放给用户的ReadOnPull回调函数的接口.
		 * @param type 和requst_id的值一样
		 * @param serialized_data 序列化后的数据
		 * @param data_size 数据大小
		 */
		typedef void (*LPFN_UTILITYREADONPUSHLL)(int type, const void* data, int data_size);

		/**
		 * @brief 设置接收Pull结果的回调函数 (QRScan的结果是通过Pull返回).
		 * @param pfunc 应为LPFN_UTILITYREADONPUSHLL类型
		 * @return 成功返回true
		 */
		void SetReadOnPull(LPFN_UTILITYREADONPUSHLL pfunc);

		/**
		 * @brief 设置接收Pull结果的回调函数 (TextScan的结果是通过Push返回).
		 * @param pfunc 应为LPFN_UTILITYREADONPUSHLL类型
		 * @return 成功返回true
		 */
		void SetReadOnPush(LPFN_UTILITYREADONPUSHLL pfunc);

		/**
		 * @brief 启动MMMojo环境以及WeChatUtility.exe程序.
		 * @return 成功返回true
		 */
		bool StartWeChatUtility();

		/**
		 * @brief 销毁MMMojo环境以及WeChatUtility.exe程序.
		 */
		void KillWeChatUtility();

		/**
		 * @brief 发送一次二维码识别请求 (在截图中框选时发送的请求) [还未实现].
		 * @param chunk 某种编码格式 还未逆出
		 * @return 成功返回true
		 */
		bool DoQRScan(void* chunk);

		/**
		 * @brief 发送一次二维码识别请求 (在微信中打开图片时发送的请求).
		 * @param pic_path 图片路径
		 * @param text_scan_id TextScan的任务ID
		 * @return 成功返回true
		 */
		bool DoPicQRScan(const char* pic_path, int text_scan_id = 1);
		
		/**
		 * @brief 发送一次重采样请求 [还未实现].
		 * @param origin_encode_path 微信加密后的图片路径
		 * @param decode_pic_path 解码后的图片路径
		 * @param pic_x 图片长
		 * @param pic_y 图片宽
		 * @return 成功返回true
		 */
		bool DoResampleImage(std::string origin_encode_path, std::string decode_pic_path, int pic_x, int pic_y);

		/**
		 * @brief [还未逆出].
		 * @param pic_path
		 * @return 
		 */
		bool DoDecodeImage(std::string pic_path);

		/**
		 * @brief 设置连接状态 [只有为true才能调用DoPicQRScan 不需要手动调用].
		 * @param true为连接上
		 */
		void SetConnectState(bool connect);

		/*
		 * @brief 获取连接状态
		 * @return 是否连接上 
		 */
		bool GetConnectState();

		/**
		 * @brief 设置传给用户回调函数的数据类型(protobuf/json)
		 * @param use_json 如果为true则传递json字符串, 为false则传递序列化后的pb二进制数据
		 */
		void SetCallbackDataMode(bool use_json);

		/**
		 * @brief 调用用户设置的回调函数.
		 * @param request_id RequstIdUtility
		 * @param serialized_data pb序列化后的数据
		 * @param data_size pb数据的大小
		 */
		void CallUsrCallback(int request_id, const void* serialized_data, int data_size, std::string pull_or_push = "pull");

	private:
		std::mutex m_connect_mutex;				//mutex和condition_variable用于只有在回调了RemoteConnect也就是Connect上后再发送任务
		std::condition_variable m_connect_con_var;
		std::string m_usr_lib_dir;
		bool m_wechatutility_running;			//WeChatUtility.exe是否正在运行
		bool m_connect_state;
		bool m_cb_data_use_json;
		LPFN_UTILITYREADONPUSHLL m_usr_cb_pull;	//用户设置的获取Utility Pull结果的回调函数
		LPFN_UTILITYREADONPUSHLL m_usr_cb_push;
	};




}
}
