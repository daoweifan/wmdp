@echo OFF
del build.log
del wmdp.axf
del wmdp.map
del wmdp.bin
del wmdp.lst
del wmdp.out
del wmdp.sim
del .sconsign.dblite
del wmconfig.pyc

scons -c

rmdir /s/q project
pause