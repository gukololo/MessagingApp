@echo off
REM Bu dosyanın bulunduğu klasöre göre çalış
set "baseDir=%~dp0"
set "exeDir=%baseDir%x64\Debug"

cd /d "%exeDir%"

echo [INFO] Server başlatılıyor...
start "" cmd /k "server.exe"

echo [INFO] 1. Client başlatılıyor...
start "" cmd /k "client.exe"
start "" cmd /k "client.exe"
start "" cmd /k "client.exe"

exit
