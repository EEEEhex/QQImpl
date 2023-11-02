#ifdef USE_WRAPPER
#include "google/protobuf/stubs/common.h"
#endif

#include <Windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
#ifdef USE_WRAPPER
        google::protobuf::ShutdownProtobufLibrary();
#endif
        break;
    }
    return TRUE;
}

