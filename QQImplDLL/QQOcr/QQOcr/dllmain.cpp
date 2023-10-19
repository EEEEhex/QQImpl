#include "framework.h"

/**
 * !此DLL采用MT进行编译!
 * 因为当主程序也使用了ocr_protobuf.pb.cc并同时也加载了libprotobuf.dll时会报File already exists in database错误.
 * 且谷歌推荐采用静态链接
 */

/**
 * !如果要编译64位的.
 * 需要右键QQOcrAsm.asm->属性->将项类型改为'Microsoft Macro Assembler'
 * !如果要编译32位的.
 * 需要右键QQOcrAsm.asm->属性->将项类型改为'不参与生成'
 */

#include "ocr_protobuf.pb.h"

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
        google::protobuf::ShutdownProtobufLibrary();
        break;
    }
    return TRUE;
}

