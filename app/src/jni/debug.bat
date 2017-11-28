set dir=%~dp0
set package=%1%
set modulename=test
set modulefile=..\libs\armeabi-v7a\lib%modulename%.so
set destso=/data/data/%package%/lib/lib%modulename%.so

cd /d %dir%

echo off
if exist %modulefile% ( 
	echo 1.delete old file: %destso%
	adb shell "su -c ' rm %destso%'"
	
	echo 2.push so to /data/local/tmp
	adb push %dir%%modulefile% /data/local/tmp/lib%modulename%.so
	
	echo 3.copy so to /data/data/%package%/lib
	adb shell "su -c ' cp /data/local/tmp/lib%modulename%.so /data/data/%package%/lib'"
	
	echo 4.chmod 755 so
	adb shell "su -c ' chmod 755 /data/data/%package%/lib/lib%modulename%.so'"
	echo success
	
	echo u can rm other data here...
	adb shell "su -c ' rm -r /data/data/%package%/databases'"
	
)else ( 
	echo error! file not found: %dir%%modulefile%
)
echo on