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

-- Java �������
local MainActivity = luajava.bindClass("com.bigsing.test.MainActivity")
print(MainActivity)
Build = bindClass("android.os.Build")
print(Build)

-- ���� ��Java ������
local method = 'testLua'
-- ���� Java ������Ҫ�Ĳ���
local n = 10
local args = {
     n
}
-- ���� Java ����
local msg = MainActivity.testLua('hello from Lua script file')
print(msg)
print(activity)
ac = luajava.newInstance("com.androlua.LuaActivity")
print(ac)
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
		text = '��ť';
		layout_width = "fill";
	};
};

local ver = Build.VERSION.SDK_INT
print(ver)
--ac.setContentView(loadlayout(layout))
--this.setContentView(loadlayout(layout))
--btn.onClick = function() print('Button Click') end
--[[
MainActivity.testLua = function()
	return 'MainActivity.testLua Changed by Lua'
end

--]]