#include <Windows.h>

#include "mojo_call_export.h"
#include "mmmojo_call.h"

#ifdef USE_WRAPPER
#include "mmmojo_ocr.h"
#include "mmmojo_utility.h"
#include "mmmojo_player.h"
#endif

#ifdef PURE_C_MODE
using namespace qqimpl;

const void* GetInstanceXPluginMgr(int mgr_type)
{
	void* ret_ptr = NULL;
	switch (mgr_type)
	{
	case 0:
		ret_ptr = new mmmojocall::XPluginManager();
		break;
#ifdef USE_WRAPPER
	case 1:
		ret_ptr = new mmmojocall::OCRManager();
		break;
	case 2:
		ret_ptr = new mmmojocall::UtilityManager();
		break;
	case 3:
		ret_ptr = new mmmojocall::PlayerManager();
		break;
#endif
	default:
		break;
	}

	return ret_ptr;
}

int CallFuncXPluginMgr(const void* class_ptr, int mgr_type, const char* func_name, void* ret_ptr, ...)
{
	if ((class_ptr == NULL) || (func_name == NULL) || (strlen(func_name) == 0))
		return FALSE;

	va_list p_args;
	va_start(p_args, ret_ptr);

	mmmojocall::XPluginManager* mgr_ptr = (mmmojocall::XPluginManager*)class_ptr;

	std::string func_name_string = func_name;
	if (func_name_string == "SetExePath")
	{
		const char* arg_exe_path = va_arg(p_args, const char*);
		bool bRet = mgr_ptr->SetExePath(arg_exe_path);
		if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
	}
	else if (func_name_string == "AppendSwitchNativeCmdLine")
	{
		const char* arg_switch_string = va_arg(p_args, const char*);
		const char* arg_value = va_arg(p_args, const char*);
		bool bRet = mgr_ptr->AppendSwitchNativeCmdLine(arg_switch_string, arg_value);
		if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
	}
	else if (func_name_string == "SetOneCallback")
	{
		int arg_type = va_arg(p_args, int);
		void* arg_pfunc = va_arg(p_args, void*);
		mgr_ptr->SetOneCallback(arg_type, arg_pfunc);
	}
	else if (func_name_string == "SetCallbacks")
	{
		void* arg_mmmjocallbacks_ptr = va_arg(p_args, void*);
		mmmojo::common::MMMojoEnvironmentCallbacks cbs = *((mmmojo::common::MMMojoEnvironmentCallbacks*)arg_mmmjocallbacks_ptr);
		mgr_ptr->SetCallbacks(cbs);
	}
	else if (func_name_string == "SetCallbackUsrData")
	{
		void* arg_usr_data = va_arg(p_args, void*);
		mgr_ptr->SetCallbackUsrData(arg_usr_data);
	}
	else if (func_name_string == "InitMMMojoEnv")
	{
		bool bRet = mgr_ptr->StartMMMojoEnv();
		if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
	}
	else if (func_name_string == "StopMMMojoEnv")
	{
		mgr_ptr->StopMMMojoEnv();
	}
	else if (func_name_string == "SendPbSerializedData")
	{
		void* arg_pb_data = va_arg(p_args, void*);
		int arg_data_size = va_arg(p_args, int);
		int arg_method = va_arg(p_args, int);
		bool arg_sync = va_arg(p_args, int);
		uint32_t arg_request_id = va_arg(p_args, uint32_t);
		mgr_ptr->SendPbSerializedData(arg_pb_data, arg_data_size, arg_method, arg_sync, arg_request_id);
	}
	else if (func_name_string == "GetLastErrStr")
	{
		const char* pszRet = mgr_ptr->GetLastErrStr();
		if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = (DWORD_PTR)pszRet;
	}

#ifdef USE_WRAPPER
	if (mgr_type == MGRTYPE::OCRManager)//OCRManager
	{
		mmmojocall::OCRManager* ocr_mgr_ptr = (mmmojocall::OCRManager*)class_ptr;
		if (func_name_string == "SetUsrLibDir")
		{
			const char* arg_usrlib_dir = va_arg(p_args, const char*);
			bool bRet = ocr_mgr_ptr->SetUsrLibDir(arg_usrlib_dir);
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "SetReadOnPush")
		{
			void* arg_callback = va_arg(p_args, void*);
			ocr_mgr_ptr->SetReadOnPush((mmmojocall::OCRManager::LPFN_OCRREADONPUSH)arg_callback);
		}
		else if (func_name_string == "StartWeChatOCR")
		{
			bool bRet = ocr_mgr_ptr->StartWeChatOCR();
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "KillWeChatOCR")
		{
			ocr_mgr_ptr->KillWeChatOCR();
		}
		else if (func_name_string == "DoOCRTask")
		{
			const char* arg_pic_path = va_arg(p_args, const char*);
			bool bRet = ocr_mgr_ptr->DoOCRTask(arg_pic_path);
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "GetConnectState")
		{
			bool bRet = ocr_mgr_ptr->GetConnectState();
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "SetCallbackDataMode")
		{
			bool arg_use_json = va_arg(p_args, int);
			ocr_mgr_ptr->SetCallbackDataMode(arg_use_json);
		}
	}
	else if (mgr_type == MGRTYPE::UtilityManager)
	{
		mmmojocall::UtilityManager* utility_mgr_ptr = (mmmojocall::UtilityManager*)class_ptr;
		if (func_name_string == "SetUsrLibDir")
		{
			const char* arg_usrlib_dir = va_arg(p_args, const char*);
			bool bRet = utility_mgr_ptr->SetUsrLibDir(arg_usrlib_dir);
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "SetReadOnPull")
		{
			void* arg_callback = va_arg(p_args, void*);
			utility_mgr_ptr->SetReadOnPull((mmmojocall::UtilityManager::LPFN_UTILITYREADONPUSHLL)arg_callback);
		}
		else if (func_name_string == "SetReadOnPush")
		{
			void* arg_callback = va_arg(p_args, void*);
			utility_mgr_ptr->SetReadOnPush((mmmojocall::UtilityManager::LPFN_UTILITYREADONPUSHLL)arg_callback);
		}
		else if (func_name_string == "StartWeChatUtility")
		{
			bool bRet = utility_mgr_ptr->StartWeChatUtility();
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "KillWeChatUtility")
		{
			utility_mgr_ptr->KillWeChatUtility();
		}
		else if (func_name_string == "DoPicQRScan")
		{
			const char* arg_pic_path = va_arg(p_args, const char*);
			int arg_text_scan_id = va_arg(p_args, int);
			bool bRet = utility_mgr_ptr->DoPicQRScan(arg_pic_path, arg_text_scan_id);
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "GetConnectState")
		{
			bool bRet = utility_mgr_ptr->GetConnectState();
			if (ret_ptr != NULL) *((DWORD_PTR*)ret_ptr) = bRet;
		}
		else if (func_name_string == "SetCallbackDataMode")
		{
			bool arg_use_json = va_arg(p_args, int);
			utility_mgr_ptr->SetCallbackDataMode(arg_use_json);
		}
	}
	else if (mgr_type == MGRTYPE::PlayerManager)
	{
		//TODO: 看看有没有办法可以反射调用, 这样写太麻烦了
	}
#endif

	va_end(p_args);
	return TRUE;
}

void ReleaseInstanceXPluginMgr(const void* mgr_ptr)
{
	if (mgr_ptr != NULL)
	{
		delete mgr_ptr;
	}
}
#endif // PURE_C_MODE