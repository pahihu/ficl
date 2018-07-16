SOURCES  = softcore.fr ifbrack.fr prefix.fr ficl.fr jhlocal.fr marker.fr
# SOURCES += oo.fr classes.fr string.fr
# SOURCES += preswoop.fr swoop.fr swoopex.fr
SOURCES += mini-oof.fs
SOURCES += ficllocal.fr fileaccess.fr
SOURCES += tester.fr toolbelt.fr switch.fr struct.fth enum.fs timer.fs
SOURCES += circ-pad.fs missing.fs strarray.fs numcvt.fs words.fs
	

../softcore.c: makesoftcore $(SOURCES)
	./makesoftcore $(SOURCES)

makesoftcore: makesoftcore.c ../lzcompress.c ../bit.c
	$(CC) -m64 -Ddarwin -o makesoftcore makesoftcore.c ../lzcompress.c ../bit.c

clean: 
	- rm ../softcore.c *.o makesoftcore

