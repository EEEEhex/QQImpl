#include <iostream>

#include "mmmojo_call.h"

#ifdef USE_WRAPPER
#include "mmmojo_ocr.h"
#include "mmmojo_utility.h"
#endif

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

//↓mmmojo::common::MMMojoReadOnPull;
void UsrReadOnPull(uint32_t request_id, const void* request_info, void* user_data)
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

#ifdef USE_WRAPPER
void OCRUsrReadOnPush(const char* pic_path, const void* pb_data, int pb_size)
{
	std::cout << "[↓] " << __FUNCTION__ << ":\n[\n";

	ocr_protobuf::OcrResponse ocr_response;
	ocr_response.ParseFromArray(pb_data, pb_size);

	char out_buf[512] = { 0 };
	sprintf_s(out_buf, sizeof(out_buf), "\t[*] type: %d task id: %d errCode: %d\n", ocr_response.type(), ocr_response.task_id(), ocr_response.err_code());
	std::cout << out_buf;

	if (pic_path != nullptr)
	{
		sprintf_s(out_buf, sizeof(out_buf), "\t[*] TaskId: %d -> PicPath: %s\n", ocr_response.task_id(), pic_path);
		std::cout << out_buf;
	}
	else
	{
		std::cout << "\t[*] This is a type 1 Push that is called back only once at startup of WeChatOCR.exe";
	}
	
	if (ocr_response.type() == 0)
	{
		std::cout << "\t[*] OcrResult:\n\t[\n";
		for (int i = 0; i < ocr_response.ocr_result().single_result_size(); i++)
		{
			ocr_protobuf::OcrResponse::OcrResult::SingleResult single_result = ocr_response.ocr_result().single_result(i);

			sprintf_s(out_buf, sizeof(out_buf), "\t\tRECT:[left: %f, top: %f, right: %f, bottom: %f]\n", single_result.left(), single_result.top(), single_result.right(), single_result.bottom());
			std::cout << out_buf;
			std::string utf8str = single_result.single_str_utf8();
			std::cout << "\t\tUTF8STR:[";
			for (size_t j = 0; j < utf8str.size(); j++)
			{
				printf_s("0x%2X ", (BYTE)utf8str[j]);
			}
			std::cout << " (" << utf8str << ")]\n";
		}
		std::cout << "\t]";
	}

	std::cout << "\n]" << std::endl;
}

void UtilityUsrReadOnPush(int type, const void* serialized_data, int data_size)
{
	std::cout << "[↓] " << __FUNCTION__ << " [RequestID: " << type << "] :\n[\n";

	switch (type)
	{
	case mmmojocall::RequestIdUtility::UtilityTextScanPushResp:
	{
		utility_protobuf::TextScanMessage text_scan_msg;
		text_scan_msg.ParseFromArray(serialized_data, data_size);
		
		char out_buf[512] = { 0 };
		sprintf_s(out_buf, sizeof(out_buf), "\t[*] TextScanResult: ID: [%d] | PicPath: [%s] | HaveText?: [%d] | UKN0: [%d] | Rate: [%f]\n", 
				text_scan_msg.text_scan_id(), text_scan_msg.pic_path().c_str(), text_scan_msg.have_text(), text_scan_msg.unknown_0(), text_scan_msg.rate());
		std::cout << out_buf;
	}
		break;
	default:
		break;
	}

	std::cout << "\n]" << std::endl;
}

void UtilityUsrReadOnPull(int type, const void* serialized_data, int data_size)
{
	std::cout << "[↓] " << __FUNCTION__ << " [RequestID: " << type << "] :\n[\n";

	switch (type)
	{
	case mmmojocall::RequestIdUtility::UtilityInitPullResp:
	{
		std::cout << "\t[*] UtilityInitPullResp";
	}
		break;
	case mmmojocall::RequestIdUtility::UtilityQRScanPullResp:
	{
		utility_protobuf::QRScanRespMessage qrscan_response;
		qrscan_response.ParseFromArray(serialized_data, data_size);

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
	}
		break;
	default:
		break;
	}

	std::cout << "\n]" << std::endl;
}
#endif

int main()
{	
	SetConsoleOutputCP(CP_UTF8);
	
	std::string wechat_utility_dir;//"C:\\Users\\{YourName}\\AppData\\Roaming\\Tencent\\WeChat\\XPlugin\\Plugins\\WeChatUtility\\8077\\extracted\\WeChatUtility.exe"
	std::string wechat_ocr_dir;//"C:\\Users\\{YourName}\\AppData\\Roaming\\Tencent\\WeChat\\XPlugin\\Plugins\\WeChatOCR\\7057\\extracted\\WeChatOCR.exe"
	std::string wechat_dir;//Such as "D:\\WeChat\\[3.9.7.29]"
	
	std::cout << "\033[34m[=] Enter WeChatOCR.exe Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_ocr_dir);

    std::cout << "\033[34m[=] Enter mmmojo(_64).dll Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_dir);

    std::cout << "\033[34m[=] Enter WeChatUtility.exe Path:\n[>]\033[0m ";
    std::getline(std::cin, wechat_utility_dir);

    //[可选] 加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.	
	//	若要启动的组件未设置"user-lib-dir"参数, 则需手动调用此函数, 
	//	但若要启动的组件设置了user-lib-dir这个参数, 则在InitMMMojoEnv时会自动调用此函数.
	if (!mmmojocall::InitMMMojoDLLFuncs(wechat_dir.c_str()))
	{
		std::cout << "\033[31m[!] mmmojocall::InitMMMojoDLLFuncs ERR!\033[0m\n";
        return 1;
	}

	//以下为直接使用XPluginManager类调用微信XPlugin组件(WeChatOCR)的示例
	mmmojocall::XPluginManager xplugin_manager;
	if (!xplugin_manager.SetExePath(wechat_ocr_dir.c_str()))//设置XPlugin组件路径
	{
		std::cout << "\033[31m[!] " << xplugin_manager.GetLastErrStr() << "\033[0m\n";
        return 1;
	}
	xplugin_manager.AppendSwitchNativeCmdLine("user-lib-dir", wechat_dir.c_str());//添加需要的Switch命令行
	xplugin_manager.SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPush, UsrReadOnPush);//设置ReadPush回调函数
	xplugin_manager.SetOneCallback(MMMojoEnvironmentCallbackType::kMMReadPull, UsrReadOnPull);//设置ReadPull回调函数
	if (!xplugin_manager.InitMMMojoEnv())//启动环境和组件
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

#ifdef USE_WRAPPER
	//------------------------
	//以下为使用封装好的OCRManager类调用WeChatOCR进行OCR的示例 与 UtilityManager类调用WeChatUtility扫描二维码的示例
	mmmojocall::OCRManager ocr_manager;
	if (!ocr_manager.SetExePath(wechat_ocr_dir.c_str()))
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

	if (!ocr_manager.SetUsrLibDir(wechat_dir.c_str()))
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

	ocr_manager.SetReadOnPush(OCRUsrReadOnPush);
	if (!ocr_manager.StartWeChatOCR())
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}
	std::cout << "\033[31m[+] StartWeChatOCR OK!\033[0m\n";

	std::cout << "[=] Press any key to send ocr request...\n"; getchar();
	if (!ocr_manager.DoOCRTask(".\\test.png"))
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

	std::cout << "[=] Press any key to stop WeChatOCR Env...\n"; getchar();
	ocr_manager.KillWeChatOCR();
	std::cout << "\033[31m[-] KillWeChatOCR!\033[0m\n";

	//QRScan
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

	utility_manager.SetReadOnPull(UtilityUsrReadOnPull);
	utility_manager.SetReadOnPush(UtilityUsrReadOnPush);
	if (!utility_manager.StartWeChatUtility())
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}
	std::cout << "\033[31m[+] StartWeChatUtility OK!\033[0m\n";

	std::cout << "[=] Press any key to send QRScan request...\n"; getchar();
	if (!utility_manager.DoPicQRScan(".\\test.png", 0xEE))
	{
		std::cout << "\033[31m[!] " << utility_manager.GetLastErrStr() << "\033[0m\n";
	}

	std::cout << "[=] Press any key to stop WeChatUtility Env...\n"; getchar();
	utility_manager.KillWeChatUtility();
	std::cout << "\033[31m[-] KillWeChatUtility\033[0m\n";
#endif

	return 0;
}
