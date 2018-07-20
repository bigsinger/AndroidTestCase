package com.bigsing;

import android.content.Context;
import android.util.Log;

import com.bigsing.util.Utils;

/**
 * Created by sing on 2017/7/4.
 */

public class NativeHandler {
    public final static String TAG = "NativeHandler";

    native static public String getString(Context context, int cmdId, String paramStr);

    native static public String getStr(Context context, int cmdId, String paramStr);

    native static public int getInt(Context context, int cmdId, String paramStr);

    public static void formJni(int i, String paramStr) {
        Utils.log("NativeHandler::formJni : " + i + " str: " + paramStr);
    }

    public void formJniAgain(int i, String paramStr) {
        Utils.log("NativeHandler::form_JNI_Again : " + i + " str: " + paramStr);
    }

    private int test() {
        Utils.log("NativeHandler::test");
        return 1001;
    }

    /*
    其实也可以根据methodId来知道当前调用的函数的参数个数及返回类型，
    这里为了NDK层解析方便也显式指定一下。
    methodId: 用以区别调用的功能。
    retType： 指明返回值类型。
    argCount: 指明后面不定参数的个数。
     */
    native static public Object Jump(int methodId, int retType, int argCount, Object []args);

}
