# QQImpl

## 使用说明

### x86:
1. 在VS项目中添加ocr_protobuf.pb.cc、QQImpl32.cpp
2. 附件包含目录添加:  
$(SolutionDir)..\QQImpl  
$(SolutionDir)..\QQImpl\ocr_protobuf\  
$(SolutionDir)..\QQImpl\ocr_protobuf\protobuf_header
3. 在链接器附加依赖库添加 $(SolutionDir)..\QQImpl\ocr_lib\libprotobuf_32_MT.lib

### x64:
1. 在VS项目中添加ocr_protobuf.pb.cc、QQImpl64.cpp、QQIpc64.asm
2. 右键QQIpc64.asm属性->项类型改为 Microsoft Macro Assembler
2. 附件包含目录添加:  
$(SolutionDir)..\QQImpl  
$(SolutionDir)..\QQImpl\ocr_protobuf\  
$(SolutionDir)..\QQImpl\ocr_protobuf\protobuf_header
3. 在链接器附加依赖库添加 $(SolutionDir)..\QQImpl\ocr_lib\libprotobuf_64_MT.lib
