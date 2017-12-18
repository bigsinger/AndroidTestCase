package com.bigsing;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.androlua.LuaContext;
import com.androlua.LuaDexLoader;
import com.androlua.LuaGcable;
import com.androlua.LuaPrint;
import com.androlua.LuaThread;
import com.bigsing.util.Utils;
import com.luajava.JavaFunction;
import com.luajava.LuaException;
import com.luajava.LuaObject;
import com.luajava.LuaState;
import com.luajava.LuaStateFactory;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by sing on 2017/12/15.
 */

public class ScriptRunner  implements LuaContext {
    private final static String DATA = "data";
    private final static String NAME = "name";

    private LuaState L;

    static private HashMap<String,Object> data=new HashMap<String,Object>();
    private ArrayList<LuaGcable> gclist = new ArrayList<LuaGcable>();
    private int mWidth;
    private int mHeight;

    public Handler handler = new MainHandler();

    private String luaPath;
    public String luaDir;
    protected String localDir;

    protected String odexDir;

    protected String libDir;

    protected String luaMdDir;

    protected String luaCpath;

    protected String luaLpath;

    protected String luaExtDir;
    private LuaDexLoader mLuaDexLoader;


    private Context mContext;
    private boolean mDebug = true;

    public void init(Context context){
        initPath(context);
        try {
            initLua(context);
            mLuaDexLoader = new LuaDexLoader(this);
            mLuaDexLoader.loadLibs();
        } catch (Exception e) {
        }
        //MultiDex.installLibs(this);
    }

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
            //todo 需要修改完善luajava.c的checkError函数（line:190）把异常的完整堆栈传递进来，否则仅仅一个java.lang.NullPointerException根本不知道是哪个类
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


    public Object doAsset(String name, Object... args) {
        int ok = 0;
        try {
            byte[] bytes = Utils.readAsset(null, name);
            L.setTop(0);
            ok = L.LloadBuffer(bytes, name);

            if (ok == 0) {
                L.getGlobal("debug");
                L.getField(-1, "traceback");
                L.remove(-2);
                L.insert(-2);
                int l = args.length;
                for (int i = 0; i < l; i++) {
                    L.pushObjectValue(args[i]);
                }
                ok = L.pcall(l, 0, -2 - l);
                if (ok == 0) {
                    return L.toJavaObject(-1);
                }
            }
            throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
        } catch (Exception e) {
            //todo
            Utils.loge(e.getMessage());
            //setTitle(errorReason(ok));
            //setContentView(layout);
            sendMsg(e.getMessage());
        }

        return null;
    }

    //运行lua函数
    public Object runFunc(String funcName, Object... args) {
        if (L != null) {
            synchronized (L) {
                try {
                    L.setTop(0);
                    L.pushGlobalTable();
                    L.pushString(funcName);
                    L.rawGet(-2);
                    if (L.isFunction(-1)) {
                        L.getGlobal("debug");
                        L.getField(-1, "traceback");
                        L.remove(-2);
                        L.insert(-2);

                        int l = args.length;
                        for (int i = 0; i < l; i++) {
                            L.pushObjectValue(args[i]);
                        }

                        int ok = L.pcall(l, 1, -2 - l);
                        if (ok == 0) {
                            return L.toJavaObject(-1);
                        }
                        throw new LuaException(errorReason(ok) + ": " + L.toString(-1));
                    }
                } catch (LuaException e) {
                    sendError(funcName, e);
                }
            }
        }
        return null;
    }


    //运行lua代码
    public Object doString(String funcSrc, Object... args) {
        try {
            L.setTop(0);
            int ok = L.LloadString(funcSrc);

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
            sendMsg(e.getMessage());
        }
        return null;
    }

    //初始化lua使用的Java函数
    public boolean initLua(Context context) throws LuaException {
        this.mContext = context;
        L = LuaStateFactory.newLuaState();
        L.openLibs();
        L.pushJavaObject(this);//todo
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
        luaDir = localDir;
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

    ///////////////////////////////////////////////////////////////
    @Override
    public int getWidth() {
        return mWidth;
    }

    @Override
    public int getHeight() {
        return mHeight;
    }


    @Override
    public ArrayList<ClassLoader> getClassLoaders() {
        // TODO: Implement this method
        return mLuaDexLoader.getClassLoaders();
    }

    public HashMap<String, String> getLibrarys() {
        return mLuaDexLoader.getLibrarys();
    }

    @Override
    public void regGc(LuaGcable obj) {
        // TODO: Implement this method
        gclist.add(obj);
    }
    @Override
    public String getLuaLpath() {
        // TODO: Implement this method
        return luaLpath;
    }

    @Override
    public String getLuaCpath() {
        // TODO: Implement this method
        return luaCpath;
    }

    @Override
    public Context getContext() {
        // TODO: Implement this method
        return mContext;
    }

    @Override
    public LuaState getLuaState() {
        return L;
    }

    @Override
    public String getLuaExtDir() {
        return luaExtDir;
    }

    @Override
    public String getLuaExtDir(String name) {
        File dir = new File(luaExtDir + "/" + name);
        if (!dir.exists())
            if (!dir.mkdirs())
                return null;
        return dir.getAbsolutePath();
    }

    @Override
    public String getLuaDir() {
        return luaDir;
    }

    @Override
    public String getLuaDir(String name) {
        File dir = new File(luaDir + "/" + name);
        if (!dir.exists())
            if (!dir.mkdirs())
                return null;
        return dir.getAbsolutePath();
    }


    @Override
    public void set(String name, Object object)
    {
        // TODO: Implement this method
        data.put(name,object);
    }

    @Override
    public Map getGlobalData() {
        return data;
    }
    public Object get(String name)
    {
        // TODO: Implement this method
        return data.get(name);
    }

    //显示信息
    public void sendMsg(String msg) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, msg);
        message.setData(bundle);
        message.what = 0;
        handler.sendMessage(message);
        Log.d("lua", msg);
    }

    @Override
    public void sendError(String title, Exception msg) {
        Object ret = runFunc("onError", title, msg);
        if (ret != null && ret.getClass() == Boolean.class && (Boolean) ret)
            return;
        else
            sendMsg(title + ": " + msg.getMessage());
    }

    public void call(String func) {
        push(2, func);

    }

    public void call(String func, Object[] args) {
        if (args.length == 0)
            push(2, func);
        else
            push(3, func, args);
    }

    public void push(int what, String s) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, s);
        message.setData(bundle);
        message.what = what;

        handler.sendMessage(message);

    }
    public void push(int what, String s, Object[] args) {
        Message message = new Message();
        Bundle bundle = new Bundle();
        bundle.putString(DATA, s);
        bundle.putSerializable("args", args);
        message.setData(bundle);
        message.what = what;

        handler.sendMessage(message);

    }

    @Override
    public String getOdexDir() {
        // TODO: Implement this method
        return odexDir;
    }

    //todo 如果集成自Activity就不用单独声明了
    public Object getSystemService(String name) {
       return mContext.getSystemService(name);
    }

    public void loadResources(String path) {
        mLuaDexLoader.loadResources(path);
    }


    public AssetManager getAssets() {
        if (mLuaDexLoader != null && mLuaDexLoader.getAssets() != null)
            return mLuaDexLoader.getAssets();
        return mContext.getAssets();
    }

    public Resources getResources() {
        if (mLuaDexLoader != null && mLuaDexLoader.getResources() != null)
            return mLuaDexLoader.getResources();
        return mContext.getResources();
    }

}
