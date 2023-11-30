/*****************************************************************//**
 * @file   mmmojo_player.h
 * @brief  在调用XPlugin的基础上对调用WeChatPlayer的方法进行封装
 * 
 * @author 0xEEEE
 * @date   2023.11.17
 *********************************************************************/
#pragma once

#include "mojo_call_export.h"
#include "mmmojo_call.h"

#include "player_protobuf.pb.h"

#include <mutex>
#include <Windows.h>

namespace qqimpl
{
namespace mmmojocall
{
	//WeChatPlayer会将视频窗口依附到(也有可能是注入)到初始化时设定的窗口句柄上

	//WeChatPlayer通信过程:
	//WeChatPlayer启动时会向WeChat发送PlayerHiPush消息, WeChat接收到PlayerHiPush消息后则向PlayerInitPullReq, 之后WeChatPlayer发送PlayerInitPullResp, 至此启动初始化流程完毕
	//需要播放视频时:
	//WeChat向Player发送PlayerCreatePlayerCorePullReq, 请求创建一个PlayerCore, Player回复PlayerCreatePlayerCorePullResp, 指明是否创建成功
	//之后WeChat发送PlayerInitPlayerCorePush初始化播发器内核, 包括了视频路径,要注入的窗口等一些信息, Player接收到InitCore消息后会回复PlayerVideoSizeChangedPush和PlayerPrepareAsyncPush消息, 
	//其中PlayerVideoSizeChangedPush包含了视频的宽高, PlayerPrepareAsyncPush包含了视频的时长, 当接收到PlayerPrepareAsyncPush消息时就说明视频准备好了
	//之后WeChat发送PlayerStartPush请求开始播放视频
	//之后WeChat会一直请求PlayerGetCurrentPositionMsPullReq来获取视频当前的播放进度


	//以下封装将异步操作暂时改为了同步操作
	class MMMOJOCALL_API PlayerManager : public XPluginManager
	{
	public:
		struct CoreStatus
		{
			bool create_success;	//是否创建成功
			int duration;			//视频时长
			int org_width;			//视频原始宽度
			int org_height;			//视频原始高度
			int cur_pos;			//视频当前播放到的位置
			int state;				//4表示正在播放 5表示暂停播放 8表示停止播放
		};

	public:
		PlayerManager();
		~PlayerManager();

		/**
		 * @brief 设置启动所需命令行参数.
		 * @param usr_lib_dir (--user-lib-path) 即微信mmmojo(_64).dll所在目录
		 * @param xlog_path (--xlog_path) 可以不设置 微信为C:\Users\{YourName}\AppData\Roaming\Tencent\WeChat\log\player
		 * @param xlog_prefix (--xlog_prefix) 可以不设置 微信为player
		 */
		bool SetSwitchArgs(const char* usr_lib_dir, const char* xlog_path = NULL, const char* xlog_prefix = NULL);

		/**
		 * @brief 启动MMMojo环境以及WeChatPlayer.exe程序.
		 * @return 成功返回true
		 */
		bool StartWeChatPlayer();

		/**
		 * @brief 销毁MMMojo环境以及WeChatPlayer.exe程序.
		 */
		void KillWeChatPlayer();

		/**
		 * @brief 设置连接状态 [不需要手动调用].
		 * @param true为连接上
		 */
		void SetConnectState(bool connect);

		/**
		 * @brief 获取连接状态
		 * @return 是否连接上
		 */
		bool GetConnectState();

		//以下为PlayerCore的封装

		/**
		 * @brief 创建一个播放器内核
		 * @return 内核ID 操作失败返回0
		 */
		int CreatePlayerCore();

		/**
		 * @brief 获取已创建的播放内核数量.
		 * @return 播放内核数量
		 */
		int GetPlayerCoreNum();

		/**
		 * @brief 初始化播放内核.
		 * @param palyer_id 播放器内核ID
		 * @param window_surface 要依附的窗口句柄
		 * @param file_name 视频文件名
		 * @param file_path 要播放的视频路径
		 * @param file_size 网络视频的大小 本地时为0
		 * @param is_local 视频是否来自本地文件 (false为来自网络)
		 * @param is_mute 是否静音
		 * @param volume 音量
		 * @param is_repeate 是否重复播放
		 * @param speed 播放速率
		 * @return 视频时长 失败返回0
		 */
		int InitPlayerCore(int player_id, HWND window_surface, const char* file_name, const char* file_path, int64_t file_size = 0, bool is_local = true, bool is_mute = false, float volume = 1.0, bool is_repeat = true, float speed = 1.0);

		/**
		 * @brief 改变PlayerCore的大小.
		 * @param player_id 内核ID
		 * @param width 新宽度
		 * @param height 新高度
		 * @return 是否操作成功
		 */
		bool ResziePlayerCore(int player_id, int width, int height);

		/**
		 * @brief 开始播放视频.
		 * @param 内核ID
		 * @return 操作是否成功
		 */
		bool StartVideo(int player_id);

		/**
		 * @brief 暂停播放视频.
		 * @param 内核ID
		 * @return 操作是否成功
		 */
		bool PauseVideo(int player_id);

		/**
		 * @brief 恢复播放视频.
		 * @param 内核ID
		 * @return 操作是否成功
		 */
		bool ResumeVideo(int player_id);

		/**
		 * @brief 停止播放视频.
		 * @param 内核ID
		 * @return 操作是否成功
		 */
		bool StopVideo(int player_id);

		/**
		 * @brief 设置/取消静音.
		 * @param player_id 内核ID
		 * @param is_mute 是否静音
		 * @return 操作是否成功
		 */
		bool MuteVideo(int player_id, bool is_mute = true);

		/**
		 * @brief 跳转播放进度.
		 * @param player_id 内核ID
		 * @param pos_ms 进度
		 * @return 操作是否成功
		 */
		bool SeekToVideo(int player_id, int pos_ms);

		/**
		 * @brief 设置注入的窗口.
		 * @param player_id 内核ID
		 * @param window_hwnd 要注入的窗口句柄
		 * @return 操作是否成功
		 */
		bool SetSurfaceVideo(int player_id, HWND window_hwnd);

		/**
		 * @brief 设置视频音量.
		 * @param player_id 内核ID
		 * @param volume 音量大小
		 * @return 操作是否成功
		 */
		bool SetVolumeVideo(int player_id, float volume);

		/**
		 * @brief 是否重复播放视频.
		 * @param player_id 内核ID
		 * @param is_repeat 是否重复播放
		 * @return 操作是否成功
		 */
		bool RepeatVideo(int player_id, bool is_repeat = true);

		/**
		 * @brief 设置播放速率.
		 * @param player_id 内核ID
		 * @param speed_ratio 播放速率
		 * @return 操作是否成功
		 */
		bool SetSpeedVideo(int player_id, float speed_ratio);

		/**
		 * @brief 获取当前视频播放进度.
		 * @param player_id 内核ID
		 * @return 
		 */
		int GetCurrentPosition(int player_id);

		/**
		 * @brief 销毁播放器内核.
		 * @param 内核ID
		 */
		void DestroyPlayerCore(int player_id);

		CoreStatus* GetPlayerCoreStatus(int player_id);

		//以下用于同步操作
		void WaitEvent(int player_id, int which_one);
		void RunEvent(int player_id, int which_one);

	private:
		int GetIdlePlayerCoreId();
		bool SetPlayerCoreIdIdle(int id);

		void CreateCoreStatusSync(int player_id);
		void DeleteCoreStatusSync(int player_id);

	private:
		bool m_wechatplayer_running;
		bool m_connect_state;				//是否连接上

		#define MAX_PLAYER_CORE_ID 256
		std::mutex m_player_core_mutex;					//用于互斥获取PlayerCore ID
		BYTE m_player_core_id[MAX_PLAYER_CORE_ID];		//空闲的PlayerCoreID

		std::map<int, CoreStatus> m_core_status;		//播放器内核状态
		
		struct SyncEvent
		{
			HANDLE hEventCreateCore;
			HANDLE hEventInitCore;
			HANDLE hEventGetPos;
		};
		std::map<int, SyncEvent> m_core_sync;	//用于同步内核操作
	};



}
}