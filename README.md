# 项目说明
本项目通过逆向QQNT 逆向出调用QQ Mojo IPC的方法, 使用QQMojoIPC可以调用QQNT的插件并与之通信。  
并在此基础上逆向出调用微信MMMojo.dll的方法, 通过调用MMMojo可以调用WeChat的插件并与之通信。  
  
**目前可以调用QQNT的如下组件:**
- [x] QQScreenShot

**目前可以调用WeChat的如下组件:**  
1. WeChatOCR.exe:  
- [x] OCR功能  

2. WeChatUtility.exe:
- [x] QRScan功能 (二维码扫描)
- [x] TextScan功能 (可能是用来判断图片上有没有文字的)
- [ ] ResampleImage功能
- [ ] DecodeImage功能

3. WeChatPlayer.exe:
- [x] 本地视频播放功能
- [ ] 网络视频播放功能

# 项目结构  
├─3rdparty  
├─examples  
├─proto  
└─src  
\ \ ├─include  
\ \ │  ├─mmmojo_source  
\ \ │  ├─qq_mojoipc  
\ \ │  └─xplugin_protobuf  
\ \ ├─mmmojocall  
\ \ │  └─wrapper  
\ \ └─qq_mojoipc  
 
(此版本根据微信的mmmojo源码再次重构, 逻辑更清晰)  
其中3rdparty是编译好的libprotobuf库, examples里是调用方法的示例, proto里是WeChat与组件之间的Protobuf通信协议的定义文件。  
src文件夹里 include中mmmojo_source是微信MMMojo的源代码中的一些定义, qq_mojoipc是实现QQMojoIPC的头文件, xplugin_protobuf是使用protoc编译后的头文件。  
qq_mojoipc文件夹是实现QQMojoIPC的实现文件, mmmojocall里是实现调用微信XPlugin组件的实现文件, 其中mmmojo_call.cpp是基础的调用XPlugin的方法, wrapper里是封装了对应组件通信协议的文件。

# 编译说明
本项目使用cmake, 以下为cmake选项说明:  
|  Option   | 说明  |
|  ----  | ----  |
| XPLUGIN_WRAPPER  | 编译封装好WeChat通信协议的实现文件 默认为ON |
| BUILD_QQIPC  | 编译QQMojoIPC的实现文件 默认为OFF |
| BUILD_CPPEXAMPLE | 编译调用方法示例项目 默认为OFF |
| EXAMPLE_USE_JSON | CPP示例项目的回调函数的数据使用Json格式而非protobuf 默认为ON |
| BUILD_PURE_C_MODE | 编译纯C接口方便其他语言调用 默认为OFF| 

## 快速上手
> 在编译前请先解压3rdparty下的压缩文件(或者自己编译一下protobuf) 以及src/include/xplugin_protobuf下的压缩的头文件

使用如下命令行:
```
cmake -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_CPPEXAMPLE=ON
```
如果需要使用其他语言调用, 需要加上(调用方法请查看examples文件夹)
```
-DBUILD_PURE_C_MODE=ON
```
然后在build文件夹里编译生成visual studio项目即可

# WeChat MMMojo说明
微信通过mmmojo_64.dll启动其组件并与组件通信。  
mmmojo就是对谷歌mojo ipc的一层封装, 有如下几种通信方式(具体请查看mmmojo.h) :  
```
typedef enum {
  kMMNone = 0,
  kMMPush,
  kMMPullReq,
  kMMPullResp,
  kMMShared,
} MMMojoInfoMethod;
```  
在与组件通信过程中,请求操作与回复都是通过发送 **request_id + pb数据 + MMMojoInfoMethod**  
其中request_id表明了请求的操作类型(比如QRScan请求), pb数据携带操作的参数, MMMojoInfoMethod表明通信方式。  
每个XPlugin组件都有自己定义的request_id(具体请查看mmmojo_call.h), 例如:  
(request_id 没有逆向全, 大体就是这么多了)  
```
//WeChatUtility组件
enum RequestIdUtility
{
	UtilityHiPush = 10001,			//是Utility启动发送的
	UtilityInitPullReq = 10002,		//初始化请求
	UtilityInitPullResp = 10003,		//回复创建的都是Shared类型的info, 但是调用了SwapMMMojoWriteInfoCallback, 所以回调的还是Pull
	UtilityResampleImagePullReq = 10010,
	UtilityResampleImagePullResp = 10011,
	UtilityDecodeImagePullReq = 10020,
	UtilityDecodeImagePullResp = 10021,
	UtilityPicQRScanPullReq = 10030,	//10030是点击OCR时(也是打开图片时)发送的请求, 参数是图片路径
	UtilityQRScanPullReq = 10031,		//10031是截图框选时发送的请求, 参数应该是某种编码后的图片数据
	UtilityQRScanPullResp = 10032,		//这两种请求的返回ID都是10032
	UtilityTextScanPushResp = 10040		//TextScan具体在扫什么不是很清楚 可能是用来判断图片上是否有文字
};
```
pb数据就是对应的序列化后的protobuf数据(具体请查看proto文件夹下的定义文件), 例如:
```
message QRScanReqMessage {
	string origin_pic = 1;		//原始图片路径 可选
	string decode_pic = 2;		//解码后的图片路径 可选
	int32 unknown_0 = 3;		//未知 可选 一般为0
	bytes encode_pic_data = 4;	//某种编码后的图片数据 可选
	int32 encode_pic_x = 5;		//编码图片长	可选
	int32 encode_pic_y = 6;		//编码图片宽 可选
	int32 unknown_1 = 7;		//未知 一般为1
	int32 text_scan_id = 8;		//可能是
	int32 unknown_3 = 9;		//未知 一般为1
}

```


