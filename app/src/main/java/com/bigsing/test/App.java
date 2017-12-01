package com.bigsing.test;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import com.bigsing.util.Constant;
import com.bigsing.util.Utils;
import com.getkeepsafe.relinker.ReLinker;


/**
 * Created by sing on 2017/10/20.
 */

public class App extends Application {
    public static Context mContext;
    private static Application mApp;
    //程序通用的配置文件
    private static SharedPreferences mSP;

    public static Application getInstance() {
        return mApp;
    }
    public static Context getContext() {
        return mApp;
    }

    public static SharedPreferences getSharedPreferences() {
        return mSP;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Utils.log("App::onCreate");
        mApp = this;
        mContext = this;
        mSP = getSharedPreferences(Constant.PACKAGE_THIS, Context.MODE_WORLD_READABLE | Context.MODE_WORLD_WRITEABLE);
        ReLinker.loadLibrary(getApplicationContext(), "substrate-dvm");
        ReLinker.loadLibrary(getApplicationContext(), "substrate");
        ReLinker.loadLibrary(getApplicationContext(), "native-lib");
    }
}
