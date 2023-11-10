import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

interface MMMojoCall extends Library {
    public static MMMojoCall MMMojoCallDLL = Native.load("./MMMojoCall.dll", MMMojoCall.class);

    // 声明DLL导出函数
    Pointer GetInstanceXPluginMgr(int mgr_type);

   int CallFuncXPluginMgr(Pointer class_ptr, int mgr_type, String func_name, Pointer ret_ptr, Object ... args);

   void ReleaseInstanceXPluginMgr(String mgr_ptr);

}

public class MMMojoCallUsage {
    public static void main(String[] args) {
        // 调用DLL导出函数
        Pointer cobj_ocr_mgr = MMMojoCall.MMMojoCallDLL.GetInstanceXPluginMgr(1);
        System.out.println("未完成");
    }
}
