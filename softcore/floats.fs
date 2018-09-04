ANEW -floats

: FVARIABLE ( "name" -- )   VARIABLE ;

hide

FVARIABLE FTMP

set-current

: FPUSHD ( F: r -- ) ( -- u )
\G Move an IEEE 64bit float from the float to the parameter stack.
   FTMP F!  FTMP @ ;

: FPOPD ( u -- ) ( F: -- r )
\G Move a 64bit pattern from parameter to float stack.
   FTMP !  FTMP F@ ;

: FPUSHS ( F: r -- ) ( -- u )
\G Move an IEEE 32bit float from the float to the parameter stack.
   FTMP SF!  FTMP Q@ ;

: FPOPS ( u -- ) ( F: -- r )
\G Move a 32bit pattern from parameter to float stack.
   FTMP Q!  FTMP SF@ ;

: FS.P. ( F: r -- )
\G Print SFLOAT bit pattern of r in hex.
   FPUSHS H. ;

: FD.P. ( F: r -- )
\G Print bit pattern of r in hex.
   FPUSHD H. ;

 1e 0e F/ FCONSTANT +INF
-1e 0e F/ FCONSTANT -INF

previous

