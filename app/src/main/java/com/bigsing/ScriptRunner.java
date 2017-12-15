package com.bigsing;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.androlua.LuaPrint;
import com.androlua.LuaThread;
import com.bigsing.util.Utils;
import com.luajava.JavaFunction;
import com.luajava.LuaException;
import com.luajava.LuaObject;
import com.luajava.LuaState;
import com.luajava.LuaStateFactory;

import java.io.File;

/**
 * Created by sing on 2017/12/15.
 */

public class ScriptRunner   {
    private LuaState L;
    private String luaPath;
    public String luaDir;
    protected String localDir;

    protected String odexDir;

    protected String libDir;

    protected String luaMdDir;

    protected String luaCpath;

    protected String luaLpath;

    protected String luaExtDir;


    private Context mContext;
    private boolean mDebug = true;

    //运行lua脚本
    public Object doFile(String filePath) {
        return doFile(filePath, new Object[0]);
    }

    public Object doFile(String filePath, Object[] args) {
        int ok = 0;
        try {
            if (filePath.charAt(0) != '/')
                filePath = luaDir + "/" + filePath;

            L.setTop(0);
            ok = L.LloadFile(filePath);

            if (ok == 0) {
                L.getGlobal("debug");
                L.getField(-1, "traceback");
                L.remove(-2);
                L.insert(-2);
                int l = args.length;
                for (int i = 0; i < l; i++) {
                    L.pushObjectValue(args[i]);
                }
                ok = L.pcall(l, 1, -2 - l);
                if (ok == 0) {
                    return L.toJavaObject(-1);
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (LuaException e) {
//            setTitle(errorReason(ok));
//            setContentView(layout);
            Utils.loge(e.getMessage());
            String s = e.getMessage();
            String p = "android.permission.";
            int i = s.indexOf(p);
            if (i > 0) {
                i = i + p.length();
                int n = s.indexOf(".", i);
                if (n > i) {
                    String m = s.substring(i, n);
                    L.getGlobal("require");
                    L.pushString("permission");
                    L.pcall(1, 0, 0);
                    L.getGlobal("permission_info");
                    L.getField(-1, m);
                    if (L.isString(-1))
                        m = m + " (" + L.toString(-1) + ")";
                    Utils.loge("权限错误: " + m);
                    return null;
                }
            }

        }

        return null;
    }

    //初始化lua使用的Java函数
    public boolean initLua(Context context) throws LuaException {
        L = LuaStateFactory.newLuaState();
        L.openLibs();
        L.pushJavaObject(this);
        L.setGlobal("activity");
        L.getGlobal("activity");
        L.setGlobal("this");
        L.pushContext(context);
        L.getGlobal("luajava");
        L.pushString(luaExtDir);
        L.setField(-2, "luaextdir");
        L.pushString(luaDir);
        L.setField(-2, "luadir");
        L.pushString(luaPath);
        L.setField(-2, "luapath");
        L.pop(1);
        initENV();

        JavaFunction print = new LuaPrint(mContext, L);
        print.register("print");

        L.getGlobal("package");
        L.pushString(luaLpath);
        L.setField(-2, "path");
        L.pushString(luaCpath);
        L.setField(-2, "cpath");
        L.pop(1);

        JavaFunction set = new JavaFunction(L) {
            @Override
            public int execute() throws LuaException {
                LuaThread thread = (LuaThread) L.toJavaObject(2);

                thread.set(L.toString(3), L.toJavaObject(4));
                return 0;
            }
        };
        set.register("set");

        JavaFunction call = new JavaFunction(L) {
            @Override
            public int execute() throws LuaException {
                LuaThread thread = (LuaThread) L.toJavaObject(2);

                int top = L.getTop();
                if (top > 3) {
                    Object[] args = new Object[top - 3];
                    for (int i = 4; i <= top; i++) {
                        args[i - 4] = L.toJavaObject(i);
                    }
                    thread.call(L.toString(3), args);
                } else if (top == 3) {
                    thread.call(L.toString(3));
                }

                return 0;
            }

            ;
        };
        call.register("call");

        return true;
    }

    private void initENV() throws LuaException {
        if (!new File(luaDir + "/init.lua").exists())
            return;

        try {
            int ok = L.LloadFile(luaDir + "/init.lua");
            if (ok == 0) {
                L.newTable();
                LuaObject env = L.getLuaObject(-1);
                L.setUpValue(-2, 1);
                ok = L.pcall(0, 0, 0);
                if (ok == 0) {
                    LuaObject title = env.getField("appname");
//                    if (title.isString())
//                        setTitle(title.getString());

                    LuaObject debug = env.getField("debugmode");
                    if (debug.isBoolean())
                        mDebug = debug.getBoolean();

                    LuaObject theme = env.getField("theme");

//                    if (theme.isNumber())
//                        setTheme((int) theme.getInteger());
//                    else if (theme.isString())
//                        setTheme(android.R.style.class.getField(theme.getString()).getInt(null));

                    return;
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (Exception e) {
            //todo
            Utils.loge(e.getMessage());
        }
    }

    //生成错误信息
    private String errorReason(int error) {
        switch (error) {
            case 6:
                return "error error";
            case 5:
                return "GC error";
            case 4:
                return "Out of memory";
            case 3:
                return "Syntax error";
            case 2:
                return "Runtime error";
            case 1:
                return "Yield error";
        }
        return "Unknown error " + error;
    }

    private void initPath(Context context){
        luaExtDir = context.getFilesDir().getAbsolutePath() + "/lua";

        File destDir = new File(luaExtDir);
        if (!destDir.exists())
            destDir.mkdirs();

        //定义文件夹
        localDir = context.getFilesDir().getAbsolutePath();
        odexDir = context.getDir("odex", Context.MODE_PRIVATE).getAbsolutePath();
        libDir = context.getDir("lib", Context.MODE_PRIVATE).getAbsolutePath();
        luaMdDir = context.getDir("lua", Context.MODE_PRIVATE).getAbsolutePath();
        luaCpath = context.getApplicationInfo().nativeLibraryDir + "/lib?.so" + ";" + libDir + "/lib?.so";
        //luaDir = extDir;
        luaLpath = luaMdDir + "/?.lua;" + luaMdDir + "/lua/?.lua;" + luaMdDir + "/?/init.lua;";
    }
    public String getLocalDir()
    {
        // TODO: Implement this method
        return localDir;
    }


    public String getMdDir()
    {
        // TODO: Implement this method
        return luaMdDir;
    }

    public String getLuaExtDir()
    {
        // TODO: Implement this method
        return luaExtDir;
    }

    public String getLuaLpath()
    {
        // TODO: Implement this method
        return luaLpath;
    }

    public String getLuaCpath()
    {
        // TODO: Implement this method
        return luaCpath;
    }

    public Context getContext()
    {
        // TODO: Implement this method
        return mContext;
    }

    public LuaState getLuaState()
    {
        // TODO: Implement this method
        return L;
    }


}
