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

print(123)

-- Java �������
local MainActivity = luajava.bindClass("com.bigsing.test.MainActivity")
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

--[[
MainActivity.testLua = function()
	return 'MainActivity.testLua Changed by Lua'
end

--]]