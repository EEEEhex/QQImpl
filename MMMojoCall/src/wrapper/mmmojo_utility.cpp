#include "mmmojo_utility.h"
#include "utils.h"

namespace qqimpl
{
namespace mmmojocall
{
	void UtilityRemoteOnConnect(bool is_connected, void* user_data)
	{
		if (user_data != NULL)
		{
			UtilityManager* pThis = (UtilityManager*)user_data;
			pThis->SetConnectState(true);
		}
	}

	void UtilityRemoteOnDisConnect(void* user_data)
	{
		if (user_data != NULL)
		{
			UtilityManager* pThis = (UtilityManager*)user_data;
			pThis->SetConnectState(false);
		}
	}

	void UtilityReadOnPull(uint32_t request_id, const void* request_info, void* user_data)
	{
		if (user_data != NULL)
		{
			UtilityManager* pThis = (UtilityManager*)user_data;

			uint32_t pb_size;
			const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);
			
			pThis->CallUsrCallback(request_id, pb_data, pb_size, "pull");

			mmmojocall::RemoveReadInfo(request_info);
		}
	}

	void UtilityReadOnPush(uint32_t request_id, const void* request_info, void* user_data)
	{
		if (user_data != NULL)
		{
			UtilityManager* pThis = (UtilityManager*)user_data;

			uint32_t pb_size;
			const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);

			switch (request_id)
			{
			case RequestIdUtility::UtilityHiPush:
			{
				utility_protobuf::InitReqMessage init_req;
				init_req.set_expire_timestamp(2840115661);//2060.01.01 01:01:01 本来是1714082745
				init_req.set_type(1);
				std::string data_;
				init_req.SerializeToString(&data_);
				pThis->SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPullReq, false, RequestIdUtility::UtilityInitPullReq);
			}
				break;
			case RequestIdUtility::UtilityTextScanPushResp:
			{
				pThis->CallUsrCallback(request_id, pb_data, pb_size, "push");
			}
			default:
				break;
			}

			mmmojocall::RemoveReadInfo(request_info);
		}
	}


	UtilityManager::UtilityManager()
	{
		m_wechatutility_running = false;
		m_connect_state = false;
		m_usr_cb_pull = nullptr;
		m_usr_cb_push = nullptr;

		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteConnect, UtilityRemoteOnConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteDisconnect, UtilityRemoteOnDisConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPull, UtilityReadOnPull);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPush, UtilityReadOnPush);
	}

	UtilityManager::~UtilityManager()
	{
		if (m_wechatutility_running)
		{
			this->KillWeChatUtility();
		}
	}

	bool UtilityManager::SetUsrLibDir(const char* usr_lib_dir)
	{
		m_usr_lib_dir = usr_lib_dir;
		return __super::AppendSwitchNativeCmdLine("user-lib-dir", usr_lib_dir);
	}

	void UtilityManager::SetReadOnPull(LPFN_UTILITYREADONPUSHLL pfunc)
	{

		m_usr_cb_pull = pfunc;
	}

	void UtilityManager::SetReadOnPush(LPFN_UTILITYREADONPUSHLL pfunc)
	{
		m_usr_cb_push = pfunc;
	}

	bool UtilityManager::StartWeChatUtility()
	{
		//设置回调函数的data为此类指针
		__super::SetCallbackUsrData(this);

		bool bRet = __super::InitMMMojoEnv();
		m_wechatutility_running = bRet;
		return bRet;
	}

	void UtilityManager::KillWeChatUtility()
	{
		m_connect_state = false;

		m_wechatutility_running = false;
		__super::StopMMMojoEnv();
	}

	bool UtilityManager::DoPicQRScan(const char* pic_path, int text_scan_id)
	{
		if (!m_wechatutility_running)
		{
			__super::SetLastErrStr(utils::string_format("%s: WeChatUtility is not RUNNING", __FUNCTION__));
			return false;
		}

		std::string pic_path_str = pic_path;
		if (pic_path_str.empty() || utils::CheckPathInfo(pic_path_str) != 2)
		{
			__super::SetLastErrStr(utils::string_format("%s: Arg is Invaild", __FUNCTION__));
			return false;
		}

		//在Send之前, 必须等待Utility进程启动
		std::unique_lock<std::mutex> lock(m_connect_mutex);
		m_connect_con_var.wait(lock, [this]() { return m_connect_state; });

		//创建QRScanReq的pb数据
		utility_protobuf::QRScanReqMessage qrscan_request;
		qrscan_request.set_origin_pic(pic_path_str);
		qrscan_request.set_decode_pic(std::string());
		qrscan_request.set_unknown_0(0);
		qrscan_request.set_unknown_1(1);
		qrscan_request.set_text_scan_id(text_scan_id);
		qrscan_request.set_unknown_3(1);

		std::string data_;
		qrscan_request.SerializeToString(&data_);

		__super::SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPullReq, false, RequestIdUtility::UtilityPicQRScanPullReq);
		return true;
	}

	bool UtilityManager::DoResampleImage(std::string origin_encode_path, std::string decode_pic_path, int pic_x, int pic_y)
	{
		if (!m_wechatutility_running)
		{
			__super::SetLastErrStr(utils::string_format("%s: WeChatUtility is not RUNNING", __FUNCTION__));
			return false;
		}

		if (origin_encode_path.empty() || utils::CheckPathInfo(origin_encode_path) != 2)
		{
			__super::SetLastErrStr(utils::string_format("%s: Arg is Invaild", __FUNCTION__));
			return false;
		}

		//在Send之前, 必须等待Utility进程启动
		std::unique_lock<std::mutex> lock(m_connect_mutex);
		m_connect_con_var.wait(lock, [this]() { return m_connect_state; });
		
		//发送Pb请求
		utility_protobuf::ResampleImageReqMessage resamlpe_img_req;
		resamlpe_img_req.set_decode_pic(origin_encode_path);
		resamlpe_img_req.set_encode_pic(decode_pic_path);
		resamlpe_img_req.set_re_pic_x(pic_x);
		resamlpe_img_req.set_re_pic_y(pic_y);
		resamlpe_img_req.set_unknown_0(1);

		std::string data_;
		resamlpe_img_req.SerializeToString(&data_);

		__super::SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPullReq, false, RequestIdUtility::UtilityResampleImagePullReq);
		return true;
	}

	void UtilityManager::SetConnectState(bool connect)
	{
		std::lock_guard<std::mutex> lock(m_connect_mutex);
		m_connect_state = connect;
		m_connect_con_var.notify_all();
	}

	bool UtilityManager::GetConnectState()
	{
		return m_connect_state;
	}

	void UtilityManager::CallUsrCallback(int request_id, const void* serialized_data, int data_size, std::string pull_or_push)
	{
		if (pull_or_push == "pull")
		{
			if (m_usr_cb_pull != nullptr)
			{
				m_usr_cb_pull(request_id, serialized_data, data_size);
			}
		}
		else if (pull_or_push == "push")
		{
			if (m_usr_cb_push != nullptr)
			{
				m_usr_cb_push(request_id, serialized_data, data_size);
			}
		}
	}
}
}
