#
# generate list of primitives
#
DAT=primitives.dat
OUT=primitives.txt
rm -f $DAT $OUT
grep ficlDictionarySet *.c | grep "\"" >$DAT
grep addPrimitive *.c | grep "\"" >>$DAT
grep PRIMDEF *.c | grep "\"" >>$DAT
cat $DAT | awk '{word=substr($3,2,length($3)-3);if(index($0,"environment"))print word,"E";else print word;}' | sort >$OUT
