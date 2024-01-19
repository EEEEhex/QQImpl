#pragma once
#include "pch.h"
#include <iostream>
#include "mmmojo_ocr.h"

namespace MMMojoCallWrapper
{
	typedef void (*LPFN_OCRREADONPUSHW)(const char* pic_path, const void* data, int data_size);

	extern "C" __declspec(dllexport) bool __stdcall InitMMMojoDLLFuncsW(const char* mmmojo_dll_path);
	
	extern "C" __declspec(dllexport) bool __stdcall InitMMMojoGlobalW(int argc, const char* const* argv);

	extern "C" __declspec(dllexport) bool __stdcall SetExePathW(const char* exe_path);

	extern "C" __declspec(dllexport) bool __stdcall SetUsrLibDirW(const char* usr_lib_dir);

	extern "C" __declspec(dllexport) void __stdcall SetCallbackDataMode(bool use_json);

	extern "C" __declspec(dllexport) void __stdcall SetReadOnPushW(LPFN_OCRREADONPUSHW pfunc);

	extern "C" __declspec(dllexport) bool __stdcall StartWeChatOCRW();

	extern "C" __declspec(dllexport) bool __stdcall DoOCRTaskW(const char* pic_path);

	extern "C" __declspec(dllexport) void __stdcall KillWeChatOCRW();

	extern "C" __declspec(dllexport) bool __stdcall ShutdownMMMojoGlobalW();

	extern "C" __declspec(dllexport) const char* __stdcall GetLastErrStrW();
}
