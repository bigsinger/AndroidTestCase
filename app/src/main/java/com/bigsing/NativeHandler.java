package com.bigsing;

import android.content.Context;
import android.util.Log;

/**
 * Created by sing on 2017/7/4.
 */

public class NativeHandler {
    public final static String TAG = "NativeHandler";

    native static public String getString(Context context, int cmdId, String paramStr);

    native static public String getStr(Context context, int cmdId, String paramStr);

    native static public int getInt(Context context, int cmdId, String paramStr);

    public static void formJni(int i, String paramStr) {
        Log.d(TAG, "formJni : " + i + " str: " + paramStr);
    }

    public void formJniAgain(int i, String paramStr) {
        Log.d(TAG, "form_JNI_Again : " + i + " str: " + paramStr);
    }

    private int test() {
        Log.e(TAG, "test");
        return 1001;
    }

    native static public Object jump(int methodId, Object... args);

}
