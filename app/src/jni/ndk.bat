set dir=%~dp0
set ndk=D:\android\sdk\ndk-bundle\ndk-build.cmd

cd /d %dir%

if exist %ndk% ( %ndk% )else ( ndk-build )

