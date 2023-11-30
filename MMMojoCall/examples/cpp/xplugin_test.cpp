/*****************************************************************//**
 * @file   xplugin_test.cpp
 * @brief  此文件代码是直接使用XPluginManager类调用微信XPlugin组件(WeChatOCR)的示例
 *********************************************************************/

#include <Windows.h>
#include <iostream>

#include "mmmojo_call.h"

using namespace qqimpl;

//↓mmmojo::common::MMMojoReadOnPush;
void UsrReadOnPush(uint32_t request_id, const void* request_info, void* user_data)
{
	std::cout << "[↓] " << __FUNCTION__ << " [RequestID: " << request_id << "] :\n[\n";

	uint32_t pb_size;
	const void* pb_data = mmmojocall::GetPbSerializedData(request_info, pb_size);

	std::cout << "\t[*] Request ID: " << request_id << " | Protobuf Size: " << pb_size << std::endl;
	std::cout << "\t[*] Protobuf Data: ";
	for (size_t i = 0; i < pb_size; i++)
	{
		printf_s("0x%02X ", ((LPBYTE)pb_data)[i]);
	}
	
	mmmojocall::RemoveReadInfo(request_info);

	std::cout << "\n]" << std::endl;
}

int main()
{	
	SetConsoleOutputCP(CP_UTF8);
	
	std::string wechat_ocr_dir;//"C:\\Users\\{YourName}\\AppData\\Roaming\\Tencent\\WeChat\\XPlugin\\Plugins\\WeChatOCR\\7057\\extracted\\WeChatOCR.exe"
	std::string wechat_dir;//Such as "D:\\WeChat\\[3.9.7.29]"
	
	std::cout << "\033[34m[=] Enter WeChatOCR.exe Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_ocr_dir);

    std::cout << "\033[34m[=] Enter mmmojo(_64).dll Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_dir);

    //	加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.	
	if (!mmmojocall::InitMMMojoDLLFuncs(wechat_dir.c_str()))
	{
		std::cout << "\033[31m[!] mmmojocall::InitMMMojoDLLFuncs ERR!\033[0m\n";
        return 1;
	}

	//	初始化MMMojo (包括ThreadPool等), 只需要调用一次.	
	mmmojocall::InitMMMojoGlobal(NULL, NULL);

	//-------------------------------
	//以下为直接使用XPluginManager类调用微信XPlugin组件(WeChatOCR)的示例
	mmmojocall::XPluginManager xplugin_manager;
	if (!xplugin_manager.SetExePath(wechat_ocr_dir.c_str()))//设置XPlugin组件路径
	{
		std::cout << "\033[31m[!] " << xplugin_manager.GetLastErrStr() << "\033[0m\n";
        return 1;
	}
	xplugin_manager.AppendSwitchNativeCmdLine("user-lib-dir", wechat_dir.c_str());//添加需要的Switch命令行
	xplugin_manager.SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPush, UsrReadOnPush);//设置ReadPush回调函数
	if (!xplugin_manager.StartMMMojoEnv())//启动环境和组件
	{
		std::cout << "\033[31m[!] " << xplugin_manager.GetLastErrStr() << "\033[0m\n";
        return 1;
	}
	std::cout << "\033[31m[+] InitMMMojoEnv OK!\033[0m\n";
	std::cout << "\033[31m[*] Will Use \'.\\test.png\' for test\033[0m\n";

	//以下为图片路径为:".\test.png"的序列化数据
	unsigned char pb_hex_data[16] = {
		0x10, 0x01, 0x1A, 0x0C, 0x0A, 0x0A, 0x2E, 0x5C, 0x74, 0x65, 0x73, 0x74, 0x2E, 0x70, 0x6E, 0x67
	};

	std::cout << "[=] Press any key to send request...\n"; getchar();//发送请求
	xplugin_manager.SendPbSerializedData(pb_hex_data, sizeof(pb_hex_data), MMMojoInfoMethod::kMMPush, false, mmmojocall::RequestIdOCR::OCRPush);

	std::cout << "[=] Press any key to stop xplugin mmmojo env...\n"; getchar();//停止环境
	xplugin_manager.StopMMMojoEnv();
	std::cout << "\033[31m[-] StopMMMojoEnv...\033[0m\n";


	mmmojocall::ShutdownMMMojoGlobal();
	return 0;
}
