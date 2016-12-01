\ ESC timeout test

: TEST   \ --
   1 prepterm
   BEGIN  key?  UNTIL
   ekey dup
   0 prepterm
   cr emit space .d
;

