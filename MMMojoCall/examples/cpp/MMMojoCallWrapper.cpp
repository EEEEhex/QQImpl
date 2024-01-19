#include "pch.h"
#include "MMMojoCallWrapper.h"
#include <iostream>
using namespace qqimpl;

namespace MMMojoCallWrapper
{
	mmmojocall::OCRManager ocr_manager;

	bool __stdcall SetExePathW(const char* exe_path)
	{
		return ocr_manager.SetExePath(exe_path);
	}

	
	bool __stdcall SetUsrLibDirW(const char* usr_lib_dir)
	{
		return ocr_manager.SetUsrLibDir(usr_lib_dir);
	}

	bool __stdcall InitMMMojoDLLFuncsW(const char* mmmojo_dll_path)
	{
		std::cout << mmmojo_dll_path << std::endl;
		return mmmojocall::InitMMMojoDLLFuncs(mmmojo_dll_path);
	}

	bool __stdcall InitMMMojoGlobalW(int argc, const char* const* argv)
	{
		return mmmojocall::InitMMMojoGlobal(argc, argv);
	}

	void __stdcall SetCallbackDataMode(bool use_json)
	{
		return ocr_manager.SetCallbackDataMode(use_json);
	}

	void __stdcall SetReadOnPushW(LPFN_OCRREADONPUSHW pfunc)
	{
		return ocr_manager.SetReadOnPush(pfunc);
	}

	bool __stdcall StartWeChatOCRW()
	{
		return ocr_manager.StartWeChatOCR();
	}

	bool __stdcall DoOCRTaskW(const char* pic_path)
	{
		return ocr_manager.DoOCRTask(pic_path);
	}

	/*
	*/
	void __stdcall KillWeChatOCRW()
	{
		return ocr_manager.KillWeChatOCR();
	}

	bool __stdcall ShutdownMMMojoGlobalW()
	{
		return mmmojocall::ShutdownMMMojoGlobal();
	}

	const char* __stdcall GetLastErrStrW()
	{
		return ocr_manager.GetLastErrStr();
	}
}