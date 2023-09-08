# QQImpl
## 项目说明
本项目为通过逆向QQNT，将QQ封装的Mojo IPC模块独立出来，可以在任何项目中集成此模块，利用QQ Mojo IPC进行IPC通信。  
同时逆向出调用WeChatOCR的方法，集成此模块也可以方便的调用WeChatOCR。  
32位程序依赖parent-ipc-core-x86.dll  
64位程序依赖parent-ipc-core-x64.dll

## 使用说明
### x86:
1. 在VS项目中添加ocr_protobuf.pb.cc、QQImpl32.cpp
2. 附加包含目录添加:  
$(SolutionDir)..\QQImpl  
$(SolutionDir)..\QQImpl\ocr_protobuf\  
$(SolutionDir)..\QQImpl\ocr_protobuf\protobuf_header
3. 在链接器附加依赖库添加 $(SolutionDir)..\QQImpl\ocr_lib\libprotobuf_32_MT.lib

### x64:
1. 在VS项目中添加ocr_protobuf.pb.cc、QQImpl64.cpp、QQIpc64.asm
2. 右键QQIpc64.asm属性->项类型改为 Microsoft Macro Assembler
2. 附加包含目录添加:  
$(SolutionDir)..\QQImpl  
$(SolutionDir)..\QQImpl\ocr_protobuf\  
$(SolutionDir)..\QQImpl\ocr_protobuf\protobuf_header
3. 在链接器附加依赖库添加 $(SolutionDir)..\QQImpl\ocr_lib\libprotobuf_64_MT.lib
