package com.bigsing.util;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import com.bigsing.test.App;
import com.bigsing.test.BuildConfig;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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

    /**
     * 复制assets目录下的文件到指定目录下
     *
     * @param srcFileName
     * @param dstDir
     * @param context
     * @return
     */
    public static Boolean copyAssetsFileToDir(String srcFileName, String dstDir, Context context) {
        byte[] array = new byte[4096];
        File fileDest = new File(dstDir, srcFileName);
        if (fileDest.exists()) {
            fileDest.delete();
        }
        try {
            InputStream open = context.getResources().getAssets().open(srcFileName);
            FileOutputStream fileOutputStream = new FileOutputStream(fileDest);
            while (true) {
                int read = open.read(array);
                if (read == -1) {
                    break;
                }
                fileOutputStream.write(array, 0, read);
            }
            fileOutputStream.flush();
            fileOutputStream.close();
            fileDest.setReadable(true);
            fileDest.setWritable(true);
            fileDest.setExecutable(true);
            return true;
        } catch (Exception ex) {
            ex.printStackTrace();
            return false;
        }
    }

    //读取asset文件

    public static byte[] readAsset(Context context, String name) throws IOException {
        AssetManager am = context.getAssets();
        InputStream is = am.open(name);
        byte[] ret = readAll(is);
        is.close();
        //am.close();
        return ret;
    }

    private static byte[] readAll(InputStream input) throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
        byte[] buffer = new byte[4096];
        int n = 0;
        while (-1 != (n = input.read(buffer))) {
            output.write(buffer, 0, n);
        }
        byte[] ret = output.toByteArray();
        output.close();
        return ret;
    }
}
