.( loading NUMBER CONVERSION ) cr

ANEW -numcvt

\ number conversion
DECIMAL

\ : NUMBER? ( ca u -- d 2 | n 1 | 0 )
\    ?number dup IF  drop  2 dpl @ 0< +  THEN ;

\ NB. ?number compiles/interprets and reports a flag
: BASE# ( n "number" -- n | ) \ cvt next word in given base
   base @ >r base !
      parse-word ?number ( interp: d tf | n tf | 0 ) ( comp: ff )
   r> base !
   0= IF  abort  THEN ;

: D# ( "number" -- n ) 10 base# ; IMMEDIATE
: H# ( "number" -- n ) 16 base# ; IMMEDIATE
