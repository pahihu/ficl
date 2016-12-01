# SOURCES = softcore.fr ifbrack.fr prefix.fr ficl.fr jhlocal.fr marker.fr oo.fr classes.fr string.fr ficllocal.fr fileaccess.fr
SOURCES = \
	softcore.fr \
	ifbrack.fr \
	prefix.fr \
	ficl.fr \
	marker.fr \
	fileaccess.fr \
	toolbelt.fr \
	switch.fr \
	circ-pad.fs \
	missing.fs \
	strarray.fs \
	numcvt.fs
	
#	preswoop.fr
#	swoop.fr
#	swoopex.fr

../softcore.c: makesoftcore $(SOURCES)
	./makesoftcore $(SOURCES)

makesoftcore: makesoftcore.c ../lzcompress.c ../bit.c
	cc  -o makesoftcore makesoftcore.c ../lzcompress.c ../bit.c

clean: 
	- rm ../softcore.c *.o makesoftcore

