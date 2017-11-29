package com.bigsing.util;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import com.bigsing.test.App;
import com.bigsing.test.BuildConfig;

import java.io.File;

/**
 * Created by sing on 2017/4/19.
 */

public class Utils {
    public synchronized static void logd(String msg) {
        Log.d(Constant.TAG, msg);
    }
    public synchronized static void log(String msg) {
        Log.d(Constant.TAG, msg);
    }
    public synchronized static void loge(String msg) {
        Log.e(Constant.TAG, msg);
    }

//    public static void logd(String msg) {
//        if (BuildConfig.DEBUG) Log.d(Constant.TAG, msg);
//    }
//    public static void loge(String msg) {
//        if (BuildConfig.DEBUG) Log.e(Constant.TAG, msg);
//    }

    public static void toast(Context context, String msg) {
        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
    }

    public static void toast(int id) {
        Toast.makeText(App.mContext, App.mContext.getString(id), Toast.LENGTH_SHORT).show();
    }

    public static void toast(String msg) {
        Toast.makeText(App.mContext, msg, Toast.LENGTH_SHORT).show();
    }

    public static String getVersionInfo(Context context) {
        String versionName = null;

        try {
            // ---get the package info---
            PackageManager pm = context.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
            versionName = pi.versionName;
            //versioncode = pi.versionCode;

        } catch (Exception e) {
        }

        if (versionName == null || versionName.length() <= 0) {
            return "";
        } else {
            return " v" + versionName;
        }
    }

    public static void openUrl(Context context, String url) {
        Uri uri = Uri.parse(url);
        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
        context.startActivity(intent);
    }

    /*
    获取应用程序名
     */
    public static String getAppName(Context ctx) {
        PackageManager packageManager = null;
        ApplicationInfo applicationInfo = null;
        String appName = null;
        try {
            packageManager = ctx.getPackageManager();
            applicationInfo = packageManager.getApplicationInfo(ctx.getPackageName(), 0);
            if (applicationInfo != null) {
                appName = (String) packageManager.getApplicationLabel(applicationInfo);
            }

        } catch (Exception e) {
        }
        return appName;
    }

    /*
    应用程序版本号
     */
    public static String getAppVer(Context ctx) {
        String version = null;
        try {
            // 获取packagemanager的实例
            PackageManager packageManager = ctx.getPackageManager();
            // getPackageName()是你当前类的包名，0代表是获取版本信息
            PackageInfo pkgInfo = packageManager.getPackageInfo(ctx.getPackageName(), 0);
            version = pkgInfo.versionName;

        } catch (Exception e) {

        }
        return version;
    }

    public static String getFileName(String strFilePath) {
        if (strFilePath != null) {
            File file = new File(strFilePath);
            return file.getName();
        }
        return null;
    }

    public static String getCachePath(Context context) {
        String path = context.getApplicationInfo().dataDir + "/cache";
        return path;
    }
}
