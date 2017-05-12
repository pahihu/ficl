.( loading NUMBER CONVERSION ) cr

ANEW -numcvt

\ number conversion
DECIMAL

: BASE# ( n "number" -- n ) \ cvt next word in given base
   base @ >r
      base !  bl word count
      ?number 0= IF  abort  THEN
      state @ IF
         dpl 0< IF    postpone  literal
                ELSE  postpone 2literal
                THEN
      THEN
   r> base ! ;

: D# ( "number" -- n ) 10 base# ; IMMEDIATE
: H# ( "number" -- n ) 16 base# ; IMMEDIATE
