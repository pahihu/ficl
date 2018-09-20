FLOATS? [IF]
\ ANEW -floats

: FVARIABLE ( "name" -- )   VARIABLE ;

HIDE

CREATE FTMP 0 , 0 ,

SET-CURRENT

1 FLOATS 8 =
[IF]

AKA FLOAT> FPUSHD
AKA >FLOAT FPOPD

: FPUSHS ( F: r -- ) ( -- u )
\G Move an IEEE 32bit float from the float to the parameter stack.
   FTMP SF!  FTMP Q@ ;

: FPOPS ( u -- ) ( F: -- r )
\G Move a 32bit pattern from parameter to float stack.
   FTMP Q!  FTMP SF@ ;

[ELSE]

: FPUSHD ( F: r -- ) ( -- u )
\G Move an IEEE 64bit float from the float to the parameter stack.
   FTMP DF!  FTMP 2@ ;

: FPOPD ( u -- ) ( F: -- r )
\G Move a 64bit pattern from parameter to float stack.
   FTMP 2!  FTMP DF@ ;

AKA FLOAT> FPUSHS
AKA >FLOAT FPOPS

[THEN]

: FS.P. ( F: r -- )
\G Print SFLOAT bit pattern of r in hex.
   FPUSHS H. ;

: FD.P. ( F: r -- )
\G Print bit pattern of r in hex.
   FPUSHD H. ;

 1e 0e F/ FCONSTANT +INF
-1e 0e F/ FCONSTANT -INF

3.1415926535897932384e FCONSTANT PI
PI   2.0e F/ FCONSTANT PI/2
PI   4.0e F/ FCONSTANT PI/4
PI 180.0e F/ FCONSTANT PI/180

: D>R ( deg -- rad )   PI/180 F* ;

: R>D ( rad -- deg )   PI/180 F/ ;

-WARNING
: >FLOAT ( ca u -- true | false ) ( F: -- r | )
\G Convert ca/u to FP number, return success or failure.
   ?FLOAT NEGATE ;
+WARNING

PREVIOUS

[THEN]
