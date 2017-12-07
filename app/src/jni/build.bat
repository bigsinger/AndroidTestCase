set dir=%~dp0
set modulename=hooktest
set modulefile=..\libs\armeabi\lib%modulename%.so

cd /d %dir%

call ./ndk.bat

if exist %modulefile% ( 
	copy %modulefile% ..\main\jniLibs\armeabi\lib%modulename%.so 
	call ./debug.bat com.bigsing.xtool
)


