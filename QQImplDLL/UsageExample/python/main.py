# -*- coding: utf-8 -*-

'''
此文件依赖protobuf包
请先pip install protobuf==3.20.3
'''

import os
import sys
from ctypes import *
import ocr_protobuf_pb2

#用户自定义接收OCR结果的函数
def py_on_usr_read_push(pic_path, ocr_response_serialize_data, serialize_size):
	print('\033[33m[#]\033[0m Recv PicPath:[{}] Serialize size:[{}]'.format(pic_path.decode('utf-8'), serialize_size))
	#把c_void_p转为python可用的数据
	ocr_response_ubyte = (c_ubyte*serialize_size).from_address(ocr_response_serialize_data)
	ocr_response_array = bytearray(ocr_response_ubyte)

	ocr_protobuf = ocr_protobuf_pb2.OcrResponse()
	ocr_protobuf.ParseFromString(ocr_response_array)
	response_type = ocr_protobuf.type
	response_task_id = ocr_protobuf.task_id
	response_err_code = ocr_protobuf.err_code
	print("\033[33m[#]\033[0m OnPyReadPush: type:{} taskId:{} errCode:{}".format(response_type, response_task_id, response_err_code))
	if (response_type == 0):
		#下面进行解析
		response_ocr_result = ocr_protobuf.ocr_result
		for single_result in response_ocr_result.single_result:
			single_str = single_result.single_str_utf8.decode('utf-8')
			print('\033[33m[#]\033[0m Utf8Str: {} | Rate: {}'.format(single_str, single_result.single_rate))

	return None


if __name__ == '__main__':
	qqocr_dll = cdll.LoadLibrary('./QQOcr.dll')
	print('[+] LoadLibrary QQOcr.dll OK!')

	#-------------------------
	#以下为DLL导出函数初始化操作
	InitOcrManager = qqocr_dll.InitManager
	InitOcrManager.restype = None
	
	UnInitOcrManager = qqocr_dll.UnInitManager
	UnInitOcrManager.restype = None
	
	SetOcrExePath = qqocr_dll.SetOcrExePath
	SetOcrExePath.argtypes = [c_char_p]
	SetOcrExePath.restype = c_bool
	
	SetOcrUsrLibPath = qqocr_dll.SetOcrUsrLibPath
	SetOcrUsrLibPath.argtypes = [c_char_p]
	SetOcrUsrLibPath.restype = c_bool
	
	SetOcrUsrReadPushCallback = qqocr_dll.SetUsrReadPushCallback
	SetOcrUsrReadPushCallback.restype = None
	
	GetOcrLastErrStr = qqocr_dll.GetLastErrStr
	GetOcrLastErrStr.restype = c_char_p
	
	DoOCRTask = qqocr_dll.DoOCRTask
	DoOCRTask.argtypes = [c_char_p]
	DoOCRTask.restype = c_bool
	print('[+] Init Dll Export Funcs Over!')

	#-----------------------------
	#以下为定义接收OCR结果的回调函数
	CFUNC_ONUSRREADPUSH = CFUNCTYPE(None, c_char_p, c_void_p, c_int)
	on_usr_read_push = CFUNC_ONUSRREADPUSH(py_on_usr_read_push)
	print('[+] Def Python OCR OnReadPush Func Callback Over!')
	
	#------------------
	#以下为OCR初始化操作
	exe_path = input('\033[34m[*]\033[0m Enter WeChatOCR.exe Path:\n\033[34m[>]\033[0m ')
	if not exe_path:
		print('WeChatOCR.exe Path can\'t be EMPTY!')
		sys.exit(1)
	exe_path_c = c_char_p(exe_path.encode('utf-8'))

	usr_lib_path = input('\033[34m[*]\033[0m Enter mmmojo(_64).dll Path:\n\033[34m[>]\033[0m ')
	if not usr_lib_path:
		print('mmmojo(_64).dll Path can\'t be EMPTY!')
		sys.exit(1)
	usr_lib_path_c = c_char_p(usr_lib_path.encode('utf-8'))


	if (os.path.exists(exe_path_c.value) == False):
		print('\033[31m[!] WeChatOCR.exe Path is Not Exist!\033[0m')
		sys.exit(1)
	if (os.path.exists(usr_lib_path_c.value) == False):
		print('\033[31m[!] mmmojo(_64).dll Path is Not Exist!\033[0m')
		sys.exit(1)

	#初始化OcrManager
	InitOcrManager()
	print('[+] InitOcrManager OK!')
	
	#设置WeChatOCR.exe所在路径
	if (SetOcrExePath(exe_path_c) == False):
		print('\033[31m[!] SetOcrExePath Err: ' + GetOcrLastErrStr() + '\033[0m')
		sys.exit(1)
	else:
		print('[+] SetOcrExePath OK!')
	
	#设置mmmojo_64.dll所在路径
	if (SetOcrUsrLibPath(usr_lib_path_c) == False):
		print('\033[31m[!] SetOcrUsrLibPath Err: ' + GetOcrLastErrStr() + '\033[0m')
		sys.exit(1)
	else:
		print('[+] SetOcrUsrLibPath OK!')

	#设置接收OCR Response序列化数据的回调函数
	SetOcrUsrReadPushCallback(on_usr_read_push)
	print('[+] SetOcrUsrReadPushCallback OK!')

	#获取要OCR的图片路径
	pic_path = input("\033[34m[*]\033[0m Enter PicPath to Send OCR Task (Default test.png):\n\033[34m[>]\033[0m ")
	if not pic_path:
		pic_path = '.\\test.png'
	if (os.path.exists(pic_path) == False):
		print('\033[31m[!] PicPath is Not Exist!\033[0m')
		sys.exit(1)

	pic_path_c = c_char_p(pic_path.encode('utf-8'))
	if (DoOCRTask(pic_path_c) == False):
		print('\033[31m[!] DoOCRTask Err: ' + GetOcrLastErrStr() + '\033[0m')
		sys.exit(1)


	input('\033[34m[>]\033[0m Press any key to Shutdown OCR Manager: \n')
	UnInitOcrManager()
	print('[-] Shutdown OCR Env!')










