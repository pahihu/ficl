SOURCES  = softcore.fr ifbrack.fr prefix.fr ficl.fr jhlocal.fr marker.fr
# SOURCES += oo.fr classes.fr string.fr
# SOURCES += preswoop.fr swoop.fr swoopex.fr
SOURCES += mini-oof.fs
SOURCES += ficllocal.fr fileaccess.fr
SOURCES += chkredef.fs header.fs
SOURCES += srcline.fs tester.fr toolbelt.fr switch.fr struct.fth sfcompat.fs
SOURCES += bitwise.fs circ-pad.fs missing.fs floats.fs dshift.fs
SOURCES += cond.fs systack.fs strarray.fs strops.fs strconst.f
SOURCES += numcvt.fs defer.fth timedate.fs tofcompat.fs
SOURCES += words.fs lose.fs clf.fs comus.fs
	

../softcore.c: makesoftcore $(SOURCES)
	./makesoftcore $(SOURCES)

makesoftcore: makesoftcore.c ../lzcompress.c ../bit.c
	$(CC) -m64 -Ddarwin -o makesoftcore makesoftcore.c ../lzcompress.c ../bit.c

clean: 
	- rm ../softcore.c *.o makesoftcore

