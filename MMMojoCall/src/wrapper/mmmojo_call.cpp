#include "mmmojo_call.h"
#include "utils.h"

#include <Windows.h>

namespace qqimpl
{
namespace mmmojocall
{
	//默认回调函数
	static void DefaultReadOnPush(uint32_t request_id, const void* request_info, void* user_data);
	static void DefaultReadOnPull(uint32_t request_id, const void* request_info, void* user_data);
	static void DefaultReadOnShared(uint32_t request_id, const void* request_info, void* user_data);
	static void DefaultRemoteOnConnect(bool is_connected, void* user_data);
	static void DefaultRemoteOnDisConnect(void* user_data);
	static void DefaultRemoteOnProcessLaunched(void* user_data);
	static void DefaultRemoteOnProcessLaunchFailed(int error_code, void* user_data);
	static void DefaultRemoteOnMojoError(const void* errorbuf, int errorsize, void* user_data);

	struct MMMojoDLL
	{
		HMODULE	hMMMojo;
		GetMMMojoReadInfoRequest* lpGetMMMojoReadInfoRequest;
		GetMMMojoReadInfoAttach* lpGetMMMojoReadInfoAttach;
		GetMMMojoReadInfoMethod* lpGetMMMojoReadInfoMethod;
		GetMMMojoReadInfoSync* lpGetMMMojoReadInfoSync;
		RemoveMMMojoReadInfo* lpRemoveMMMojoReadInfo;
		CreateMMMojoWriteInfo* lpCreateMMMojoWriteInfo;
		SetMMMojoWriteInfoMessagePipe* lpSetMMMojoWriteInfoMessagePipe;
		SetMMMojoWriteInfoResponseSync* lpSetMMMojoWriteInfoResponseSync;
		GetMMMojoWriteInfoRequest* lpGetMMMojoWriteInfoRequest;
		GetMMMojoWriteInfoAttach* lpGetMMMojoWriteInfoAttach;
		SwapMMMojoWriteInfoCallback* lpSwapMMMojoWriteInfoCallback;
		SwapMMMojoWriteInfoMessage* lpSwapMMMojoWriteInfoMessage;
		SendMMMojoWriteInfo* lpSendMMMojoWriteInfo;
		RemoveMMMojoWriteInfo* lpRemoveMMMojoWriteInfo;
		CreateMMMojoEnvironment* lpCreateMMMojoEnvironment;
		SetMMMojoEnvironmentCallbacks* lpSetMMMojoEnvironmentCallbacks;
		SetMMMojoEnvironmentInitParams* lpSetMMMojoEnvironmentInitParams;
		AppendMMSubProcessSwitchNative* lpAppendMMSubProcessSwitchNative;
		StartMMMojoEnvironment* lpStartMMMojoEnvironment;
		StopMMMojoEnvironment* lpStopMMMojoEnvironment;
		RemoveMMMojoEnvironment* lpRemoveMMMojoEnvironment;
		InitializeMMMojo* lpInitializeMMMojo;
		ShutdownMMMojo* lpShutdownMMMojo;
	}g_MMMojoDLL;

	bool InitMMMojoDLLFuncs(const char* mmmojo_dll_path)
	{
		std::string smmmojo_dll_path = mmmojo_dll_path;
		if (smmmojo_dll_path.empty() || (utils::CheckPathInfo(smmmojo_dll_path) != 1))
			return false;

		if (g_MMMojoDLL.hMMMojo == NULL || g_MMMojoDLL.lpInitializeMMMojo == NULL)
		{
			std::string dll_path = smmmojo_dll_path, dll_name;
#ifdef _WIN64
			dll_name = "mmmojo_64.dll";
#else
			dll_name = "mmmojo.dll";
#endif // _WIN64

			if ((dll_path.back() == '\\') || (dll_path.back() == '/'))
				dll_path += dll_name;
			else
				dll_path += ("\\" + dll_name);

			g_MMMojoDLL.hMMMojo = LoadLibraryA(dll_path.c_str());
			HMODULE hMMMojo = g_MMMojoDLL.hMMMojo;
			if (hMMMojo == NULL)
				return false;

			g_MMMojoDLL.lpInitializeMMMojo = (InitializeMMMojo*)GetProcAddress(hMMMojo, "InitializeMMMojo");
			g_MMMojoDLL.lpShutdownMMMojo = (ShutdownMMMojo*)GetProcAddress(hMMMojo, "ShutdownMMMojo");
			g_MMMojoDLL.lpCreateMMMojoEnvironment = (CreateMMMojoEnvironment*)GetProcAddress(hMMMojo, "CreateMMMojoEnvironment");
			g_MMMojoDLL.lpSetMMMojoEnvironmentCallbacks = (SetMMMojoEnvironmentCallbacks*)GetProcAddress(hMMMojo, "SetMMMojoEnvironmentCallbacks");
			g_MMMojoDLL.lpSetMMMojoEnvironmentInitParams = (SetMMMojoEnvironmentInitParams*)GetProcAddress(hMMMojo, "SetMMMojoEnvironmentInitParams");
			g_MMMojoDLL.lpAppendMMSubProcessSwitchNative = (AppendMMSubProcessSwitchNative*)GetProcAddress(hMMMojo, "AppendMMSubProcessSwitchNative");
			g_MMMojoDLL.lpStartMMMojoEnvironment = (StartMMMojoEnvironment*)GetProcAddress(hMMMojo, "StartMMMojoEnvironment");
			g_MMMojoDLL.lpStopMMMojoEnvironment = (StopMMMojoEnvironment*)GetProcAddress(hMMMojo, "StopMMMojoEnvironment");
			g_MMMojoDLL.lpRemoveMMMojoEnvironment = (RemoveMMMojoEnvironment*)GetProcAddress(hMMMojo, "RemoveMMMojoEnvironment");
			g_MMMojoDLL.lpGetMMMojoReadInfoRequest = (GetMMMojoReadInfoRequest*)GetProcAddress(hMMMojo, "GetMMMojoReadInfoRequest");
			g_MMMojoDLL.lpGetMMMojoReadInfoAttach = (GetMMMojoReadInfoAttach*)GetProcAddress(hMMMojo, "GetMMMojoReadInfoAttach");
			g_MMMojoDLL.lpRemoveMMMojoReadInfo = (RemoveMMMojoReadInfo*)GetProcAddress(hMMMojo, "RemoveMMMojoReadInfo");
			g_MMMojoDLL.lpGetMMMojoReadInfoMethod = (GetMMMojoReadInfoMethod*)GetProcAddress(hMMMojo, "GetMMMojoReadInfoMethod");
			g_MMMojoDLL.lpGetMMMojoReadInfoSync = (GetMMMojoReadInfoSync*)GetProcAddress(hMMMojo, "GetMMMojoReadInfoSync");
			g_MMMojoDLL.lpCreateMMMojoWriteInfo = (CreateMMMojoWriteInfo*)GetProcAddress(hMMMojo, "CreateMMMojoWriteInfo");
			g_MMMojoDLL.lpGetMMMojoWriteInfoRequest = (GetMMMojoWriteInfoRequest*)GetProcAddress(hMMMojo, "GetMMMojoWriteInfoRequest");
			g_MMMojoDLL.lpRemoveMMMojoWriteInfo = (RemoveMMMojoWriteInfo*)GetProcAddress(hMMMojo, "RemoveMMMojoWriteInfo");
			g_MMMojoDLL.lpGetMMMojoWriteInfoAttach = (GetMMMojoWriteInfoAttach*)GetProcAddress(hMMMojo, "GetMMMojoWriteInfoAttach");
			g_MMMojoDLL.lpSetMMMojoWriteInfoMessagePipe = (SetMMMojoWriteInfoMessagePipe*)GetProcAddress(hMMMojo, "SetMMMojoWriteInfoMessagePipe");
			g_MMMojoDLL.lpSetMMMojoWriteInfoResponseSync = (SetMMMojoWriteInfoResponseSync*)GetProcAddress(hMMMojo, "SetMMMojoWriteInfoResponseSync");
			g_MMMojoDLL.lpSendMMMojoWriteInfo = (SendMMMojoWriteInfo*)GetProcAddress(hMMMojo, "SendMMMojoWriteInfo");
			g_MMMojoDLL.lpSwapMMMojoWriteInfoCallback = (SwapMMMojoWriteInfoCallback*)GetProcAddress(hMMMojo, "SwapMMMojoWriteInfoCallback");
			g_MMMojoDLL.lpSwapMMMojoWriteInfoMessage = (SwapMMMojoWriteInfoMessage*)GetProcAddress(hMMMojo, "SwapMMMojoWriteInfoMessage");

			if (g_MMMojoDLL.lpInitializeMMMojo == NULL)
				return false;
		}

		return true;
	}


	const void* GetPbSerializedData(const void* request_info, uint32_t& data_size)
	{
		const void* pb_data = nullptr;
		if (request_info != NULL)
		{
			uint32_t request_size = 0;
			const void* read_request = g_MMMojoDLL.lpGetMMMojoReadInfoRequest(request_info, &request_size);
			pb_data = read_request;
			data_size = request_size;
		}
		return pb_data;
	}

	const void* GetReadInfoAttachData(const void* request_info, uint32_t& data_size)
	{
		const void* attach_data = nullptr;
		if (request_info != NULL)
		{
			uint32_t attach_size = 0;
			const void* read_attach = g_MMMojoDLL.lpGetMMMojoReadInfoAttach(request_info, &attach_size);
			attach_data = read_attach;
			data_size = attach_size;
		}
		return attach_data;
	}

	void RemoveReadInfo(const void* request_info)
	{
		if (request_info != NULL)
			g_MMMojoDLL.lpRemoveMMMojoReadInfo((void*)request_info);
	}

	//以下为XPluginManager类的实现
	XPluginManager::XPluginManager()
	{
		m_last_err = "ERR_OK";
		m_mmmojo_env_ptr = nullptr;
		m_cb_usrdata = this;	//默认设为this
		m_callbacks.read_on_pull = DefaultReadOnPull;
		m_callbacks.read_on_push = DefaultReadOnPush;
		m_callbacks.read_on_shared = DefaultReadOnShared;
		m_callbacks.remote_on_connect = DefaultRemoteOnConnect;
		m_callbacks.remote_on_disconnect = DefaultRemoteOnDisConnect;
		m_callbacks.remote_on_mojoerror = DefaultRemoteOnMojoError;
		m_callbacks.remote_on_processlaunched = DefaultRemoteOnProcessLaunched;
		m_callbacks.remote_on_processlaunchfailed = DefaultRemoteOnProcessLaunchFailed;
		m_init_mmmojo_env = false;
	}

	XPluginManager::~XPluginManager()
	{
		if (m_init_mmmojo_env)
		{
			this->StopMMMojoEnv();
		}
	}

	bool XPluginManager::SetExePath(const char* exe_path)
	{
		if (utils::CheckPathInfo(exe_path) != 2)
		{
			m_last_err = utils::string_format("%s: Arg exe_path is Invaild", __FUNCTION__);
			return false;
		}

		m_exe_path = utils::Utf8ToUnicode(exe_path);
		return true;
	}

	bool XPluginManager::AppendSwitchNativeCmdLine(const char* switch_string, const char* value)
	{
		std::string switch_ = switch_string, value_ = value;
		if (switch_.empty() || value_.empty())
		{
			m_last_err = utils::string_format("%s: Arg is Invaild", __FUNCTION__);
			return false;
		}

		m_switch_native.emplace(switch_, value_);
		return true;
	}

	bool XPluginManager::SetCommandLine(int argc, std::vector<std::string>& argv)
	{
		if (argv.size() != argc)
		{
			m_last_err = utils::string_format("%s: Args not equal", __FUNCTION__);
			return false;
		}

		m_cmdline = argv;
		return true;
	}

	void XPluginManager::SetOneCallback(int type, void* pfunc)
	{
		switch (type)
		{
		case MMMojoEnvironmentCallbackType::kMMReadPull:
				m_callbacks.read_on_pull = reinterpret_cast<mmmojo::common::MMMojoReadOnPull*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMReadPush:
				m_callbacks.read_on_push = reinterpret_cast<mmmojo::common::MMMojoReadOnPush*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMReadShared:
				m_callbacks.read_on_shared = reinterpret_cast<mmmojo::common::MMMojoReadOnShared*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMRemoteConnect:
				m_callbacks.remote_on_connect = reinterpret_cast<mmmojo::common::MMMojoRemoteOnConnect*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMRemoteDisconnect:
				m_callbacks.remote_on_disconnect = reinterpret_cast<mmmojo::common::MMMojoRemoteOnDisConnect*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMRemoteMojoError:
				m_callbacks.remote_on_mojoerror = reinterpret_cast<mmmojo::common::MMMojoRemoteOnMojoError*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMRemoteProcessLaunched:
				m_callbacks.remote_on_processlaunched = reinterpret_cast<mmmojo::common::MMMojoRemoteOnProcessLaunched*>(pfunc);
			break;
		case MMMojoEnvironmentCallbackType::kMMRemoteProcessLaunchFailed:
				m_callbacks.remote_on_processlaunchfailed = reinterpret_cast<mmmojo::common::MMMojoRemoteOnProcessLaunchFailed*>(pfunc);
			break;
		default:
			break;
		}
	}

	void XPluginManager::SetCallbacks(mmmojo::common::MMMojoEnvironmentCallbacks callbacks)
	{
		m_callbacks = callbacks;
	}

	void XPluginManager::SetCallbackUsrData(void* usr_data)
	{
		m_cb_usrdata = usr_data;
	}

	bool XPluginManager::InitMMMojoEnv()
	{
		//未初始化DLL导出函数, 进行加载
		if (g_MMMojoDLL.hMMMojo == NULL)
		{
			if (m_switch_native.count("user-lib-dir") > 0)
			{
				std::string mmmojo_path = m_switch_native["user-lib-dir"];
				if (!InitMMMojoDLLFuncs(mmmojo_path.c_str()))
				{
					m_last_err = utils::string_format("%s: InitMMMojoDLLFuncs Err", __FUNCTION__);
					return false;
				}
			}
			else
			{
				m_last_err = utils::string_format("%s: Can't Find mmmojo(_64).dll Path, Please initialize manually", __FUNCTION__);
				return false;
			}
		}

		if (m_exe_path.empty())
		{
			m_last_err = utils::string_format("%s: Required Path is EMPTY!", __FUNCTION__);
			return false;
		}

		if (m_mmmojo_env_ptr == nullptr)
		{
			//初始化命令行参数以及MojoIPC等环境
			int argc = m_cmdline.size();
			const char** argv = NULL;
			if (argc != 0)
			{
				argv = (const char**)malloc(sizeof(char*) * argc);
				for (size_t i = 0; i < argc; i++)
					argv[i] = m_cmdline.at(i).c_str();
			}

			g_MMMojoDLL.lpInitializeMMMojo(argc, argv);

			//创建MMMojo环境
			m_mmmojo_env_ptr = g_MMMojoDLL.lpCreateMMMojoEnvironment();

			//设置回调函数
			g_MMMojoDLL.lpSetMMMojoEnvironmentCallbacks(m_mmmojo_env_ptr, MMMojoEnvironmentCallbackType::kMMUserData, m_cb_usrdata);
			for (size_t i = MMMojoEnvironmentCallbackType::kMMReadPush; i <= MMMojoEnvironmentCallbackType::kMMRemoteMojoError; i++)
			{
				DWORD_PTR* cb = (DWORD_PTR*)(((DWORD_PTR*)&m_callbacks)[i - 1]);
				g_MMMojoDLL.lpSetMMMojoEnvironmentCallbacks(m_mmmojo_env_ptr, i, cb);
			}

			//设置启动所需参数
			g_MMMojoDLL.lpSetMMMojoEnvironmentInitParams(m_mmmojo_env_ptr, MMMojoEnvironmentInitParamType::kMMHostProcess, TRUE);
			g_MMMojoDLL.lpSetMMMojoEnvironmentInitParams(m_mmmojo_env_ptr, MMMojoEnvironmentInitParamType::kMMExePath, m_exe_path.c_str());

			//设置SwitchNative命令行参数
			for (auto& switch_ : m_switch_native)
			{
				std::string switch_arg = switch_.first;
				std::wstring value_ = utils::Utf8ToUnicode(switch_.second);

				g_MMMojoDLL.lpAppendMMSubProcessSwitchNative(m_mmmojo_env_ptr, switch_arg.c_str(), value_.c_str());
			}
				

			//启动程序
			g_MMMojoDLL.lpStartMMMojoEnvironment(m_mmmojo_env_ptr);

			if (argv != NULL) free(argv);
			m_init_mmmojo_env = true;
		}

		return true;
	}

	void XPluginManager::StopMMMojoEnv()
	{
		if (g_MMMojoDLL.hMMMojo != NULL)
		{
			if (m_mmmojo_env_ptr != nullptr)
			{
				g_MMMojoDLL.lpStopMMMojoEnvironment(m_mmmojo_env_ptr);
				g_MMMojoDLL.lpRemoveMMMojoEnvironment(m_mmmojo_env_ptr);
				m_init_mmmojo_env = false;
				m_mmmojo_env_ptr = nullptr;
			}
		}
	}

	void XPluginManager::SendPbSerializedData(void* pb_data, int data_size, int method, bool sync, uint32_t request_id)
	{
		if (data_size < 0) return;

		void* write_info = g_MMMojoDLL.lpCreateMMMojoWriteInfo(method, sync, request_id);
		if (data_size != 0)
		{
			void* request = g_MMMojoDLL.lpGetMMMojoWriteInfoRequest(write_info, data_size);
			memmove(request, pb_data, data_size);//写入protobuf数据
		}
		g_MMMojoDLL.lpSendMMMojoWriteInfo(m_mmmojo_env_ptr, write_info);
	}

	const char* XPluginManager::GetLastErrStr()
	{
		return m_last_err.c_str();
	}

	void XPluginManager::SetLastErrStr(std::string err_str)
	{
		m_last_err = err_str;
	}

	void DefaultReadOnPush(uint32_t request_id, const void* request_info, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG

		//示例
		/*
		uint32_t pb_size = 0;
		const void* pb_data = GetPbSerializedData(request_info, pb_size);
		|Do Your Task|
		RemoveReadInfo(request_info);
		*/
	}

	void DefaultReadOnPull(uint32_t request_id, const void* request_info, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultReadOnShared(uint32_t request_id, const void* request_info, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultRemoteOnConnect(bool is_connected, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultRemoteOnDisConnect(void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultRemoteOnProcessLaunched(void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultRemoteOnProcessLaunchFailed(int error_code, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

	void DefaultRemoteOnMojoError(const void* errorbuf, int errorsize, void* user_data)
	{
#ifdef _DEBUG
		OutputDebugStringA(__FUNCTION__);
#endif // _DEBUG
	}

}
}

