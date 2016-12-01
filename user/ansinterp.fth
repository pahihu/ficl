\ A.Haley 18may16

: NUMBER ( a n1 - n2)
   2>r 0 0 2r>  dup >r  >number  r> = ABORT" ?"  2drop ;

: COMPILING ( i*x a - j*x)
   find  ?dup 0= IF
      count number  postpone literal
   ELSE 0< IF  compile,
   ELSE        EXECUTE
   THEN THEN ;

: INTERPRETING ( i*x a - j*x)
   find  ?dup 0= IF  count number
   ELSE  drop EXECUTE  THEN ;

: INTERP-LOOP ( i*x - j*x)
   BEGIN
      bl word  dup c@ WHILE
         STATE @ IF  compiling  ELSE  interpreting  THEN
   REPEAT  drop ;
