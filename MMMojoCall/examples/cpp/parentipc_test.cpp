#include <iostream>
#include <string>

#include "qq_ipc.h"

using namespace qqimpl;

void __stdcall OnParentReceiveMsg32(void*, char*, int, int, char*, int);
void  OnParentReceiveMsg64(void*, char*, int, char*, int);

int main()
{
    std::cout << "[+] [P] Parent Ipc Begin!\n";
    int child_pid_1 = 0, child_pid_2 = 0;

    qqipc::QQIpcParentWrapper parent_ipc;
    //初始化环境
    if (!parent_ipc.InitEnv())
    {
        std::cout << "[!] [P] Init ParentIpc Err: " << parent_ipc.GetLastErrStr() << std::endl;
        getchar(); return 0;
    }
    parent_ipc.InitLog(5);
    parent_ipc.InitParentIpc();

    qqipc::callback_ipc cb_ptr;
#ifdef _WIN64
    cb_ptr = OnParentReceiveMsg64;
#else
    cb_ptr = OnParentReceiveMsg32;
#endif // _WIN64

    const char* child_path;
#ifdef _DEBUG
    child_path = "qqipc_child_test_d.exe";
#else
    child_path = "qqipc_child_test.exe";
#endif // _WIN64


    const char* cmd_args[] = {"-test1", "-test2"};//传递命令行参数
    child_pid_1 = parent_ipc.LaunchChildProcess(child_path, cb_ptr, (void*)"Child_1", (char**)cmd_args, 2);// "Child_1"为传递给OnUsrReceiveMsg的参数
    if (child_pid_1 == 0)
    {
        std::cout << "[!] [P] Launch CihldIpc.exe Err: " << parent_ipc.GetLastErrStr() << std::endl;
        getchar(); return 0;
    }

    std::cout << "[+] [P] Launch OK! Ready to Connect!\n";
    parent_ipc.ConnectedToChildProcess(child_pid_1);

    std::cout << "[=] [P] Press any key to SendIpcMessage to Child1: \n";
    getchar();
    std::string addtion_msg = "addition_parent";
    parent_ipc.SendIpcMessage(child_pid_1, "test_parent", addtion_msg.data(), addtion_msg.size());


    std::cout << "[=] [P] Press any key to Terminate Child1: \n";
    getchar();
    parent_ipc.TerminateChildProcess(child_pid_1, 0, true);

    std::cout << "[=] [P] Press any key to ReLaunchChildProcess Child1: \n";
    getchar();
    child_pid_1 = parent_ipc.ReLaunchChildProcess(child_pid_1);
    parent_ipc.ConnectedToChildProcess(child_pid_1);

    MSG msg;
    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void __stdcall OnParentReceiveMsg32(void* pArg, char* msg, int arg3, int arg4, char* addition_msg, int addition_msg_size)
{
    char* cb_arg = (char*)pArg;
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Arg: " << cb_arg << std::endl;

    std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
    if (addition_msg_size != 0)
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | The Addition Msg: ";
        for (size_t i = 0; i < addition_msg_size; i++)
            printf_s("0x%02X ", (byte)(addition_msg[i]));
        printf_s("(%s)", addition_msg);
        puts("");
    }

    if (msg_str == "IPC_CONNECTED")
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | " << cb_arg << " : Alive!" << std::endl;
    }
    else if (msg_str == "IPC_DISCONNECTED")
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | " << cb_arg << " : Dead!" << std::endl;
    }

    //qqimpl::QQIpcParentWrapper::OnDefaultReceiveMsg(pArg, msg, arg3, arg4, addition_msg, addition_msg_size);
}


void OnParentReceiveMsg64(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size)
{
    char* cb_arg = (char*)pArg;
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Arg: " << cb_arg << std::endl;

    std::string msg_str = msg, add_msg_str(addition_msg, addition_msg_size);
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Msg: " << msg_str << std::endl;
    std::cout << "[*] [P] [ " << __FUNCTION__ << "] | Addition Msg Size: " << addition_msg_size << std::endl;
    if (addition_msg_size != 0)
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | The Addition Msg: ";
        for (size_t i = 0; i < addition_msg_size; i++)
            printf_s("0x%02X ", (byte)(addition_msg[i]));
        printf_s("(%s)", addition_msg);
        puts("");
    }

    if (msg_str == "IPC_CONNECTED")
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | " << cb_arg << " : Alive!" << std::endl;
    }
    else if (msg_str == "IPC_DISCONNECTED")
    {
        std::cout << "[*] [P] [ " << __FUNCTION__ << "] | " << cb_arg << " : Dead!" << std::endl;
    }

    //qqimpl::QQIpcParentWrapper::OnDefaultReceiveMsg(pArg, msg, arg3, addition_msg, addition_msg_size);
}
