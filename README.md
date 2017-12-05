# AndroidTestCase
这是一个测试用的App，程序启动时HOOK用户类的所有函数。

## 一个BUG
AllHookInOne里有一个BUG，在dvmGetMethodParamTypes里，isArray = false;

## 返回值问题
目前返回值为object的还没解析成功，其他基本类型已解析成功，参考了：
[HookJava中返回值问题的解决和修改](https://github.com/Harold1994/program_total/blob/7659177e1d562a8396d0ee9c9eda9f36a12727e5/program_total/3%E6%B3%A8%E5%85%A5%E7%9A%84so%E6%96%87%E4%BB%B6/documents/HookJava%E4%B8%AD%E8%BF%94%E5%9B%9E%E5%80%BC%E9%97%AE%E9%A2%98%E7%9A%84%E8%A7%A3%E5%86%B3%E5%92%8C%E4%BF%AE%E6%94%B9.md)
