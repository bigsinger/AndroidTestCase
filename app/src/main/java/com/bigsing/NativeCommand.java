package com.bigsing;

/**
 * 调用Native函数的功能使用的不同CMDID，注意数值不要随意篡改，需参考jni工程中的Constant.h文件，保持一致。
 * Created by sing on 2017/11/3.
 */

public class NativeCommand {
    public static final int CMD_GET_INFO		    =0;
    public static final int CMD_GET_TEST_STR	=1;
    public static final int CMD_GET_FILE_TEXT	=2;
    public static final int CMD_GET_MAC			=3;
    public static final int CMD_GET_PPID		    =0x991D;
}
