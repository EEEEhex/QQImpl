# -*- coding: utf-8 -*-

'''
此文件依赖protobuf包
请先pip install protobuf==3.20.3
'''
import mmmojo_call
import ctypes
import sys
import ocr_protobuf_pb2
import utility_protobuf_pb2

def PyUsrReadOnPush(request_id, request_info, user_data):
	print("[↓] PyUsrReadOnPush [RequestID: {}] :\n[".format(request_id))

	pb_size = ctypes.c_int();
	pb_data = mmmojo_call.GetPbSerializedData(request_info, pb_size);

	print("\t[*] Request ID: {} | Protobuf Size: {}".format(request_id, pb_size))
	print("\t[*] Protobuf Data: ", end="")

	# 使用 ctypes 来访问和处理返回的内存内容
	data_ = (ctypes.c_char * pb_size.value).from_address(pb_data)
	for byte_ in data_:
		print("0x{:02X} ".format(byte_), end="");

	mmmojo_call.RemoveReadInfo(request_info);
	print("\n]")


def PyUsrReadOnPull(request_id, request_info, user_data):
	print("[↓] PyUsrReadOnPull [RequestID: {}] :\n[".format(request_id))

	pb_size = ctypes.c_int();
	pb_data = mmmojo_call.GetPbSerializedData(request_info, pb_size);

	print("\t[*] Request ID: {} | Protobuf Size: {}".format(request_id, pb_size))
	print("\t[*] Protobuf Data: ", end="")

	# 使用 ctypes 来访问和处理返回的内存内容
	data_ = (ctypes.c_ubyte * pb_size.value).from_address(pb_data)
	for byte_ in data_:
		print("0x{:02X} ".format(byte_), end="");

	mmmojo_call.RemoveReadInfo(request_info);
	print("\n]")

def PyOCRUsrReadOnPush(pic_path, pb_data, pb_size):
	print("[↓] OCRUsrReadOnPush:\n[")

	#把void*转为python可用的数据
	ocr_response_ubyte = (ctypes.c_ubyte*pb_size).from_address(pb_data)
	ocr_response_array = bytearray(ocr_response_ubyte)

	ocr_protobuf = ocr_protobuf_pb2.OcrResponse()
	ocr_protobuf.ParseFromString(ocr_response_array)

	print("\t[*] type:{} taskId:{} errCode:{}".format(ocr_protobuf.type, ocr_protobuf.task_id, ocr_protobuf.err_code))

	if pic_path != None:
		print("\t[*] TaskId: {} -> PicPath: {}".format(ocr_response.task_id, pic_path));
	else:
		print("\t[*] This is a type 1 Push that is called back only once at startup of WeChatOCR.exe", end="")
	
	if ocr_response.type == 0:
		print("\t[*] OcrResult:\n\t[")
		response_ocr_result = ocr_protobuf.ocr_result
		for single_result in response_ocr_result.single_result:
			print("\t\tRECT:[left: {}, top: {}, right: {}, bottom: {}]".format(single_result.left, single_result.top, single_result.right, single_result.bottom))
			single_str = single_result.single_str_utf8.decode('utf-8')
			print("\t\tUTF8STR:[{} | Rate: {}]".format(single_str, single_result.single_rate))
		print("\t]", end="")
	print("\n]")

def PyUtilityUsrReadOnPush(type_id, serialized_data, data_size):
	print("[↓] PyUtilityUsrReadOnPush [RequestID: {}] :\n[".format(type_id))

	if type_id == mmmojo_call.UtilityTextScanPushResp:
		utility_response_ubyte = (ctypes.c_ubyte*data_size).from_address(serialized_data)
		utility_response_array = bytearray(utility_response_ubyte)

		text_scan_msg = utility_protobuf_pb2.TextScanMessage()
		text_scan_msg.ParseFromArray(utility_response_array);
		print("\t[*] TextScanResult: ID: [{}] | PicPath: [{}] | HaveText?: [{}] | UKN0: [{}] | Rate: [{}]".format(text_scan_msg.text_scan_id, text_scan_msg.pic_path, text_scan_msg.have_text, text_scan_msg.unknown_0, text_scan_msg.rate))
	print("\n]")

def PyUtilityUsrReadOnPull(type_id, serialized_data, data_size):
	print("[↓] PyUtilityUsrReadOnPush [RequestID: {}] :\n[".format(type_id))

	if type_id == mmmojo_call.UtilityInitPullResp:
		print("\t[*] UtilityInitPullResp", end="")
	elif type_id == mmmojo_call.UtilityQRScanPullResp:
		utility_response_ubyte = (ctypes.c_ubyte*data_size).from_address(serialized_data)
		utility_response_array = bytearray(utility_response_ubyte)

		qrscan_response = utility_protobuf_pb2.QRScanRespMessage()
		qrscan_response.ParseFromArray(utility_response_array)

		print("\t[*] QRScanResult UKN0: [{}] :\n\t[".format(qrscan_response.unknown_0))
		for qrscan_result in qrscan_response.qr_result:
			result = qrscan_result.result
			unknown_0 = qrscan_result.unknow_0
			unknown_1 = qrscan_result.unknown_1
			unknown_2 = qrscan_result.unknown_2
			unknown_3 = qrscan_result.unknown_3
			print("t\tResult: [{}] | UKN0: [{}] | UKN1: [{}] | UKN2: [{}] |  UNK3: [{}]".format(result, unknown_0, unknown_1, unknown_2, unknown_3))
		print("\t]", end="")

	print("\n]")


if __name__ == '__main__':
	wechat_ocr_dir = input('\033[34m[=] Enter WeChatOCR.exe Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('WeChatOCR.exe Path can\'t be EMPTY!')
		sys.exit(1)

	wechat_dir = input('\033[34m[=] Enter mmmojo(_64).dll Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('mmmojo(_64).dll Path can\'t be EMPTY!')
		sys.exit(1)

	wechat_utility_dir = input('\033[34m[=] Enter WeChatUtility.exe Path:\n[>]\033[0m ')
	if not wechat_ocr_dir:
		print('WeChatUtility.exe Path can\'t be EMPTY!')
		sys.exit(1)


	#-----------------------------
	#以下为定义回调函数
	CFUNC_PYUSRREADONPUSH = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)
	py_usr_read_on_push = CFUNC_PYUSRREADONPUSH(PyUsrReadOnPush)
	py_usr_read_on_push_void_ptr = ctypes.cast(py_usr_read_on_push, ctypes.c_void_p)

	CFUNC_PYUSRREADONPULL = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)
	py_usr_read_on_pull = CFUNC_PYUSRREADONPULL(PyUsrReadOnPull)
	py_usr_read_on_pull_void_ptr = ctypes.cast(py_usr_read_on_pull, ctypes.c_void_p)

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
	if not mmmojo_call.InitMMMojoDLLFuncs(wechat_dir):
		print('\033[31m[!] mmmojocall::InitMMMojoDLLFuncs ERR!\033[0m')
		sys.exit(1)


	# 以下为直接使用XPluginManager类调用微信XPlugin组件(WeChatOCR)的示例
	cobj_xlpugin_mgr = mmmojo_call.XPluginManager()
	if not cobj_xlpugin_mgr.SetExePath(wechat_ocr_dir): # 设置XPlugin组件路径
		print('\033[31m[!] {}\033[0m'.format(cobj_xlpugin_mgr.GetLastErrStr()))
		sys.exit(1)

	cobj_xlpugin_mgr.AppendSwitchNativeCmdLine("user-lib-dir", wechat_dir)# 添加需要的Switch命令行
	cobj_xlpugin_mgr.SetOneCallback(mmmojo_call.kMMReadPush, py_usr_read_on_push_void_ptr.value)# 设置ReadPush回调函数
	cobj_xlpugin_mgr.SetOneCallback(mmmojo_call.kMMReadPull, py_usr_read_on_pull_void_ptr.value)# 设置ReadPull回调函数
	if not cobj_xlpugin_mgr.InitMMMojoEnv():# 启动环境和组件
		print('\033[31m[!] {}\033[0m'.format(cobj_xlpugin_mgr.GetLastErrStr()))
		sys.exit(1)
	print("\033[31m[+] InitMMMojoEnv OK!\033[0m")

	pb_hex_data = [0x10, 0x01, 0x1A, 0x0C, 0x0A, 0x0A, 0x2E, 0x5C, 0x74, 0x65, 0x73, 0x74, 0x2E, 0x70, 0x6E, 0x67]
	input("[=] Press any key to send request...\n")# 发送请求
	cobj_xplugin_mgr.SendPbSerializedData(pb_hex_data, len(pb_hex_data), mmmojo_call.kMMPush, False, mmmojo_call.OCRPush);

	input("[=] Press any key to stop xplugin mmmojo env...\n")# 停止环境
	cobj_xplugin_mgr.StopMMMojoEnv();
	print("\033[31m[-] StopMMMojoEnv...\033[0m")


	#------------------------
	#以下为使用封装好的OCRManager类调用WeChatOCR进行OCR的示例 与 UtilityManager类调用WeChatUtility扫描二维码的示例
	cobj_ocr_mgr = mmmojo_call.OCRManager()
	if not cobj_ocr_mgr.SetExePath(wechat_ocr_dir):
		print('\033[31m[!] {}\033[0m'.format(cobj_ocr_mgr.GetLastErrStr()))
		sys.exit(1)
	

	if not cobj_ocr_mgr.SetUsrLibDir(wechat_dir):
		print('\033[31m[!] {}\033[0m'.format(cobj_ocr_mgr.GetLastErrStr()))
		sys.exit(1)

	cobj_ocr_mgr.SetReadOnPush(py_ocr_usr_read_on_push);
	if not cobj_ocr_mgr.StartWeChatOCR():
		print('\033[31m[!] {}\033[0m'.format(cobj_ocr_mgr.GetLastErrStr()))
		sys.exit(1)
	print("\033[31m[+] StartWeChatOCR OK!\033[0m")

	input("[=] Press any key to send ocr request...\n")
	if not cobj_ocr_mgr.DoOCRTask(".\\test.png"):# 请求一次OCR 图片路径为.\\test.png
		print('\033[31m[!] {}\033[0m'.format(cobj_ocr_mgr.GetLastErrStr()))
		sys.exit(1)

	input("[=] Press any key to stop WeChatOCR Env...\n")
	cobj_ocr_mgr.KillWeChatOCR();
	print("\033[31m[-] KillWeChatOCR!\033[0m")

	#QRScan
	cobj_utility_mgr = mmmojo_call.UtilityManager()
	if not cobj_utility_mgr.SetExePath(wechat_utility_dir):
		print('\033[31m[!] {}\033[0m'.format(cobj_utility_mgr.GetLastErrStr()))
		sys.exit(1)

	if not cobj_utility_mgr.SetUsrLibDir(wechat_dir):
		print('\033[31m[!] {}\033[0m'.format(cobj_utility_mgr.GetLastErrStr()))
		sys.exit(1)

	cobj_utility_mgr.SetReadOnPull(py_utility_usr_read_on_pull);
	cobj_utility_mgr.SetReadOnPush(py_utility_usr_read_on_push);
	if not cobj_utility_mgr.StartWeChatUtility():
		print('\033[31m[!] {}\033[0m'.format(cobj_utility_mgr.GetLastErrStr()))
		sys.exit(1)
	print("\033[31m[+] StartWeChatUtility OK!\033[0m")

	input("[=] Press any key to send QRScan request...\n")
	if not cobj_utility_mgr.DoPicQRScan(".\\test.png", 0xEE):# 参数为要QRScan的图片路径 TextScan任务ID
		print('\033[31m[!] {}\033[0m'.format(cobj_utility_mgr.GetLastErrStr()))

	input("[=] Press any key to stop WeChatUtility Env...\n")
	cobj_utility_mgr.KillWeChatUtility();
	print("\033[31m[-] KillWeChatUtility\033[0m")







