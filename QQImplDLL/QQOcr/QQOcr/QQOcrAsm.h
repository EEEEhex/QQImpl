#include <Windows.h>

//MMMojo_64
extern "C" int OnReadPushTransfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnReserved2Transfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnReserved3Transfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);
extern "C" int OnRemoteConnectTransfer(DWORD64 arg1, DWORD64 arg2);
extern "C" int OnRemoteDisconnectTransfer(DWORD64 arg1);
extern "C" int OnRemoteProcessLaunchedTransfer(DWORD64 arg1);
extern "C" int OnRemoteProcessLaunchFailedTransfer(DWORD64 arg1, DWORD64 arg2);
extern "C" int OnRemoteMojoErrorTransfer(DWORD64 arg1, DWORD64 arg2, DWORD64 arg3);