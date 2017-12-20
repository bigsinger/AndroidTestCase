require "import"
import "console"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "com.androlua.*"
import "java.io.*"
import "android.text.method.*"
import "android.net.*"
import "android.content.*"
import "android.graphics.drawable.*"

require "layout"

bindClass = luajava.bindClass

-- Java 类的名称
local MainActivity = luajava.bindClass("com.bigsing.test.MainActivity")
Build = bindClass("android.os.Build")
LinearLayout = bindClass("android.widget.LinearLayout")
EditText = bindClass("android.widget.EditText")
Button = bindClass("android.widget.Button")
Toast = bindClass("android.widget.Toast")

-- 调用 的Java 方法名
local method = 'testLua'
-- 调用 Java 方法需要的参数
local n = 10
local args = {
     n
}
-- 调用 Java 方法
local msg = MainActivity.testLua('hello from Lua script file')
ac = luajava.newInstance("com.androlua.LuaActivity")
layout={
	LinearLayout;
	orientation = "vertical";
	{
		EditText;
		id = 'edit';
		layout_width = "fill";
	};
	{
		Button;
		id = 'btn';
		text = '按钮';
		layout_width = "fill";
	};
};

local ver = Build.VERSION.SDK_INT
print(ver)
l = loadlayout(layout)
--ac.setContentView()
this.setContentView(l)
btn.onClick = function()
	msg = 'Button Click'
	print(msg)
	Toast.makeText(this, msg, Toast.LENGTH_SHORT).show()
end
--[[
MainActivity.testLua = function()
	return 'MainActivity.testLua Changed by Lua'
end

--]]