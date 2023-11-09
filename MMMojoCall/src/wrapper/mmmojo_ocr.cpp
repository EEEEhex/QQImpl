#include "mmmojo_ocr.h"
#include "utils.h"

namespace qqimpl
{
namespace mmmojocall
{
	void OCRRemoteOnConnect(bool is_connected, void* user_data)
	{
		if (user_data != NULL)
		{
			OCRManager* pThis = (OCRManager*)user_data;
			pThis->SetConnectState(true);
		}
	}

	void OCRRemoteOnDisConnect(void* user_data)
	{
		if (user_data != NULL)
		{
			OCRManager* pThis = (OCRManager*)user_data;
			pThis->SetConnectState(false);
		}
	}

	void OCRReadOnPush(uint32_t request_id, const void* request_info, void* user_data)
	{
		if (user_data != NULL)
		{
			OCRManager* pThis = (OCRManager*)user_data;

			uint32_t pb_size;
			const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);
		
			pThis->CallUsrCallback(request_id, pb_data, pb_size);
			
			mmmojocall::RemoveReadInfo(request_info);
		}
	}

	OCRManager::OCRManager()
	{
		m_wechatocr_running = false;
		m_connect_state = false;
		m_usr_callback = nullptr;

		//判断是否为64位系统
		SYSTEM_INFO si; GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			m_is_arch64 = true;
		else
			m_is_arch64 = false;

		//初始化任务ID
		for (size_t i = 0; i < OCR_MAX_TASK_ID; i++) SetTaskIdIdle(i + 1);

		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteConnect, OCRRemoteOnConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMRemoteDisconnect, OCRRemoteOnDisConnect);
		__super::SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPush, OCRReadOnPush);
	}

	OCRManager::~OCRManager()
	{
		if (m_wechatocr_running)
		{
			this->KillWeChatOCR();
		}
	}

	bool OCRManager::SetUsrLibDir(const char* usr_lib_dir)
	{
		m_usr_lib_dir = usr_lib_dir;
		return __super::AppendSwitchNativeCmdLine("user-lib-dir", usr_lib_dir);
	}

	void OCRManager::SetReadOnPush(LPFN_OCRREADONPUSH pfunc)
	{
		if (pfunc != nullptr)
		{
			m_usr_callback = pfunc;
		}
	}

	bool OCRManager::StartWeChatOCR()
	{
		if (!m_is_arch64)
		{
			__super::SetLastErrStr(utils::string_format("%s: WeChatOCR.exe must run on 64bit system!", __FUNCTION__));
			return false;
		}

		//设置回调函数的data为此类指针
		__super::SetCallbackUsrData(this);

		bool bRet = __super::InitMMMojoEnv();
		m_wechatocr_running = bRet;
		return bRet;
	}

	void OCRManager::KillWeChatOCR()
	{
		m_connect_state = false;
		m_id_path.clear();
		for (size_t i = 0; i < OCR_MAX_TASK_ID; i++) SetTaskIdIdle(i + 1);

		m_wechatocr_running = false;
		__super::StopMMMojoEnv();
	}

	bool OCRManager::DoOCRTask(const char* pic_path)
	{
		if (!m_wechatocr_running)
		{
			__super::SetLastErrStr(utils::string_format("%s: WeChatOCR is not RUNNING", __FUNCTION__));
			return false;
		}

		std::string pic_path_str = pic_path;
		if (pic_path_str.empty() || utils::CheckPathInfo(pic_path_str) != 2)
		{
			__super::SetLastErrStr(utils::string_format("%s: Arg is Invaild", __FUNCTION__));
			return false;
		}

		int id = GetIdleTaskId();
		if (id == 0)
		{
			__super::SetLastErrStr(utils::string_format("%s: No Idle TASK ID", __FUNCTION__));
			return false;
		}

		//在Send之前, 必须等待OCR进程启动
		std::unique_lock<std::mutex> lock(m_connect_mutex);
		m_connect_con_var.wait(lock, [this]() { return m_connect_state; });

		if (!SendOCRTask(id, pic_path))
		{
			SetTaskIdIdle(id);
			__super::SetLastErrStr(utils::string_format("%s: SendOCRTask Err", __FUNCTION__));
			return false;
		}

		return true;
	}

	void OCRManager::SetConnectState(bool connect)
	{
		std::lock_guard<std::mutex> lock(m_connect_mutex);
		m_connect_state = connect;
		m_connect_con_var.notify_all();
	}

	
	bool OCRManager::GetConnectState()
	{
		return m_connect_state;
	}

	void OCRManager::CallUsrCallback(int request_id, const void* serialized_data, int data_size)
	{
		ocr_protobuf::OcrResponse ocr_response;
		ocr_response.ParseFromArray(serialized_data, data_size);
		uint32_t task_id = ocr_response.task_id();
		uint32_t type = ocr_response.type();

		if (type == 0)//如果Start后立马发送OCR任务请求 则启动PUSH回调时就记录了ID和PIC的信息 会被启动回调抹掉 所以需要判断一下
		{
			if (task_id >= 1 && task_id <= OCR_MAX_TASK_ID)
			{
				std::string pic_path;
				const char* pic_path_ptr = nullptr;
				if (m_id_path.count(task_id) > 0)
				{
					pic_path = m_id_path[task_id];
					pic_path_ptr = pic_path.c_str();
				}
					
				if (m_usr_callback != nullptr)
				{
					m_usr_callback(pic_path_ptr, serialized_data, data_size);
				}

				//删除id与pic_path的map
				SetTaskIdIdle(task_id);
				m_id_path.erase(task_id);
			}
		}
	}

	bool OCRManager::SendOCRTask(uint32_t task_id, std::string pic_path)
	{
		m_id_path[task_id] = pic_path;//记录ID和图片路径的关系

		//创建OcrRequest的pb数据
		ocr_protobuf::OcrRequest ocr_request;
		ocr_request.set_unknow(0);
		ocr_request.set_task_id(task_id);
		ocr_protobuf::OcrRequest::PicPaths* pp = new ocr_protobuf::OcrRequest::PicPaths();
		pp->add_pic_path(pic_path);
		ocr_request.set_allocated_pic_path(pp);

		std::string data_;
		ocr_request.SerializeToString(&data_);

		__super::SendPbSerializedData((void*)(data_.data()), data_.size(), MMMojoInfoMethod::kMMPush, false, RequestIdOCR::OCRPush);
		return true;
	}

	int OCRManager::GetIdleTaskId()
	{
		m_task_mutex.lock();
		for (size_t i = 0; i < OCR_MAX_TASK_ID; i++)
		{
			if (m_task_id[i] == 0)
			{
				m_task_id[i] = 1;
				m_task_mutex.unlock();
				return (i + 1);
			}
		}
		m_task_mutex.unlock();
		return 0;//返回0说明没有空闲的
	}

	bool OCRManager::SetTaskIdIdle(int id)
	{
		if (id < 1 || id > OCR_MAX_TASK_ID) return false;
		m_task_id[id - 1] = 0;
		return true;
	}
}
}
