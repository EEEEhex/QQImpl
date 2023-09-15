# QQImpl
## 项目说明
本项目为通过逆向QQNT，将QQ封装的Mojo IPC模块独立出来，可以在任何项目中集成此模块，利用QQ Mojo IPC进行IPC通信。  
同时逆向出调用WeChatOCR的方法，集成此模块也可以方便的调用WeChatOCR。   

QQIMPL_TEST.7z是测试使用的, ParentIpc和ChildIpc用于测试Mojio IPC功能, OcrTest用于测试WeChatOCR功能

## 使用说明
> 假设你的项目结构是这样的:  
> .  
> ├─Bin  
> ├─YourProject  
> ├─QQImpl  

### IPC
> 32位程序依赖parent-ipc-core-x86.dll   
> 64位程序依赖parent-ipc-core-x64.dll

1. 在vs项目中添加文件: QQIpc.cpp
2. 附加包含目录添加: \$(SolutionDir)..\\QQImpl\\QQIPC  
  
如果是x64则还需要:  
1. 在vs项目中新添加文件: QQIpcAsm.asm
2. 右键项目->生成依赖项->生成自定义->将masm勾上
3. 右键QQIpcAsm.asm属性->项类型改为 Microsoft Macro Assembler

### OCR
> 在QQImpl\\QQOCR\\ocr_lib中有编译好的libprotobuf (3.20.3), 可以自己编译protobuf, 再用protoc.exe编译一下QQImpl\\QQOCR\\ocr_protobuf\\ocr_protobuf.proto即可生成ocr_protobuf.pb.h与ocr_protobuf.pb.cc
1. 在vs项目中添加文件: QQOcr.cpp ocr_protobuf.pb.cc
2. 附加包含目录添加:  
\$(SolutionDir)..\\QQImpl\\QQOCR  
\$(SolutionDir)..\\QQImpl\\QQOCR\\ocr_protobuf\\  
3. 在链接器附加依赖库添加: \$(SolutionDir)..\\QQImpl\\ocr_lib\\对应的lib   
4. 如果想要使用dll版的libprotobuf, 需要在预处理器定义里添加PROTOBUF_USE_DLLS

如果是x64则还需要:
1. 在vs项目中新添加文件: QQOcrAsm.asm
2. 右键项目->生成依赖项->生成自定义->将masm勾上
3. 右键QQOcrAsm.asm属性->项类型改为 Microsoft Macro Assembler
