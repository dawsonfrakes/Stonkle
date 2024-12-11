@echo off

where /q cl || call vcvars64.bat
cl -FeStonkle.exe^
 -nologo -std:c++14 -W4 -WX -Z7 -Oi -J -EHa- -GR- -GS- -Gs0x1000000^
 main_windows.cpp kernel32.lib user32.lib gdi32.lib ws2_32.lib dwmapi.lib winmm.lib^
 -link -incremental:no -nodefaultlib -subsystem:windows^
 -stack:0x1000000,0x1000000 -heap:0,0
