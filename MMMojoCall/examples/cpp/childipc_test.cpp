#include <iostream>

#include "qq_ipc.h"

using namespace qqimpl;

void __stdcall OnChildReceiveMsg32(void* pArg, char* msg, int arg3, int arg4, char* addition_msg, int addition_msg_size)
{
    //std::cout << "[ " << __FUNCTION__ << "] | Arg: 0x" << std::hex << pArg << std::endl;
    std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
    std::cout << "[*] [C] [ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
    std::cout << "[*] [C] [ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
    if (addition_msg_size != 0)
    {
        std::cout << "[*] [C] [ " << __FUNCTION__ << "] | The Addition Msg: ";
        for (size_t i = 0; i < addition_msg_size; i++)
            printf_s("0x%02X ", (byte)(addition_msg[i]));
        printf_s("(%s)", addition_msg);
        puts("");
    }
}

void OnChildReceiveMsg64(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size)
{
    //std::cout << "[ " << __FUNCTION__ << "] | Arg: 0x" << std::hex << pArg << std::endl;

    std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
    std::cout << "[*] [C] [ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
    std::cout << "[*] [C] [ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
    if (addition_msg_size != 0)
    {
        std::cout << "[*] [C] [ " << __FUNCTION__ << "] | The Addition Msg: ";
        for (size_t i = 0; i < addition_msg_size; i++)
            printf_s("0x%02X ", (byte)(addition_msg[i]));
        printf_s("(%s)", addition_msg);
        puts("");
    }

    //qqimpl::QQIpcParentWrapper::OnDefaultReceiveMsg(pArg, msg, arg3, addition_msg, addition_msg_size);
}

int main()
{
    std::cout << "[+] [C] Child Begin!\n";

    qqipc::QQIpcChildWrapper child_ipc;

    if (!child_ipc.InitEnv())
    {
        std::cout << "[!] [C] Init Child Err: " << child_ipc.GetLastErrStr() << std::endl;
        getchar(); return 0;
    }

    child_ipc.InitLog();
#ifdef _WIN64
    child_ipc.SetChildReceiveCallback(OnChildReceiveMsg64);
#else
    child_ipc.SetChildReceiveCallback(OnChildReceiveMsg32);
#endif // _WIN64
    child_ipc.InitChildIpc();
    
    std::cout << "[=] [C] Press any key to SendIpcMessage to Parent: \n";
    getchar();

    std::string addtion_msg = "addition_test_child";
    child_ipc.SendIpcMessage("child_test", addtion_msg.data(), addtion_msg.size());

    MSG msg;
    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}


