package com.bigsing.test;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import com.getkeepsafe.relinker.ReLinker;


/**
 * Created by sing on 2017/10/20.
 */

public class App extends Application {
    public static Context mContext;
    private static Application mApp;
    //程序通用的配置文件
    private static SharedPreferences mSP;

    public static Application getApplication() {
        return mApp;
    }

    public static SharedPreferences getSharedPreferences() {
        return mSP;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("MyApplication", "onCreate");
        ReLinker.loadLibrary(getApplicationContext(), "test");
    }
}
