package com.bigsing;

import android.view.View;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;

/**
 * Created by sing on 2017/4/12.
 */

public class XposedHook implements IXposedHookLoadPackage {

    /**
     * @param param
     * @throws Throwable
     */
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam param) throws Throwable {
        XposedBridge.log("[handleLoadPackage] " + param.packageName);

        if (param.packageName.equals("com.example.crash") == false) {
            return;
        }

        XC_MethodHook.Unhook unhook = findAndHookMethod("com.example.crash.MainActivity$4", param.classLoader, "onClick", View.class, new XC_MethodHook() {
            @Override
            protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                XposedBridge.log("com.example.crash.MainActivity$4: before onClick");
                param.setResult(null);
            }

            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                XposedBridge.log("com.example.crash.MainActivity$4: after onClick");
            }
        });

        if (unhook != null) {
            XposedBridge.log("com.example.crash.MainActivity$4: onClick HOOK OK!!!");
        } else {
            XposedBridge.log("com.example.crash.MainActivity$4: onClick HOOK FAILED!!!");
        }
    }

}