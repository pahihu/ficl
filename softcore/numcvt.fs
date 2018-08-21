.( loading NUMBER CONVERSION ) cr

ANEW -numcvt

\ number conversion
DECIMAL

\ NB. ?number compiles/interprets and reports a flag
hide

: BASE# ( n "number" -- n | ) \ cvt next word in given base
   base @ >r base !
      parse-word ?number ( interp: d tf | n tf | 0 ) ( comp: ff )
   r> base !
   0= IF  abort  THEN ;

set-current

: NUMBER? ( ca # - d 2 | n 1 | 0 )
   ?NUMBER DUP IF  DROP DPL @ 0< 2+  THEN ;

: D# ( "number" -- n ) 10 base# ; IMMEDIATE
: H# ( "number" -- n ) 16 base# ; IMMEDIATE

previous
