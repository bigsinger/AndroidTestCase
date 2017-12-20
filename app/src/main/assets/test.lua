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
local MainActivity = bindClass("com.bigsing.test.MainActivity")
Build = bindClass("android.os.Build")
LinearLayout = bindClass("android.widget.LinearLayout")
EditText = bindClass("android.widget.EditText")
Button = bindClass("android.widget.Button")
Toast = bindClass("android.widget.Toast")
android_R=bindClass("android.R")
android={R=android_R}
R = bindClass("com.bigsing.test.R")
View = bindClass("android.view.View")

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
--l = loadlayout(layout)
--ac.setContentView()
--[[this.setContentView(l)
btn.onClick = function()
	msg = 'Button Click'
	print(msg)
	Toast.makeText(this, msg, Toast.LENGTH_SHORT).show()
end--]]

btn_enumui = this.findViewById(R.id.btn_enumui)
print(btn_enumui)

--以下代码不能正常执行，设计上没考虑到
--orign_onClick = btn_enumui:onClick

--以下代码可以正常执行，但是设计上不符合Lua语法，应该设计成btn_enumui:setOnClickListener
btn_enumui.setOnClickListener(View.OnClickListener {onClick = function(v) 
		msg = 'Button Click1'
		print(msg)
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show()
		orign_onClick(v)
	end })

--以下代码可以正常执行，但是设计上不符合Lua语法，应该设计成btn_enumui:onClick
btn_enumui.onClick=function(...)
	temp = btn_enumui.getText()
	--toString尚有BUG: attempt to call a nil value (field 'toString')
	--msg = temp.toString()
	msg = tostring(temp)
	Toast.makeText(this, msg, Toast.LENGTH_SHORT).show()
	--orign_onClick(...)
end

--[[
MainActivity.testLua = function()
	return 'MainActivity.testLua Changed by Lua'
end

--]]