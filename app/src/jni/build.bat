set dir=%~dp0
set modulename=hooktest
set modulefile=..\libs\armeabi-v7a\lib%modulename%.so

cd /d %dir%

call ./ndk.bat

if exist %modulefile% ( 
	::copy %modulefile% ..\main\jniLibs\armeabi-v7a\lib%modulename%.so 
	call ./debug.bat com.bigsing.xtool
)


