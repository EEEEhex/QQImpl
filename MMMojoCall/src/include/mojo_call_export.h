#pragma once

#ifdef MMMojoCall_EXPORTS
#define MMMOJOCALL_API __declspec(dllexport)
#else
#define MMMOJOCALL_API __declspec(dllimport)
#endif

enum MGRTYPE
{
	XPluginManager = 0,
	OCRManager,
	UtilityManager,
	PlayerManager
};

/**
 * @brief 以纯C的方式导出XPluginMgr类 方便其他语言调用
 * @param mgr_type 0为XPluginManager 1为OCRManager 2为UtilityManager
 * @return 类指针 失败返回NULL
 */
extern "C" MMMOJOCALL_API const void* GetInstanceXPluginMgr(int mgr_type);

/**
 * @brief 根据字符串调用相应的函数
 * @param class_ptr 类指针
 * @param mgr_type 类的类型
 * @param func_name 函数名
 * @param ret_ptr	存储返回值的指针 如果不使用返回值则传入NULL
 * @param ... 参数 一定要对应函数的正确参数 内部不做检查
 * @return 执行状态 0失败 1成功 2未知错误
 */
extern "C" MMMOJOCALL_API int CallFuncXPluginMgr(const void* class_ptr, int mgr_type, const char* func_name, void* ret_ptr, ...);

/**
 * @brief 销毁XPluginMgr类指针
 * @param 类指针
 */
extern "C" MMMOJOCALL_API void ReleaseInstanceXPluginMgr(const void* mgr_ptr);
