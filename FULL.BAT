@echo off
attrib -r *.*
del *.exe >nul
del *.obj >nul
make iq.mak
pklite iq.exe

