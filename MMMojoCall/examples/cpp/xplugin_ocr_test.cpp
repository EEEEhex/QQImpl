/*****************************************************************//**
 * @file   xplugin_test.cpp
 * @brief  此文件代码是使用OCRManager类调用WeChatOCR.exe的示例
 *********************************************************************/

#include <iostream>

#include "mmmojo_ocr.h"

using namespace qqimpl;

void OCRUsrReadOnPush(const char* pic_path, const void* data, int data_size)
{
	std::cout << "[↓] " << __FUNCTION__ << ":\n[\n";

#ifdef EXAMPLE_USE_JSON
	//此时参数data为json字符串
	std::cout << (char*)data << std::endl;
#else
	ocr_protobuf::OcrResponse ocr_response;
	ocr_response.ParseFromArray(data, data_size);

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
#endif
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

	//------------------------
	//以下为使用封装好的OCRManager类调用WeChatOCR进行OCR的示例
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

#ifdef EXAMPLE_USE_JSON
	ocr_manager.SetCallbackDataMode(true);//回调函数的参数传入json
#endif //EXAMPLE_USE_JSON
	ocr_manager.SetReadOnPush(OCRUsrReadOnPush);

	//启动WeChatOCR.exe
	if (!ocr_manager.StartWeChatOCR())
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}
	std::cout << "\033[31m[+] StartWeChatOCR OK!\033[0m\n";

	//向其发送OCR请求
	std::cout << "[=] Press any key to send ocr request...\n"; getchar();
	if (!ocr_manager.DoOCRTask(".\\test.png"))
	{
		std::cout << "\033[31m[!] " << ocr_manager.GetLastErrStr() << "\033[0m\n";
		return 1;
	}

	//结束WeChatOCR.exe
	std::cout << "[=] Press any key to stop WeChatOCR Env...\n"; getchar();
	ocr_manager.KillWeChatOCR();
	std::cout << "\033[31m[-] KillWeChatOCR!\033[0m\n";

	
	mmmojocall::ShutdownMMMojoGlobal();
	return 0;
}
