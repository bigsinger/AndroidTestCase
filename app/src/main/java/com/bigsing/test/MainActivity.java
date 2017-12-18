package com.bigsing.test;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.bigsing.NativeCommand;
import com.bigsing.NativeHandler;
import com.bigsing.ScriptRunner;
import com.bigsing.util.Utils;
import com.bigsing.view.BaseActivity;

import java.io.File;
import java.lang.reflect.Method;

import dalvik.system.DexClassLoader;

public class MainActivity extends BaseActivity {
    public static final String TAG = "MainActivity";

    static {
        //System.loadLibrary("test");
    }

    TextView tv_text;
    Button btn_hello;
    Button btn_enumui;
    Button btn_dbAddData;
    Button btn_dbQueryData;
    EditText etUsername;
    EditText etPwd;
    private SQLiteDatabase m_db;
    private int m_nIndex = 0;

    public String setActName(){
        return TAG;
    }

    //////////////////////////////////////////////////
    //该函数并没有真正注册为NATIVE函数，而是在SO启动的时候HOOK了MainActivity的构造函数，把该函数修复为了methodJava的代码。
    native public String methodWillBeNotNative();
    public String methodJava(){
        return "methodWillBeNotNative is here m_nIndex: " + m_nIndex;
    }
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    //
    public String methodWillBeNative(int n) {
        return null;
    }
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
   static private int testA(int a, String b, Object c, double d, int[] arr, float f){
        int[]ret = (int[]) NativeHandler.Jump(100, a, b, c, d, arr, f);
        return ret[0];
    }
    private String testB(String a){
        return (String)NativeHandler.Jump(101, a);
    }
    private void testC(String a){
        NativeHandler.Jump(102, a);
    }
    static public String testLua(String s){
        Utils.logd(s);
        return s;
    }
    //////////////////////////////////////////////////

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
        initView();

        m_nIndex = 2017;
        String text = NativeHandler.getString(App.getContext(), NativeCommand.CMD_INIT, null);
        //text += "\n" + methodWillBeNotNative();
        text += "\ntestA: " + testA(10, "testA", "testA", 1.0, new int[]{1,2}, (float) 2.0);
        text += "\ntestB: " + testB("testB");
        testC("testC");
        tv_text.setText(text);

        btn_hello.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = NativeHandler.getString(getApplicationContext(), NativeCommand.CMD_GET_TEST_STR, null);
                tv_text.setText(text);
            }
        });
        findViewById(R.id.btn_getstr_native2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = NativeHandler.getStr(App.getContext(), NativeCommand.CMD_GET_TEST_STR, null);
                tv_text.setText(text);
            }
        });
        findViewById(R.id.btn_getmac).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = NativeHandler.getStr(getApplicationContext(), NativeCommand.CMD_GET_MAC, null);
                tv_text.setText(text);
            }
        });
        findViewById(R.id.btn_build_prop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = NativeHandler.getStr(getApplicationContext(), NativeCommand.CMD_GET_FILE_TEXT, "/system/build.prop");
                tv_text.setText(text);
            }
        });
        findViewById(R.id.btn_get_android_id).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text =Settings.Secure.getString(getApplicationContext().getContentResolver(), Settings.Secure.ANDROID_ID);
                tv_text.setText(text);
            }
        });
        findViewById(R.id.btn_get_IMEI).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TelephonyManager telephonyManager = (TelephonyManager) getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
                WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);

                String str = telephonyManager.getDeviceId();
                if (TextUtils.isEmpty(str)) {
                    //it works, ref: https://stackoverflow.com/questions/3802644/will-telephonymanger-getdeviceid-return-device-id-for-tablets-like-galaxy-tab
                    str = SystemProperties.get("gsm.sim.imei");
                    if (TextUtils.isEmpty(str)) {
                        str = android.os.SystemProperties.get("ro.gsm.imei");
                    }
                }
                tv_text.setText(str);
            }
        });

        findViewById(R.id.btn_lua).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ScriptRunner luaRunner = new ScriptRunner();
                try {
                    String luaDir = getDir("lua", Context.MODE_WORLD_READABLE).getAbsolutePath();
                    //Utils.copyFilesFassets(App.getContext(), "lua", luaDir);

                    String luaFile = luaDir + "/test.lua";
                    File file = new File(luaFile);
                    //if (file.exists() == false) {
                    Utils.copyAssetsFileToDir("test.lua", luaDir, MainActivity.this);
                    Utils.copyAssetsFileToDir("layout.lua", luaDir, MainActivity.this);
                   // }

                    luaRunner.init(MainActivity.this);
                    luaRunner.doFile(luaDir + "/test.lua");
                    //String text = testLua("");
                    //tv_text.setText(text);
                } catch (Exception e) {
                    e.printStackTrace();
                    Utils.loge(e.getMessage());
                }
            }
        });


        btn_enumui.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String text = "";
                ViewParent parent = view.getParent().getParent();
                if (parent instanceof FrameLayout) {
                    text = getChildrenInfo((View) parent);
                }
                tv_text.setText(text);
            }
        });

        MyDatabaseHelper dbHelper = new MyDatabaseHelper(this, "demo.db", null, 1);
        m_db = dbHelper.getWritableDatabase();
        btn_dbAddData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ContentValues values = new ContentValues();
                values.put("name", "张三abc");
                values.put("age", 20);
                m_db.insert("student", null, values);
            }
        });
        btn_dbQueryData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String text = "";
                Cursor cursor = m_db.query("student", null, null, null, null, null, null);
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        String name = cursor.getString(cursor.getColumnIndex("name"));
                        int age = cursor.getInt(cursor.getColumnIndex("age"));
                        text += "name: " + name + " age: " + age + "\n";
                    }
                }
                tv_text.setText(text);
            }
        });

        findViewById(R.id.btn_recycler_view).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, RecyclerViewActivity.class));
            }
        });
        findViewById(R.id.btn_h5).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, H5Activity.class));
            }
        });
    }

    private void initView() {
        tv_text = (TextView) findViewById(R.id.tv_text);
        btn_hello = (Button) findViewById(R.id.btn_getstr_native1);
        btn_enumui = (Button) findViewById(R.id.btn_enumui);
        btn_dbAddData = (Button) findViewById(R.id.btn_dbAddData);
        btn_dbQueryData = (Button) findViewById(R.id.btn_dbQueryData);
        etUsername = (EditText) findViewById(R.id.etUsername);
        etPwd = (EditText) findViewById(R.id.etPwd);
    }


    public String getChildrenInfo(View parent) {
        String text = parent.toString() + "\n";

        if (parent instanceof ViewGroup) {
            int nCount = ((ViewGroup) parent).getChildCount();
            for (int i = 0; i < nCount; ++i) {
                View v = ((ViewGroup) parent).getChildAt(i);
                if (v instanceof ViewGroup) {
                    text += getChildrenInfo(v) + "\n";
                } else {
                    if (v instanceof TextView) {
                        text += "\t" + v.toString() + "【" + ((TextView) v).getText() + "】\n";
                    } else {
                        text += "\t" + v.toString() + "\n";
                    }
                }
            }
        }

        return text;
    }

}
