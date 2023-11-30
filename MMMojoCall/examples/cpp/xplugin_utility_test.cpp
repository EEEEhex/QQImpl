/*****************************************************************//**
 * @file   xplugin_test.cpp
 * @brief  此文件代码是UtilityManager类调用WeChatUtility扫描二维码的示例
 *********************************************************************/

#include <Windows.h>
#include <iostream>

#include "mmmojo_utility.h"

using namespace qqimpl;

void UtilityUsrReadOnPush(int type, const void* data, int data_size)
{
	std::cout << "[↓] " << __FUNCTION__ << " [RequestID: " << type << "] :\n[\n";

	switch (type)
	{
	case mmmojocall::RequestIdUtility::UtilityTextScanPushResp:
	{
#ifdef EXAMPLE_USE_JSON
		std::cout << (char*)data << std::endl;
#else
		utility_protobuf::TextScanMessage text_scan_msg;
		text_scan_msg.ParseFromArray(data, data_size);
		
		char out_buf[512] = { 0 };
		sprintf_s(out_buf, sizeof(out_buf), "\t[*] TextScanResult: ID: [%d] | PicPath: [%s] | HaveText?: [%d] | UKN0: [%d] | Rate: [%f]\n", 
				text_scan_msg.text_scan_id(), text_scan_msg.pic_path().c_str(), text_scan_msg.have_text(), text_scan_msg.unknown_0(), text_scan_msg.rate());
		std::cout << out_buf;
#endif // EXAMPLE_USE_JSON
	}
		break;
	default:
		break;
	}

	std::cout << "\n]" << std::endl;
}

void UtilityUsrReadOnPull(int type, const void* data, int data_size)
{
	std::cout << "[↓] " << __FUNCTION__ << " [RequestID: " << type << "] :\n[\n";

	switch (type)
	{
	case mmmojocall::RequestIdUtility::UtilityQRScanPullResp:
	{
#ifdef EXAMPLE_USE_JSON
		std::cout << (char*)data << std::endl;
#else
		utility_protobuf::QRScanRespMessage qrscan_response;
		qrscan_response.ParseFromArray(data, data_size);

		std::cout << "\t[*] QRScanResult UKN0: [" << qrscan_response.unknown_0() << "] :\n\t[\n";
		for (int i = 0; i < qrscan_response.qr_result_size(); i++)
		{
			utility_protobuf::QRScanRespMessage::QRScanResult qrscan_result = qrscan_response.qr_result(i);
			std::string result = qrscan_result.result();
			uint32_t unknown_0 = qrscan_result.unknow_0();
			double unknown_1 = qrscan_result.unknown_1();
			double unknown_2 = qrscan_result.unknown_2();
			uint32_t unknown_3 = qrscan_result.unknown_3();

			char out_buf[512] = { 0 };
			sprintf_s(out_buf, sizeof(out_buf), "\t\tResult: [%s] | UKN0: [%d] | UKN1: [%f] | UKN2: [%f] |  UNK3: [%d]\n",
				result.c_str(), unknown_0, unknown_1, unknown_2, unknown_3);
			std::cout << out_buf;
		}
		std::cout << "\t]";
#endif
	}
		break;
	default:
		break;
	}

	std::cout << "\n]" << std::endl;
}

int main()
{	
	SetConsoleOutputCP(CP_UTF8);
	
	std::string wechat_utility_dir;//"C:\\Users\\{YourName}\\AppData\\Roaming\\Tencent\\WeChat\\XPlugin\\Plugins\\WeChatUtility\\8077\\extracted\\WeChatUtility.exe"
	std::string wechat_dir;//Such as "D:\\WeChat\\[3.9.7.29]"
	
	std::cout << "\033[34m[=] Enter WeChatUtility.exe Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_utility_dir);

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


	//UtilityManager类调用WeChatUtility扫描二维码的示例
	mmmojocall::UtilityManager utility_manager;
	if (!utility_manager.SetExePath(wechat_utility_dir.c_str()))
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

	if (!utility_manager.SetUsrLibDir(wechat_dir.c_str()))
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

#ifdef EXAMPLE_USE_JSON
	utility_manager.SetCallbackDataMode(true);//回调函数的参数传入json
#endif //EXAMPLE_USE_JSON

	//设置回调函数, Push用于接收TextScan的结果
	utility_manager.SetReadOnPull(UtilityUsrReadOnPull);
	utility_manager.SetReadOnPush(UtilityUsrReadOnPush);

	//启动WeChatUtility.exe
	if (!utility_manager.StartWeChatUtility())
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}
	std::cout << "\033[31m[+] StartWeChatUtility OK!\033[0m\n";

	//发起QRScan请求, 第二个参数为TextScan的任务ID
	std::cout << "[=] Press any key to send QRScan request...\n"; getchar();
	if (!utility_manager.DoPicQRScan(".\\test.png", 0xEE))
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
	}

	//结束WeChatUtility.exe
	std::cout << "[=] Press any key to stop WeChatUtility Env...\n"; getchar();
	utility_manager.KillWeChatUtility();
	std::cout << "\033[31m[-] KillWeChatUtility\033[0m\n";


	mmmojocall::ShutdownMMMojoGlobal();
	return 0;
}
