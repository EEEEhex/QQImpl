#include "mmmojo_player.h"
#include "utils.h"

#include "player_protobuf.pb.h"

namespace qqimpl
{
namespace mmmojocall
{
	void PlayerRemoteOnConnect(bool is_connected, void* user_data)
	{
		if (user_data != NULL)
		{
			PlayerManager* pThis = (PlayerManager*)user_data;
			pThis->SetConnectState(true);
		}
	}

	void PlayerRemoteOnDisConnect(void* user_data)
	{
		if (user_data != NULL)
		{
			PlayerManager* pThis = (PlayerManager*)user_data;
			pThis->SetConnectState(false);
		}
	}

	void PlayerReadOnPull(uint32_t request_id, const void* request_info, void* user_data)
	{
#ifdef _DEBUG
		//std::cout << "\033[31mPull: [ " << request_id << " ]\033[0m\n";
#endif // _DEBUG

		if (user_data != NULL)
		{
			PlayerManager* pThis = (PlayerManager*)user_data;

			uint32_t pb_size;
			const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);

			switch (request_id)
			{
			case mmmojocall::RequestIdPlayer::PlayerInitPullResp:
			{
				//std::cout << "ReadOnPull PlayerInitPullResp ";

				//Player回复初始化状态
				player_protobuf::InitRespMessage init_resp;
				init_resp.ParseFromArray(pb_data, pb_size);
				int status = init_resp.status();
				//std::cout << status << std::endl;
				if (status != 1)
				{
					//未初始化成功
				}
			}
				break;
			case mmmojocall::RequestIdPlayer::PlayerCreatePlayerCorePullResp:
			{
				//WeChatPlayer回复创建Core的请求
				//std::cout << "ReadOnPull PlayerCreatePlayerCorePullResp ";

				player_protobuf::CreatePlayerCoreRespMessage cpc_resp;
				cpc_resp.ParseFromArray(pb_data, pb_size);
				//std::cout << cpc_resp.player_id() << " | " << cpc_resp.is_suceess() << std::endl;

				PlayerManager::CoreStatus* pCore = pThis->GetPlayerCoreStatus(cpc_resp.player_id());
				pCore->create_success = cpc_resp.is_suceess();
				pThis->RunEvent(cpc_resp.player_id(), 1);//让请求创建Core的函数继续运行
			}
				break;
			case mmmojocall::RequestIdPlayer::PlayerGetCurrentPositionMsPullResp:
			{
				//std::cout << "ReadOnPull PlayerGetCurrentPositionMsPullResp:\n{\n";
				player_protobuf::CurrentPositionMsInfo pos_info;
				pos_info.ParseFromArray(pb_data, pb_size);
				
				//for (size_t i = 0; i < pb_size; i++)
				//{
				//	printf("%02X ", (BYTE)(((BYTE*)pb_data)[i]));
				//}
				//puts("");

				for (int i = 0; i < pos_info.ms_info_size(); i++)
				{
					std::string pb_internal = pos_info.ms_info(i);

					player_protobuf::CurrentPositionMsInfo::MsInfo ms_info;
					ms_info.ParseFromString(pb_internal);
					//std::cout << "ID: " << ms_info.player_id() << " | Pos: " << ms_info.pos() << std::endl;

					PlayerManager::CoreStatus* pCore = pThis->GetPlayerCoreStatus(ms_info.player_id());
					pCore->cur_pos = ms_info.pos();
					pThis->RunEvent(ms_info.player_id(), 3);
				}
				//std::cout << "}\n";
			}
				break;
			default:
				break;
			}

			mmmojocall::RemoveReadInfo(request_info);
		}
	}


	void PlayerReadOnPush(uint32_t request_id, const void* request_info, void* user_data)
	{
#ifdef _DEBUG
		//std::cout << "\033[31mPush: [ " << request_id << " ]\033[0m\n";
#endif // _DEBUG
		
		if (user_data != NULL)
		{
			PlayerManager* pThis = (PlayerManager*)user_data;

			uint32_t pb_size;
			const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);

			switch (request_id)
			{
			case RequestIdPlayer::PlayerHiPush:
			{
				//std::cout << "ReadOnPush PlayerHiPush\n";
				player_protobuf::InitReqMessage init_req;
				init_req.set_expire_timestamp(2840115661);
				init_req.set_type(1);
				std::string data_;
				init_req.SerializeToString(&data_);
				pThis->SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPullReq, false, mmmojocall::RequestIdPlayer::PlayerInitPullReq);
			}
				break;
			case RequestIdPlayer::PlayerInfoPush:
			{
				//std::cout << "ReadOnPush PlayerInfoPush: ";
				player_protobuf::InfoPlayerCoreMessage player_info_msg;
				player_info_msg.ParseFromArray(pb_data, pb_size);
				//std::cout << "ID: " << player_info_msg.player_id() << " | type: " << player_info_msg.type() << " | arg: " << player_info_msg.arg()
				//	<< " | unknown: " << player_info_msg.unknown_2() << std::endl;

			}
				break;
			case RequestIdPlayer::PlayerVideoSizeChangedPush:
			{
				//std::cout << "ReadOnPush PlayerVideoSizeChangedPush: ";
				player_protobuf::VideoSizeChangedPlayerCoreMessage size_change_msg;
				size_change_msg.ParseFromArray(pb_data, pb_size);
				//std::cout << "ID: " << size_change_msg.player_id() << " | OrgWidth: " << size_change_msg.org_width() << " | OrgHeight: " << size_change_msg.org_height() << std::endl;
				PlayerManager::CoreStatus* pCore = pThis->GetPlayerCoreStatus(size_change_msg.player_id());
				pCore->org_height = size_change_msg.org_height();
				pCore->org_width = size_change_msg.org_width();
			}
				break;
			case RequestIdPlayer::PlayerPrepareAsyncPush:
			{
				//std::cout << "ReadOnPush PlayerPrepareAsyncPush: ";
				player_protobuf::PreparePlayerCoreMessage async_msg;
				async_msg.ParseFromArray(pb_data, pb_size);
				//std::cout << "ID: " << async_msg.player_id() << " | Time: " << async_msg.duration() << std::endl;
				PlayerManager::CoreStatus* pCore = pThis->GetPlayerCoreStatus(async_msg.player_id());
				pCore->duration = async_msg.duration();
				pThis->RunEvent(async_msg.player_id(), 2);//让请求InitCore的函数继续运行

			}
				break;
			case RequestIdPlayer::PlayerStateChangePush:
			{
				//std::cout << "ReadOnPush PlayerStateChangePush: ";
				player_protobuf::StatePlayerCoreMessage state_msg;
				state_msg.ParseFromArray(pb_data, pb_size);
				//std::cout << state_msg.player_id() << " | " << state_msg.state() << std::endl;
			}
				break;
			case RequestIdPlayer::PlayerCompletedPush:
			{
				//std::cout << "ReadOnPush PlayerCompletedPush\n";
			}
			case RequestIdPlayer::PlayerSeekCompletedPush:
			{
				//std::cout << "ReadOnPush PlayerSeekCompletedPush\n";
			}
				break;
			default:
				break;
			}

			mmmojocall::RemoveReadInfo(request_info);
		}
	}

	PlayerManager::PlayerManager()
	{
		m_connect_state = false;
		m_wechatplayer_running = false;

		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteConnect, PlayerRemoteOnConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteDisconnect, PlayerRemoteOnDisConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPull, PlayerReadOnPull);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPush, PlayerReadOnPush);

		//初始化PlayerCore ID
		for (size_t i = 0; i < MAX_PLAYER_CORE_ID; i++) SetPlayerCoreIdIdle(i + 1);
	}

	PlayerManager::~PlayerManager()
	{
		if (m_wechatplayer_running != false)
		{
			this->KillWeChatPlayer();
		}
	}

	bool PlayerManager::SetSwitchArgs(const char* usr_lib_dir, const char* xlog_path, const char* xlog_prefix)
	{
		if (xlog_path != NULL)
			__super::AppendSwitchNativeCmdLine("xlog_path", xlog_path);
		if (xlog_prefix != NULL)
			__super::AppendSwitchNativeCmdLine("xlog_prefix", xlog_prefix);

		return __super::AppendSwitchNativeCmdLine("user-lib-dir", usr_lib_dir);
	}

	bool PlayerManager::StartWeChatPlayer()
	{
		//设置回调函数的data为此类指针
		__super::SetCallbackUsrData(this);

		bool bRet = __super::StartMMMojoEnv();
		m_wechatplayer_running = bRet;
		return bRet;
	}

	void PlayerManager::KillWeChatPlayer()
	{
		m_wechatplayer_running = false;
		__super::StopMMMojoEnv();
	}

	void PlayerManager::SetConnectState(bool connect)
	{
		m_connect_state = connect;
	}

	bool PlayerManager::GetConnectState()
	{
		return m_connect_state;
	}

	int PlayerManager::CreatePlayerCore()
	{
		if (!m_connect_state)
		{
			__super::SetLastErrStr(utils::string_format("%s: WeChatPlayer is not CONNECTED", __FUNCTION__));
			return 0;
		}

		//获取一个PlayerCore ID
		int player_id = GetIdlePlayerCoreId();
		if (player_id == 0)
		{
			__super::SetLastErrStr(utils::string_format("%s: No Idle Player Core ID", __FUNCTION__));
			return 0;
		}

		//创建一个播放内核信息
		CreateCoreStatusSync(player_id);

		//发送CreatePlayerCore请求
		player_protobuf::CreatePlayerCoreReqMessage create_playercore_req;
		create_playercore_req.set_player_id(player_id);
		create_playercore_req.set_is_post_frame(1);
		std::string data_;
		create_playercore_req.SerializeToString(&data_);
		SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPullReq, false, RequestIdPlayer::PlayerCreatePlayerCorePullReq);

		this->WaitEvent(player_id, 1);//等待回调完成
		if (m_core_status[player_id].create_success)
		{
			return player_id;
		}
		else
		{
			DeleteCoreStatusSync(player_id);
			SetPlayerCoreIdIdle(player_id);
			return 0;
		}

		return 0;
	}

	int PlayerManager::GetPlayerCoreNum()
	{
		return m_core_status.size();
	}

	int PlayerManager::InitPlayerCore(int player_id, HWND window_surface, const char* file_name, const char* file_path, int64_t file_size, bool is_local, bool is_mute, float volume, bool is_repeat, float speed)
	{
		if (window_surface == NULL)
		{
			__super::SetLastErrStr(utils::string_format("%s:No Window Handle:", __FUNCTION__));
			return 0;
		}

		//来自网络 但没有size
		if (!is_local && (file_size == 0))
		{
			__super::SetLastErrStr(utils::string_format("%s: Arg file_size can't be ZERO", __FUNCTION__));
			return 0;
		}

		player_protobuf::InitPlayerCoreMessage init_playercore_msg;
		init_playercore_msg.set_player_id(player_id);
		init_playercore_msg.set_is_local(is_local);
		init_playercore_msg.set_file_name(file_name);
		init_playercore_msg.set_file_path(file_path);
		init_playercore_msg.set_placeholder_6(0);
		init_playercore_msg.set_media_id(1);
		init_playercore_msg.set_is_audio_mute(is_mute);
		init_playercore_msg.set_video_surface((int32_t)window_surface);
		init_playercore_msg.set_audio_volume(volume);
		init_playercore_msg.set_is_repeat(1);
		init_playercore_msg.set_speed_ratio(speed);
		init_playercore_msg.set_max_frame_height(0);
		init_playercore_msg.set_max_frame_width(0);
		init_playercore_msg.set_codec_type(-1);
		init_playercore_msg.set_decoder_type(-1);

		std::string data_;
		init_playercore_msg.SerializeToString(&data_);
		SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerInitPlayerCorePush);

		//等待回调完成
		this->WaitEvent(player_id, 2);
		return m_core_status[player_id].duration;
	}

	bool PlayerManager::ResziePlayerCore(int player_id, int width, int height)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::ResizePlayerCoreMessage resize_msg;
			resize_msg.set_player_id(player_id);
			resize_msg.set_new_width(width);
			resize_msg.set_new_height(height);

			std::string data_;
			resize_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerResizePush);
			return true;
		}
		//不存在该player_id
		return false;
	}

	bool PlayerManager::StartVideo(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::PlayerIdMessage id_msg;
			id_msg.set_player_id(player_id);

			std::string data_;
			id_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerStartPush);
			return true;
		}
		return false;
	}

	bool PlayerManager::PauseVideo(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::PlayerIdMessage id_msg;
			id_msg.set_player_id(player_id);

			std::string data_;
			id_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerPausePush);
			return true;
		}
		return false;
	}

	bool PlayerManager::ResumeVideo(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::PlayerIdMessage id_msg;
			id_msg.set_player_id(player_id);

			std::string data_;
			id_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerResumePush);
		}
		return false;
	}

	bool PlayerManager::StopVideo(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::PlayerIdMessage id_msg;
			id_msg.set_player_id(player_id);

			std::string data_;
			id_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerStopPush);
			return true;
		}

		return false;
	}

	bool PlayerManager::MuteVideo(int player_id, bool is_mute)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::AudioMutePlayerCoreMessage mute_msg;
			mute_msg.set_player_id(player_id);
			mute_msg.set_is_audio_mute(is_mute);

			std::string data_;
			mute_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSetAudioMutePush);
			return true;
		}

		return false;
	}

	bool PlayerManager::SeekToVideo(int player_id, int pos_ms)
	{
		if ((m_core_status.count(player_id) > 0) && (pos_ms >= 0) && (pos_ms <= m_core_status[player_id].duration))
		{
			player_protobuf::SeekToPlayerCoreMessage seek_msg;
			seek_msg.set_player_id(player_id);
			seek_msg.set_pos_ms(pos_ms);

			std::string data_;
			seek_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSeekToAsyncPush);
			return true;
		}

		return false;
	}

	bool PlayerManager::SetSurfaceVideo(int player_id, HWND window_hwnd)
	{
		if ((m_core_status.count(player_id) > 0) && (window_hwnd != NULL))
		{
			player_protobuf::VideoSurfaceMessage surface_msg;
			surface_msg.set_player_id(player_id);
			surface_msg.set_window((int32_t)window_hwnd);

			std::string data_;
			surface_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSetVideoSurfacePush);
			return true;
		}
		return false;
	}

	bool PlayerManager::SetVolumeVideo(int player_id, float volume)
	{
		if ((m_core_status.count(player_id) > 0) && (volume >= 0.0))
		{
			player_protobuf::AudioVolumePlayerCoreMessage volume_msg;
			volume_msg.set_player_id(player_id);
			volume_msg.set_audio_volume(volume);

			std::string data_;
			volume_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSetAudioVolumePush);
			return true;
		}
		return false;
	}

	bool PlayerManager::RepeatVideo(int player_id, bool is_repeat)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::RepeatPlayerCoreMessage repeat_msg;
			repeat_msg.set_player_id(player_id);
			repeat_msg.set_is_repeat(is_repeat);

			std::string data_;
			repeat_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSetRepeatPush);
			return true;
		}

		return false;
	}

	bool PlayerManager::SetSpeedVideo(int player_id, float speed_ratio)
	{
		if ((m_core_status.count(player_id) > 0) && (speed_ratio >= 0.0))
		{
			player_protobuf::SpeedRatioPlayerCoreMessage speed_msg;
			speed_msg.set_player_id(player_id);
			speed_msg.set_speed_ratio(speed_ratio);

			std::string data_;
			speed_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerSetPlaySpeedRatio);
			return true;
		}
		return false;
	}

	int PlayerManager::GetCurrentPosition(int player_id)
	{
		char data_[2] = {0x00, 0x00};
		SendPbSerializedData((void*)(data_), sizeof(data_), MMMojoInfoMethod::kMMPullReq, false, RequestIdPlayer::PlayerGetCurrentPositionMsPullReq);
		this->WaitEvent(player_id, 3);//等待回调完成
		return m_core_status[player_id].cur_pos;
	}

	void PlayerManager::DestroyPlayerCore(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			player_protobuf::PlayerIdMessage id_msg;
			id_msg.set_player_id(player_id);

			std::string data_;
			id_msg.SerializeToString(&data_);
			SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdPlayer::PlayerDestroyPlayerCorePush);
			
			//清除m_core_status和ID占用
			DeleteCoreStatusSync(player_id);
			SetPlayerCoreIdIdle(player_id);
		}
	}

	PlayerManager::CoreStatus* PlayerManager::GetPlayerCoreStatus(int player_id)
	{
		if (m_core_status.count(player_id) > 0)
		{
			return &(m_core_status[player_id]);
		}

		return NULL;
	}

	void PlayerManager::WaitEvent(int player_id, int which_one)
	{
		HANDLE hHandle = NULL;
		switch (which_one)
		{
		case 1:
			hHandle = m_core_sync[player_id].hEventCreateCore;
			break;
		case 2:
			hHandle = m_core_sync[player_id].hEventInitCore;
			break;
		case 3:
			hHandle = m_core_sync[player_id].hEventGetPos;
			break;
		default:
			break;
		}

		if (hHandle != NULL)
		{
			WaitForSingleObject(hHandle, INFINITE);
			ResetEvent(hHandle);
		}
	}

	void PlayerManager::RunEvent(int player_id, int which_one)
	{
		HANDLE hHandle = NULL;
		switch (which_one)
		{
		case 1:
			hHandle = m_core_sync[player_id].hEventCreateCore;
			break;
		case 2:
			hHandle = m_core_sync[player_id].hEventInitCore;
			break;
		case 3:
			hHandle = m_core_sync[player_id].hEventGetPos;
			break;
		default:
			break;
		}

		if (hHandle != NULL)
		{
			SetEvent(hHandle);
		}
	}

	int PlayerManager::GetIdlePlayerCoreId()
	{
		m_player_core_mutex.lock();
		for (size_t i = 0; i < MAX_PLAYER_CORE_ID; i++)
		{
			if (m_player_core_id[i] == 0)
			{
				m_player_core_id[i] = 1;
				m_player_core_mutex.unlock();
				return (i + 1);
			}
		}
		m_player_core_mutex.unlock();
		return 0;//返回0说明没有空闲的
	}

	bool PlayerManager::SetPlayerCoreIdIdle(int id)
	{
		if (id < 1 || id > MAX_PLAYER_CORE_ID) return false;
		m_player_core_id[id - 1] = 0;
		return true;
	}

	void PlayerManager::CreateCoreStatusSync(int player_id)
	{
		SyncEvent sync_event = { 0 };
		CoreStatus core_status = { 0 };
		sync_event.hEventCreateCore = CreateEventA(NULL, TRUE, FALSE, NULL);
		sync_event.hEventInitCore = CreateEventA(NULL, TRUE, FALSE, NULL);
		sync_event.hEventGetPos = CreateEventA(NULL, TRUE, FALSE, NULL);

		m_core_status[player_id] = core_status;
		m_core_sync[player_id] = sync_event;
	}

	void PlayerManager::DeleteCoreStatusSync(int player_id)
	{
		CloseHandle(m_core_sync[player_id].hEventCreateCore);
		CloseHandle(m_core_sync[player_id].hEventInitCore);
		CloseHandle(m_core_sync[player_id].hEventGetPos);

		m_core_status.erase(player_id);
		m_core_sync.erase(player_id);
	}
}
}