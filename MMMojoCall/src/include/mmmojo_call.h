/*****************************************************************//**
 * @file   mmmojo_call.h
 * @brief  实现调用mmmojo(微信XPlugin组件)的方法 根据mmmojo源码再次重构
 * 
 * @author 0xEEEE
 * @date   2023.10.27 
 *********************************************************************/
#pragma once

#include <map>
#include <string>
#include <vector>

#include "mojo_call_export.h"
#include "mmmojo.h"
#include "mmmojo_environment_callbacks.h"

//因为一开始是从QQNT逆向出来的, 所以保留qqimpl的叫法
//根据https://github.com/PCWeChat/mmmojo源码再次重构
namespace qqimpl
{
	namespace mmmojocall
	{
		//每一个组件的每一个请求都有一个自己的RequestId 组件会根据请求ID进行对应的操作
		
		//WeChatOCR组件
		enum RequestIdOCR
		{
			OCRPush = 1
		};

		//WeChatUtility组件
		enum RequestIdUtility
		{
			UtilityHiPush = 10001,				//是Utility启动发送的
			UtilityInitPullReq = 10002,			//初始化请求
			UtilityInitPullResp = 10003,		//回复创建的都是Shared类型的info, 但是调用了SwapMMMojoWriteInfoCallback, 所以回调的还是Pull
			UtilityResampleImagePullReq = 10010,
			UtilityResampleImagePullResp = 10011,
			UtilityDecodeImagePullReq = 10020,
			UtilityDecodeImagePullResp = 10021,
			UtilityPicQRScanPullReq = 10030,	//10030是点击OCR时(也是打开图片时)发送的请求, 参数是图片路径
			UtilityQRScanPullReq = 10031,		//10031是截图框选时发送的请求, 参数应该是某种编码后的图片数据
			UtilityQRScanPullResp = 10032,		//这两种请求的返回ID都是10032
			UtilityTextScanPushResp = 10040		//TextScan具体在扫什么不是很清楚 可能是用来判断图片上是否有文字
		};

		//ThumbPlayer组件
		enum RequestIdPlayer
		{
			PlayerHiPush = 10001,								//ThumbPlayer启动时发送的
			PlayerInitPullReq = 10002,							//PlayerMgr::Init
			PlayerInitPullResp = 10003,							//PlayerMgr::Init
			PlayerInitPlayerCorePush = 10010,
			PlayerCreatePlayerCorePullReq = 10011,				//PlayerMgr::CreatePlayerCore
			PlayerCreatePlayerCorePullResp = 10012,				//PlayerMgr::CreatePlayerCore
			PlayerDestroyPlayerCorePush = 10013,				//PlayerMgr::DestroyCore
			PlayerPrepareAsyncPush = 10014,						//PlayerMgr::PrepareCore
			PlayerStartPush = 10015,							//PlayerMgr::StartCore
			PlayerStopPush = 10016,								//PlayerMgr::StopCore
			PlayerPausePush = 10017,							//PlayerMgr::PauseCore
			PlayerResumePush = 10018,							//PlayerMgr::ResumeCore
			PlayerSetAudioMutePush  = 10019,					//PlayerMgr::AudioMuteCore
			PlayerSeekToAsyncPush = 10020,						//PlayerMgr::SeekToCore
			PlayerGetCurrentPositionMsPullReq = 10021,			//PlayerMgr::GetCurrentPositionMsCore
			PlayerGetCurrentPositionMsPullResp = 10022,			//PlayerMgr::GetCurrentPositionMsCore
			PlayerSetVideoSurfacePush = 10023,					//PlayerMgr::VideoSurfaceCore
			PlayerSetAudioVolumePush = 10024,					//PlayerMgr::AudioVolumeCore
			PlayerSetDataSourcePush = 10025,					//PlayerMgr::ReadyDataSourceCore
			PlayerSetLoaderSourcePush = 10026,					//PlayerMgr::DownloadDataSourceCore
			PlayerSetRepeatPush = 10027,						//PlayerMgr::RepeatCore
			PlayerResizePush = 10028,							//PlayerMgr::ResizeCore
			PlayerSetPlaySpeedRatio = 10029,					//PlayerMgr::SpeedRatioCore
			PlayerInfoPush = 10030,
			PlayerErrorPlayerPush = 10031,
			PlayerVideoSizeChangedPush = 10032,
			PlayerUnknown0Push = 10033,
			PlayerStatePush = 10034,	
			PlayerUnknonw1Push = 10035,
			PlayerUnknown2Push = 10036,
			PlayerStartTaskProxyPush = 10050,
			PlayerStartRequestProxyPush = 10051,
			PlayerCloseRequestProxyPush = 10052,
			PlayerPollingDatProxyPullReq = 10054				 
		};

		//小程序组件
		enum RequestIdWMPF
		{
			//
		};

		//一个类就是一个WeChat XPlugin组件
		class MMMOJOCALL_API XPluginManager
		{
		public:
			XPluginManager();
			~XPluginManager();
			
			/**
			 * @brief 设置要启动的WeChat组件(XPlugin)路径.
			 * @param exe_path char*类型路径
			 * @param 成功返回true
			*/
			bool SetExePath(const char* exe_path);

			/**
			 * @brief 对应Chromium源码中的base::CommandLine->AppendSwitchNative方法 用于添加一个'开关(Switch)'.
			 * @param switch_string 例如L"user-lib-dir", 这样会在命令行参数添加一个"--user-lib-dir"
			 * @param value 参数'='后面的值
			 * @return 是否添加成功
			 */
			bool AppendSwitchNativeCmdLine(const char* switch_string, const char* value);

			/**
			 * @brief 设置启动命令行参数 (在InitializeMMMojo中 会调用base::CommandLine::Init(argc, argv)初始化命令行参数).
			 * @param argc 参数个数
			 * @param argv 参数数组 当使用CallFuncXPluginMgr需传入const char**
			 * @return 成功返回true
			 */
			bool SetCommandLine(int argc, std::vector<std::string>& argv);

			/**
			 * @brief 设置对应type的回调.
			 * @param type MMMojoEnvironmentCallbackType
			 * @param pfunc 函数指针 mmmojo::common::MMMojoReadOnPull*等类型
			 */
			void SetOneCallback(int type, void* pfunc);

			/**
			 * @brief 设置全部回调函数.
			 * @param callbacks 回调函数 mmmojo::common::MMMojoEnvironmentCallbacks类型 如果是CallFuncXPluginMgr需传入结构体指针	 
			 */
			void SetCallbacks(mmmojo::common::MMMojoEnvironmentCallbacks callbacks);

			/**
			 * @brief 设置回调函数的UsrData (一般为类的this指针).
			 * @param usr_data 即回调函数的最后一个参数
			 */
			void SetCallbackUsrData(void* usr_data);

			/**
			 * @brief 初始化MMMojo环境并启动XPlugin组件.
			 * @return 成功返回true
			 */
			bool InitMMMojoEnv();

			/**
			 * @brief 关闭MMMojo环境.
			 */
			void StopMMMojoEnv();

			/**
			 * @brief 发送序列化后的protobuf数据.
			 * @param pb_data 序列化后的数据
			 * @param data_size 数据大小
			 * @param method 默认为MMMojoInfoMethod::kMMPush
			 * @param sync 默认为false
			 * @param request_id 默认为MMMojoRequestId::OCRPush
			 */
			void SendPbSerializedData(void* pb_data, int data_size, int method = MMMojoInfoMethod::kMMPush, bool sync = false, uint32_t request_id = RequestIdOCR::OCRPush);

			/**
			 * @brief 获取上一次的错误信息.
			 * @return last err string
			 */
			const char* GetLastErrStr();
		
		protected:
			void SetLastErrStr(std::string err_str);

		private:
			void* m_cb_usrdata;									//回调函数的最后一个参数
			std::string m_last_err;								//错误信息
			std::wstring m_exe_path;							//要启动的组件路径
			std::map<std::string, std::string> m_switch_native;	//switch命令行参数
			std::vector<std::string> m_cmdline;					//启动参数
			void* m_mmmojo_env_ptr;								//MMMojoEnv指针
			bool m_init_mmmojo_env;								//是否启动了MMMojo环境
			mmmojo::common::MMMojoEnvironmentCallbacks m_callbacks;	//回调函数
		};

		/**
		 * @brief 初始化m_MMMojoDLL.
		 * @param mmmojo_dll_path mmmojo(_64).dll所在路径
		 * @return 是否初始化成功
		 */
		extern "C" MMMOJOCALL_API bool InitMMMojoDLLFuncs(const char* mmmojo_dll_path);

		/**
		 * @brief 根据request_info获取protobuf序列化数据.
		 * @param request_info [IN] 回调函数的参数request_info
		 * @param data_size [OUT] 数据大小
		 * @return void* pb数据
		 */
		extern "C" MMMOJOCALL_API const void* GetPbSerializedData(const void* request_info, uint32_t &data_size);

		/**
		 * @brief 获取ReadInfo (reqeust_info)的attach数据.
		 * @param request_info [IN] 回调函数的参数request_info
		 * @param data_size [OUT] 数据大小
		 * @return void* attach数据
		 */
		extern "C" MMMOJOCALL_API const void* GetReadInfoAttachData(const void* request_info, uint32_t& data_size);

		/**
		 * @brief 销毁request_info. 正确使用方法为 GetPbSerializedData(...); /Your Code;/ RemoveReadInfoRequest(...);
		 * @param request_info [IN] 回调函数的参数request_info
		 */
		extern "C" MMMOJOCALL_API void RemoveReadInfo(const void* request_info);
	}
}