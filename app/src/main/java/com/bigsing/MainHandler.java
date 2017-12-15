package com.bigsing;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.Toast;

import java.io.Serializable;

/**
 * Created by sing on 2017/12/15.
 */

public class MainHandler extends Handler {
    private final static String DATA = "data";

    private final static String NAME = "name";

    private boolean mDebug = true;

    //显示toast
    public void showToast(String text) {
//        long now = System.currentTimeMillis();
//        if (toast == null || now - lastShow > 1000) {
//            toastbuilder.setLength(0);
//            toast = Toast.makeText(this, text, Toast.LENGTH_LONG);
//            toastbuilder.append(text);
//        } else {
//            toastbuilder.append("\n");
//            toastbuilder.append(text);
//            toast.setText(toastbuilder.toString());
//            toast.setDuration(Toast.LENGTH_LONG);
//        }
//        lastShow = now;
//        toast.show();
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);
        switch (msg.what) {
            case 0: {
                String data = msg.getData().getString(DATA);
                if (mDebug)
                    showToast(data);
            }
            break;
            case 1: {
                Bundle data = msg.getData();
                //setField(data.getString(DATA), ((Object[]) data.getSerializable("args"))[0]);
            }
            break;
            case 2: {
                String src = msg.getData().getString(DATA);
                //runFunc(src);
            }
            break;
            case 3: {
                String src = msg.getData().getString(DATA);
                Serializable args = msg.getData().getSerializable("args");
                //runFunc(src, (Object[]) args);
            }
        }
    }
}
