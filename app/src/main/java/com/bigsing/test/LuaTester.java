package com.bigsing.test;

import android.content.Context;
import android.content.SharedPreferences;

import com.bigsing.util.Constant;
import com.bigsing.util.Utils;

/**
 * Created by sing on 2017/12/18.
 */

public class LuaTester {
    //从assets目录中释放lua库到app_lua目录下
    public static boolean initLua(Context context){
        SharedPreferences sp = context.getSharedPreferences(Constant.TAG, Context.MODE_WORLD_READABLE | Context.MODE_WORLD_WRITEABLE);
        boolean inited = sp.getBoolean("luainited", false);
        if (inited == false) {
            String luaDir = context.getDir("lua", Context.MODE_WORLD_READABLE).getAbsolutePath();
            Utils.copyFilesFassets(App.getContext(), "lua", luaDir);

            SharedPreferences.Editor editor = sp.edit();
            editor.putBoolean("luainited", true);
            editor.commit();
        }

        return true;
    }
}
