@echo off
cd /d "C:\Users\gurka\OneDrive\Desktop\MessagingApp\MessagingApp\x64\Debug"

echo [INFO] Server başlatılıyor...
start "" cmd /k "server.exe"

echo [INFO] 1. Client başlatılıyor...
start "" cmd /k "client.exe"

echo [INFO] 2. Client başlatılıyor...
start "" cmd /k "client.exe"

echo [INFO] 3. Client başlatılıyor...
start "" cmd /k "client.exe"

exit
