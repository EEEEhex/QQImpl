#include <iostream>

#include "QQOcr.h"
#include "ocr_protobuf.pb.h"

using namespace qqimpl;

void OnOcrReadPush(const char* pic_path, void* ocr_response_serialize, int serialize_size);

//返回0->不存在 1->是文件夹 2->是文件
int check_path_info(std::string path)
{
    struct _stat64i32 info;
    if (_stat(path.c_str(), &info) != 0) {  // does not exist
        return 0;
    }

    if (info.st_mode & S_IFDIR) // directory
        return 1;
    else
        return 2;
}

HANDLE hEvent = NULL;

int main()
{
    //创建Event用于等待OCR任务返回
    hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
    {
        std::cout << "\033[31m[!] CreateEventA Err: " << GetLastError() << "\033[0m\n";
        return 1;
    }

    std::string ocr_path, usr_lib_path, pic_path;

    std::cout << "\033[34m[*]\033[0m Enter WeChatOCR.exe Path:\n\033[34m[>]\033[0m ";
    std::getline(std::cin, ocr_path);

    std::cout << "\033[34m[*]\033[0m Enter mmmojo(_64).dll Path:\n\033[34m[>]\033[0m ";
    std::getline(std::cin, usr_lib_path);

    std::cout << "\033[34m[*]\033[0m Enter Pic Path to OCR (Default \'.\\test.png\'):\n\033[34m[>]\033[0m ";
    std::getline(std::cin, pic_path);
    if (pic_path.empty()) pic_path = ".\\test.png";

    if (check_path_info(ocr_path) != 2)
    {
        std::cout << "\033[31m[!] OCR Exe Path is not exist!\033[0m\n";
        return 0;
    }

    if (check_path_info(usr_lib_path) != 1)
    {
        std::cout << "\033[31m[!] OCR Usr Lib Path is not exist!\033[0m\n";
        return 0;
    }

    //初始化OCR Manager
    qqocr::InitManager();
    std::cout << "[+] InitOcrManager OK!\n";

    //设置WeChatOCR.exe所在目录
    if (!qqocr::SetOcrExePath(ocr_path.c_str()))
    {
        std::cout << "\033[31m[!] SetOcrExePath Err: " << qqocr::GetLastErrStr() << "\033[0m\n";
        return 1;
    }
    std::cout << "[+] SetOcrExePath OK!\n";

    //设置mmmojo_64.dll所在目录
    if (!qqocr::SetOcrUsrLibPath(usr_lib_path.c_str()))
    {
        std::cout << "\033[31m[!] SetOcrUsrLibPath Err: " << qqocr::GetLastErrStr() << "\033[0m\n";
        return 1;
    }
    std::cout << "[+] SetOcrUsrLibPath OK!\n";

    //设置接收OCR结果的回调函数
    qqocr::SetUsrReadPushCallback(OnOcrReadPush);
    std::cout << "[+] SetOcrUsrReadPushCallback OK!\n";


    //发送OCR任务
    std::cout << "\033[34m[>]\033[0m Press any key to Send OCR Task:";
    getchar();
    qqocr::DoOCRTask(pic_path.c_str());

    //等待OnOcrReadPush返回
    WaitForSingleObject(hEvent, INFINITE);
        
    std::cout << "\033[34m[>]\033[0m Press any key to Shutdown OCR Env:";
    getchar();
    qqocr::UnInitManager();

    std::cout << "[-] Shutdown WeChatOCR Env!\n";
    CloseHandle(hEvent);
    return 0;
}

void OnOcrReadPush(const char* pic_path, void* ocr_response_serialize, int serialize_size)
{
    ocr_protobuf::OcrResponse ocr_response;
    ocr_response.ParseFromArray(ocr_response_serialize, serialize_size);

    char out_buf[512] = { 0 };
    sprintf(out_buf, "[*] OnReadPush type:%d task id:%d errCode:%d\n", ocr_response.type(), ocr_response.task_id(), ocr_response.err_code());
    std::cout << out_buf;

    sprintf(out_buf, "[*] OnReadPush TaskId:%d -> PicPath:%s\n", ocr_response.task_id(), pic_path);
    std::cout << out_buf;
    if (ocr_response.type() == 0)
    {
        std::cout << "[*] OcrResult:\n[\n";
        for (int i = 0; i < ocr_response.ocr_result().single_result_size(); i++)
        {
            ocr_protobuf::OcrResponse::OcrResult::SingleResult single_result = ocr_response.ocr_result().single_result(i);
            sprintf(out_buf, "\tRECT:[lx:%f, ly:%f, rx:%f, ry:%f]\n", single_result.lx(), single_result.ly(), single_result.rx(), single_result.ry());
            std::cout << out_buf;
            std::string utf8str = single_result.single_str_utf8();
            printf("\tUTF8STR:[");
            for (size_t j = 0; j < utf8str.size(); j++)
            {
                printf("%02X ", (BYTE)utf8str[j]);
            }
            std::cout << " (" << utf8str << ")]\n";
        }
        puts("]");

        if (hEvent != NULL) SetEvent(hEvent);//让main函数继续运行
    }
}