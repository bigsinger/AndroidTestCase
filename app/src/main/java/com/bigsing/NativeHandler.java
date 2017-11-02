package com.bigsing;

import android.content.Context;
import android.util.Log;

/**
 * Created by sing on 2017/7/4.
 */

public class NativeHandler {
    public final static String TAG = "NativeHandler";


    private int test(){
        Log.e(TAG, "test");
        return 1001;
    }

    native static public String getString(Context context, int cmdId, String paramStr);
    native static public String getInfo(Context context, int cmdId, String paramStr);
}
