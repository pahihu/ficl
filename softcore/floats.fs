ANEW -floats

: FVARIABLE ( "name" -- )   VARIABLE ;

hide

FVARIABLE FTMP

set-current

: FPUSH ( F: r -- ) ( -- u )
\G Move an IEEE float from the float to the parameter stack.
   FTMP F!  FTMP @ ;

: FPOP ( u -- ) ( F: -- r )
\G Move a bit pattern from parameter to float stack.
   FTMP !  FTMP F@ ;

: F.P. ( F: r -- )
\G Print bit pattern of r in hex.
   FPUSH H. ;

 1e 0e F/ FCONSTANT +INF
-1e 0e F/ FCONSTANT -INF

previous

