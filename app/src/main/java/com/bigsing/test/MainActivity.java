package com.bigsing.test;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
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
import com.bigsing.util.Utils;
import com.bigsing.view.BaseActivity;

import java.io.InputStream;

public class MainActivity extends BaseActivity {
    public static final String TAG = "MainActivity";

    static {
        System.loadLibrary("test");
    }

    TextView tv_text;
    Button btn_hello;
    Button btn_enumui;
    Button btn_dbAddData;
    Button btn_dbQueryData;
    EditText etUsername;
    EditText etPwd;
    private SQLiteDatabase m_db;

    public String setActName(){
        return TAG;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
        tv_text = (TextView) findViewById(R.id.tv_text);
        btn_hello = (Button) findViewById(R.id.btn_getstr_native1);
        btn_enumui = (Button) findViewById(R.id.btn_enumui);
        btn_dbAddData = (Button) findViewById(R.id.btn_dbAddData);
        btn_dbQueryData = (Button) findViewById(R.id.btn_dbQueryData);
        etUsername = (EditText) findViewById(R.id.etUsername);
        etPwd = (EditText) findViewById(R.id.etPwd);

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
                String text = NativeHandler.getStr(getApplicationContext(), NativeCommand.CMD_GET_TEST_STR, null);
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
                values.put("name", "达芬奇密码");
                values.put("pages", 566);
                m_db.insert("Book", null, values);
            }
        });
        btn_dbQueryData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Cursor cursor = m_db.query("Book", null, null, null, null, null, null);
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        String name = cursor.getString(cursor.getColumnIndex("name"));
                        int pages = cursor.getInt(cursor.getColumnIndex("pages"));
                        Utils.logd("book name is " + name);
                        Utils.logd("book pages is " + pages);
                    }
                }
                cursor.close();

                SharedPreferences sp = getSharedPreferences("test", Context.MODE_PRIVATE);
                String val = sp.getString("strkey", "default");
                Utils.logd("get strkey , value=" + val);

                try {
                    InputStream fstm = getAssets().open("test.txt");
                    byte[] content = new byte[1];
                    fstm.read(content);
                    Utils.loge("content is " + new String(content));
                } catch (Exception e) {
                    Utils.loge("e=" + e.toString());
                }
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
