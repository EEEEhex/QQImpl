using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TencenOCR.CSharp
{
    internal class Program
    {
 

        /// <summary>
        /// 加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.
        /// </summary>
        /// <param name="mmmojo_dll_path"></param>
        /// <returns></returns>
        [DllImport("MMMojoCall_d.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static bool InitMMMojoDLLFuncs(string mmmojo_dll_path);

        /// <summary>
        /// 初始化MMMojo (包括ThreadPool等), 只需要调用一次.	
        /// </summary>
        /// <param name="mmmojo_dll_path"></param>
        /// <returns></returns>
        [DllImport("MMMojoCall_d.dll", CallingConvention = CallingConvention.Cdecl)]
        extern static bool InitMMMojoGlobal(int argc,string[] argv);


        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OCRCALLBACK(string pic_path, IntPtr data, int data_size);

        [DllImport("MMMojoCall_d.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern  void SetReadOnPush(OCRCALLBACK callback);

        //下面为正式封装的方法，SetReadOnPushW是回调方法
        //[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        //public delegate void OCRCALLBACK(string pic_path, IntPtr data, int data_size);

        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetReadOnPushW(OCRCALLBACK callback);

        static void Main(string[] args)
        {
            Console.WriteLine("请输入WeChatOCR.exe的路径，包含WeChatOCR.exe");
            var ocrPath = ""; //Console.ReadLine();
            ocrPath = "D:\\TencentOCR\\QQScreenShot\\Bin\\TencentOCR.exe";

            Console.WriteLine("请输入mmmojo.dll的路径，不包含mmmojo.dll 例如 \"D:\\\\WeChat\\\\[3.9.7.29]\" 不带双引号");
            var mmPath = ""; //Console.ReadLine();
            mmPath = "D:\\TencentOCR\\QQScreenShot";
            if (!InitMMMojoDLLFuncsW(mmPath))
            {
                Console.WriteLine("mmmojocall::InitMMMojoDLLFuncs ERR");
                return;
            }
            if (!InitMMMojoGlobalW(0, null))
            {
                Console.WriteLine("初始化MMMojo ERR");
                return;
            }
            if (!SetExePathW(ocrPath))
            {
                Console.WriteLine("初始化MMMojo ERR: "+GetLastErrStrW());
                return;
            }
            if (!SetUsrLibDirW(mmPath))
            {
                Console.WriteLine("初始化用户路径失败: " + GetLastErrStrW());
                return;
            }

            SetCallbackDataMode(true);

            var oper = new OCRCALLBACK(GetMsg);
            SetReadOnPushW(oper);

            if (!StartWeChatOCRW())
            {
                Console.WriteLine("启动Tencent OCR失败: " + GetLastErrStrW());
                return ;
            }
            while (true)
            {
                Console.WriteLine("输入图片名称，包含完整路径。");
                var input = Console.ReadLine();
                if (!string.IsNullOrWhiteSpace(input) && input == "q")
                {
                    KillWeChatOCRW();
                    ShutdownMMMojoGlobalW();
                }
                if (!string.IsNullOrWhiteSpace(input) && input != "q")
                {
                    if (!DoOCRTaskW(input))
                    {
                        Console.WriteLine("发送图片路劲失败： "+input + GetLastErrStrW());
                        return;
                    }
                }
            }            
        }

        static void GetMsg(string pic_path, IntPtr data, int data_size)
        {
           var msg = Marshal.PtrToStringAnsi(data);
            var itmes = msg.Split('\n');
            foreach (var item in itmes)
            {
                if (string.IsNullOrWhiteSpace(item))
                { continue; }
                if (item.Contains("single_str_utf8") || item.Contains("one_str_utf8"))
                {
                    string encryMsg = "";
                    var arry = item.Split('"');
                    if (arry.Last()==","||string.IsNullOrEmpty(arry.Last()))
                        encryMsg= arry[arry.Length-2];
                    else
                        encryMsg = arry.Last();
                    encryMsg = DecodeBase64("utf-8", encryMsg);
                    Console.WriteLine(encryMsg);
                }
            }
            //Marshal.FreeHGlobal(data);
            //Console.WriteLine(msg);
        }

        ///编码
        public static string EncodeBase64(string code_type, string code)
        {
            string encode = "";
            byte[] bytes = Encoding.GetEncoding(code_type).GetBytes(code);
            try
            {
                encode = Convert.ToBase64String(bytes);
            }
            catch
            {
                encode = code;
            }
            return encode;
        }
        ///解码
        public static string DecodeBase64(string code_type, string code)
        {
            string decode = "";
            byte[] bytes = Convert.FromBase64String(code);
            try
            {
                decode = Encoding.GetEncoding(code_type).GetString(bytes);
            }
            catch
            {
                decode = code;
            }
            return decode;
        }


        /// <summary>
        /// 加载mmmojo(_64).dll并获取导出函数, 只需要调用一次.
        /// </summary>
        /// <param name="mmmojo_dll_path"></param>
        /// <returns></returns>
        [DllImport(".\\MMMojoCallWrapper.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public extern static bool InitMMMojoDLLFuncsW(string mmmojo_dll_path);

        /// <summary>
        /// 设置mmmojo.dll路径，就是微信所在的路径 Such as "D:\\WeChat\\[3.9.7.29]"
        /// </summary>
        /// <param name="argc"></param>
        /// <param name="argv"></param>
        /// <returns></returns>
        //MMMojoCallWrapper.dll
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool InitMMMojoGlobalW(int argc, string[] argv);


        /// <summary>
        /// 设置WeChatOCR.exe的路径
        /// </summary>
        /// <param name="argc"></param>
        /// <param name="argv"></param>
        /// <returns></returns>
        //MMMojoCallWrapper.dll
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool SetExePathW(string exe_path);

        //SetUsrLibDirW
        /// <summary>
        /// 设置--user-lib-dir命令，即微信mmmojo.dll所在路径
        /// </summary>
        /// <param name="argc"></param>
        /// <param name="argv"></param>
        /// <returns></returns>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool SetUsrLibDirW(string usr_lib_dir);

        //SetCallbackDataMode
        /// <summary>
        /// 设置返回给用户的数据类型，ture的话为json
        /// </summary>
        /// <param name="use_json"></param>
        /// <returns></returns>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static void SetCallbackDataMode(bool use_json);

        //StartWeChatOCR
        /// <summary>
        /// 		 * @brief 启动MMMojo环境以及WeChatOCR.exe程序.
		/// @return 成功返回true
        /// </summary>
        /// <param name="usr_lib_dir"></param>
        /// <returns></returns>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool StartWeChatOCRW();

        //DoOCRTaskW
        /// <summary>
        /// 		 * @brief 发送一次OCR请求.
		 ///* @param pic_path 图片路径
        ///* @return 成功返回true
        /// </summary>
        /// <param name="pic_path"></param>
        /// <returns></returns>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool DoOCRTaskW(string pic_path);

        //KillWeChatOCRW
        /// <summary>
        /// 销毁MMMojo环境以及WeChatOCR.exe程序. 
        /// </summary>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static void KillWeChatOCRW();

        //ShutdownMMMojoGlobalW
        /// <summary>
        /// 结束全局MMMojo环境 (全局调一次).
        /// </summary>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static bool ShutdownMMMojoGlobalW();

        //GetLastErrStrW
        /// <summary>
        /// 获取最后一次报错信息.
        /// </summary>
        [DllImport("MMMojoCallWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        public extern static string GetLastErrStrW();

    }
}
