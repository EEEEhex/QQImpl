#pragma once
#include <Windows.h>

//Parent IPC
extern "C" void InitLogAsm(DWORD64 this_ptr, DWORD64 callback, DWORD64 level, DWORD64 call_addr);
extern "C" void InitParentIpcAsm(DWORD64 this_ptr, DWORD64 call_addr);
extern "C" int  LaunchChildProcessAsm(DWORD64 this_ptr, DWORD64 path_ptr, DWORD64 cmdlines, DWORD64 cmdlines_num, DWORD64 callback, DWORD64 class_ptr, DWORD64 call_addr);//arg6是一个类指针, 是接收MSG函数的第一个参数
extern "C" void ConnectedToChildProcessAsm(DWORD64 this_ptr, DWORD64 child_pid, DWORD64 call_addr);
extern "C" int SendIpcMessageAsm(DWORD64 this_ptr, DWORD64 child_pid, DWORD64 msg_ptr, DWORD64 arg4, DWORD64 addition_ptr, DWORD64 addition_len, DWORD64 call_addr);//arg4为1
extern "C" void TerminateChildProcessAsm(DWORD64 this_ptr, DWORD64 child_pid, DWORD64 exit_code, DWORD64 is_wait, DWORD64 call_addr);
extern "C" int	ReLaunchChildProcessAsm(DWORD64 this_ptr, DWORD64 child_pid, DWORD64 call_addr);

//Child IPC
extern "C" void InitChildIpcAsm(DWORD64 this_ptr, DWORD64 call_addr);
extern "C" void SetChildReceiveCallbackAndCOMPtrAsm(DWORD64 this_ptr, DWORD64 callback, DWORD64 com_ptr, DWORD64 call_addr);
extern "C" int ChildSendIpcMessageAsm(DWORD64 this_ptr, DWORD64 msg_ptr, DWORD64 arg3, DWORD64 addition_ptr, DWORD64 addition_len, DWORD64 call_addr);//arg3为0

//MMMojo_64
extern "C" int OnReadPushTransfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnReserved2Transfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnReserved3Transfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnRemoteConnectTransfer(DWORD64 arg1, DWORD64 arg2);
extern "C" int OnRemoteDisconnectTransfer(DWORD64 arg1);
extern "C" int OnRemoteProcessLaunchedTransfer(DWORD64 arg1);
extern "C" int OnRemoteProcessLaunchFailedTransfer(DWORD64 arg1, DWORD64 arg2);
extern "C" int OnRemoteMojoErrorTransfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);