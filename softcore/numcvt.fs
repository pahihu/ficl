.( loading NUMBER CONVERSION ) cr

ANEW -numcvt

\ number conversion
DECIMAL

VARIABLE DPL
: ?SEP ( c -- f ) \ separator?
   [char] . over = IF  drop TRUE  ELSE
   [char] / over = IF  drop TRUE  ELSE
   [char] : over = IF  drop TRUE  ELSE
   [char] ,      =
   THEN  THEN  THEN ;
   
: NUMBER? ( ca u -- ud f ) \ cvt string to number, leave t if number
   dup >r  0 DPL !  0 0 2swap
   BEGIN  >number dup
   WHILE  over c@
      ?sep IF    dup dpl !  1 /string  
           ELSE  r> drop    2drop FALSE EXIT
           THEN
   REPEAT  r> drop  2drop TRUE ;

: BASE# ( n "number" -- n ) \ cvt next word in given base
   base @ >r
      base !  bl word count
      number? 0= IF  abort  THEN
      dpl @   0= IF  drop   THEN
      state @ IF
         dpl @ IF    postpone 2literal
               ELSE  postpone  literal
               THEN
      THEN
   r> base ! ;
   
: D# ( "number" -- n ) 10 base# ; IMMEDIATE
: H# ( "number" -- n ) 16 base# ; IMMEDIATE
