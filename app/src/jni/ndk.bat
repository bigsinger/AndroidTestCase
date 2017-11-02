set dir=%~dp0
set ndk=d:\Android\ndk\ndk-build.cmd

cd /d %dir%

if exist %ndk% ( %ndk% )else ( ndk-build )

