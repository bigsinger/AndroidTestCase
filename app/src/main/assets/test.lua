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

-- Java 类的名称
local MainActivity = luajava.bindClass("com.bigsing.test.MainActivity")
-- 调用 的Java 方法名
local method = 'testLua'
-- 调用 Java 方法需要的参数
local n = 10
local args = {
     n
}
-- 调用 Java 方法
local msg = MainActivity.testLua('hello from Lua script file')
print(msg)

--[[
MainActivity.testLua = function()
	return 'MainActivity.testLua Changed by Lua'
end

--]]