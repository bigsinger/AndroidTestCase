package com.androlua;

/**
 * Created by sing on 2017/12/19.
 */

//当Lua调用print时，输出信息的回调接口
public interface ILuaPrintListener {
    public void onPrint(String msg);
}
