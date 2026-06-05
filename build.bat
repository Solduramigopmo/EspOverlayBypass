@echo off
set "VC_VARS=C:\Program Files\Microsoft Visual Studio\..."

call "%VC_VARS%"
cl /EHsc /O2 main.cpp /Fe:SystemUtility.exe user32.lib gdi32.lib dwmapi.lib /link /SUBSYSTEM:WINDOWS
echo Done!
