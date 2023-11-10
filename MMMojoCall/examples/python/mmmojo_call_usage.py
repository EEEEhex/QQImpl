# -*- coding: utf-8 -*-

import ctypes
import sys
import json
import base64

def PyOCRUsrReadOnPush(pic_path, data, data_size):
	print("[↓] OCRUsrReadOnPush:\n[")

	# 设置了Mode的use_json为True则data为json字符串
	if pic_path != None:
		chr_ptr = ctypes.cast(data, ctypes.c_char_p)
		json_string = chr_ptr.value.decode('utf-8')
		json_obj = json.loads(json_string)

		# 解析json
		print("\t[*] type:{} taskId:{} errCode:{}".format(json_obj['type'], json_obj['task_id'], json_obj['err_code']))
		print("\t[*] TaskId: {} -> PicPath: {}".format(json_obj['task_id'], pic_path.decode('utf-8')));

		print("\t[*] OcrResult:\n\t[")
		response_ocr_result = json_obj['ocr_result']
		for single_result in response_ocr_result['single_result']:
			print("\t\tRECT:[left: {}, top: {}, right: {}, bottom: {}]".format(single_result['left'], single_result['top'], single_result['right'], single_result['bottom']))
			b64str = single_result['single_str_utf8']
			single_str = base64.b64decode(b64str).decode('utf-8')
			print("\t\tUTF8STR:[{} | Rate: {}]".format(single_str, single_result['single_rate']))
		print("\t]", end="")
	print("\n]")
	

def PyUtilityUsrReadOnPush(type_id, data, data_size):
	print("[↓] PyUtilityUsrReadOnPush [RequestID: {}] :\n[".format(type_id))

	UtilityTextScanPushResp = 10040
	# 设置了Mode的use_json为True则data为json字符串
	if type_id == UtilityTextScanPushResp:
		chr_ptr = ctypes.cast(data, ctypes.c_char_p)
		json_string = chr_ptr.value.decode('utf-8')
		json_obj = json.loads(json_string)
		print("\t[*] TextScanResult: ID: [{}] | PicPath: [{}] | HaveText?: [{}] | UKN0: [{}] | Rate: [{}]".format(\
			json_obj['text_scan_id'], json_obj['pic_path'], json_obj['have_text'], json_obj['unknown_0'], json_obj['rate']))
	print("\n]")

def PyUtilityUsrReadOnPull(type_id, data, data_size):
	print("[↓] PyUtilityUsrReadOnPush [RequestID: {}] :\n[".format(type_id))

	UtilityInitPullResp = 10003
	UtilityQRScanPullResp = 10032
	if type_id == UtilityInitPullResp:
		print("\t[*] UtilityInitPullResp", end="")
	elif type_id == UtilityQRScanPullResp:
		chr_ptr = ctypes.cast(data, ctypes.c_char_p)
		json_string = chr_ptr.value.decode('utf-8')
		json_obj = json.loads(json_string)

		print("\t[*] QRScanResult UKN0: [{}] :\n\t[".format(json_obj['unknown_0']))
		for qrscan_result in json_obj['qr_result']:
			b64result = qrscan_result['result']
			result = base64.b64decode(b64result).decode('utf-8')
			unknown_0 = qrscan_result['unknow_0']
			unknown_1 = qrscan_result['unknown_1']
			unknown_2 = qrscan_result['unknown_2']
			unknown_3 = qrscan_result['unknown_3']
			print("\t\tResult: [{}] | UKN0: [{}] | UKN1: [{}] | UKN2: [{}] |  UNK3: [{}]".format(result, unknown_0, unknown_1, unknown_2, unknown_3))
		print("\t]", end="")

	print("\n]")

# 调用XPluginManager::GetLastErrStr
def call_mmmojocall_getlasterrstr(cobj_mgr):
	call_ret = ctypes.c_ulonglong()
	method_get_last_err_str = ctypes.c_char_p("GetLastErrStr".encode('utf-8'))
	mmmojocall_call_func(cobj_mgr, 0, method_get_last_err_str, ctypes.byref(call_ret))
	chr_ptr = ctypes.cast(call_ret.value, ctypes.c_char_p)
	err_string = chr_ptr.value.decode('utf-8')
	return err_string

if __name__ == '__main__':
	mmmojocall_dll = ctypes.cdll.LoadLibrary('./MMMojoCall.dll')
	print('[+] LoadLibrary MMMojoCall.dll OK!')

	#-------------------------
	#以下为DLL导出函数初始化操作
	mmmojocall_get_instance = mmmojocall_dll.GetInstanceXPluginMgr
	mmmojocall_get_instance.argtypes = [ctypes.c_int]
	mmmojocall_get_instance.restype = ctypes.c_void_p
	
	mmmojocall_call_func = mmmojocall_dll.CallFuncXPluginMgr
	mmmojocall_call_func.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_void_p]
	mmmojocall_call_func.restype = ctypes.c_int
	
	mmmojocall_release_instance = mmmojocall_dll.ReleaseInstanceXPluginMgr
	mmmojocall_release_instance.argtypes = [ctypes.c_void_p]
	mmmojocall_release_instance.restype = None

	mmmojocall_init_dll_funcs = mmmojocall_dll.InitMMMojoDLLFuncs
	mmmojocall_init_dll_funcs.argtypes = [ctypes.c_char_p]
	mmmojocall_init_dll_funcs.restype = ctypes.c_bool

	print('[+] Init Dll Export Funcs Over!')

	#----------------------------
	#以下为读入WeChat组件路径
	wechat_ocr_dir = input('\033[34m[=] Enter WeChatOCR.exe Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('[!] WeChatOCR.exe Path can\'t be EMPTY!')
		sys.exit(1)
	wechat_ocr_dir_c = ctypes.c_char_p(wechat_ocr_dir.encode('utf-8'))

	wechat_dir = input('\033[34m[=] Enter mmmojo(_64).dll Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('[!] mmmojo(_64).dll Path can\'t be EMPTY!')
		sys.exit(1)
	wechat_dir_c = ctypes.c_char_p(wechat_dir.encode('utf-8'))

	wechat_utility_dir = input('\033[34m[=] Enter WeChatUtility.exe Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('[!] WeChatUtility.exe Path can\'t be EMPTY!')
		sys.exit(1)
	wechat_utility_dir_c = ctypes.c_char_p(wechat_utility_dir.encode('utf-8'))

	#-----------------------------
	#以下为定义回调函数
	CFUNC_PYOCRUSRREADONPUSH = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int)
	py_ocr_usr_read_on_push = CFUNC_PYOCRUSRREADONPUSH(PyOCRUsrReadOnPush)

	CFUNC_PYUTILITYUSRREADONPUSH = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_void_p, ctypes.c_int)
	py_utility_usr_read_on_push = CFUNC_PYUTILITYUSRREADONPUSH(PyUtilityUsrReadOnPush)

	CFUNC_PYUTILITYUSRREADONPULL = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_void_p, ctypes.c_int)
	py_utility_usr_read_on_pull = CFUNC_PYUTILITYUSRREADONPULL(PyUtilityUsrReadOnPull)
	print('\033[31m[+] Def Python Callback Funcs Over!\033[0m')

	'''
	[可选] 加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.	
		若要启动的组件未设置"user-lib-dir"参数, 则需手动调用此函数, 
		但若要启动的组件设置了user-lib-dir这个参数, 则在InitMMMojoEnv时会自动调用此函数.
	'''
	if not mmmojocall_init_dll_funcs(wechat_dir_c):
		print('\033[31m[!] mmmojocall::InitMMMojoDLLFuncs ERR!\033[0m')
		sys.exit(1)
	print('\033[31m[+] InitMMMojoDLLFuncs Over!\033[0m')
	
	#------------------------
	#以下为使用封装好的OCRManager类调用WeChatOCR进行OCR的示例 与 UtilityManager类调用WeChatUtility扫描二维码的示例
	call_ret = ctypes.c_ulonglong()

	mgr_type_ocr = 1
	cobj_ocr_mgr = mmmojocall_get_instance(mgr_type_ocr)
	
	method_name = ctypes.c_char_p("SetExePath".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret), wechat_ocr_dir_c)
	if not call_ret.value:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_ocr_mgr)))
		sys.exit(1)
	
	method_name = ctypes.c_char_p("SetUsrLibDir".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret), wechat_dir_c)
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_ocr_mgr)))
		sys.exit(1)

	method_name = ctypes.c_char_p("SetCallbackDataMode".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret), True)

	method_name = ctypes.c_char_p("SetReadOnPush".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret), py_ocr_usr_read_on_push)

	method_name = ctypes.c_char_p("StartWeChatOCR".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret))
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_ocr_mgr)))
		sys.exit(1)

	print("\033[31m[+] StartWeChatOCR OK!\033[0m")

	input("[=] Press any key to send ocr request...\n")
	method_name = ctypes.c_char_p("DoOCRTask".encode('utf-8'))
	pic_path_c = ctypes.c_char_p(".\\test.png".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, ctypes.byref(call_ret), pic_path_c)
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_ocr_mgr)))
		sys.exit(1)

	input("[=] Press any key to stop WeChatOCR Env...\n")
	method_name = ctypes.c_char_p("KillWeChatOCR".encode('utf-8'))
	mmmojocall_call_func(cobj_ocr_mgr, mgr_type_ocr, method_name, None)
	print("\033[31m[-] KillWeChatOCR!\033[0m")

	mmmojocall_release_instance(cobj_ocr_mgr)

	#QRScan
	mgr_type_utility = 2
	cobj_utility_mgr = mmmojocall_get_instance(mgr_type_utility)
	
	method_name = ctypes.c_char_p("SetExePath".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), wechat_utility_dir_c)
	if not call_ret.value:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_utility_mgr)))
		sys.exit(1)
	
	method_name = ctypes.c_char_p("SetUsrLibDir".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), wechat_dir_c)
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_utility_mgr)))
		sys.exit(1)

	method_name = ctypes.c_char_p("SetCallbackDataMode".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), True)

	method_name = ctypes.c_char_p("SetReadOnPull".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), py_utility_usr_read_on_pull)

	method_name = ctypes.c_char_p("SetReadOnPush".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), py_utility_usr_read_on_push)

	method_name = ctypes.c_char_p("StartWeChatUtility".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret))
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_utility_mgr)))
		sys.exit(1)

	print("\033[31m[+] StartWeChatUtility OK!\033[0m")

	input("[=] Press any key to send OCRScan request...\n")
	method_name = ctypes.c_char_p("DoPicQRScan".encode('utf-8'))
	pic_path_c = ctypes.c_char_p(".\\test.png".encode('utf-8'))
	text_scan_id = 1
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, ctypes.byref(call_ret), pic_path_c, text_scan_id)
	if not call_ret:
		print('\033[31m[!] {}\033[0m'.format(call_mmmojocall_getlasterrstr(cobj_utility_mgr)))
		sys.exit(1)

	input("[=] Press any key to stop WeChatUtility Env...\n")
	method_name = ctypes.c_char_p("KillWeChatUtility".encode('utf-8'))
	mmmojocall_call_func(cobj_utility_mgr, mgr_type_utility, method_name, None)
	print("\033[31m[-] KillWeChatUtility!\033[0m")

	mmmojocall_release_instance(cobj_utility_mgr)



