set dir=%~dp0
set package=com.bigsing.test
cd /d %dir%

adb shell am force-stop %package%
adb shell am start -n %package%/%package%.MainActivity




