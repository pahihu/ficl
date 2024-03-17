@echo off

cd softcore

if "%1" == "clean" goto CLEAN

if exist makesoftcore.exe goto SKIPCL
cl /Zi /Od /DWIN32=1 /I../cinegy-pthread/include makesoftcore.c ..\lzcompress.c ..\bit.c
goto MAKESOFTCORE

:SKIPCL
echo makesoftcore.exe exists, skipping building it.

:MAKESOFTCORE
echo on
makesoftcore softcore.fr ifbrack.fr prefix.fr ficl.fr jhlocal.fr marker.fr mini-oof.fs ficllocal.fr fileaccess.fr chkredef.fs header.fs srcline.fs toolbelt.fr switch.fr clf.fs struct.fth sfcompat.fs bitwise.fs circ-pad.fs missing.fs floats.fs tester.fr dshift.fs cond.fs systack.fs strarray.fs strops.fs strconst.f numcvt.fs defer.fth timedate.fs tofcompat.fs ofcompat.fs ffi.f substitute.fs lose.fs comus.fs tbelt2k.fs words.fs
goto EXIT

:CLEAN
del *.obj
del makesoftcore.exe
del ..\softcore.c

:EXIT
