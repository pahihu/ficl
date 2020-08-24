FLOATS? [IF]
\ ANEW -floats

: FVARIABLE ( "name" -- )   VARIABLE ;

HIDE

CREATE FTMP1 0 , 0 ,
CREATE FTMP2 0 , 0 ,

SET-CURRENT

1 FLOATS 8 = CONSTANT DFLOATS?
1 FLOATS 4 = CONSTANT SFLOATS?

DFLOATS? OPTION \DFLOATS
SFLOATS? OPTION \SFLOATS

DFLOATS?
[IF]

AKA FLOAT> FPUSHD
AKA >FLOAT FPOPD

: FPUSHS ( F: r -- ) ( -- u )
\G Move an IEEE 32bit float from the float to the parameter stack.
   FTMP1 SF!  FTMP1 Q@ ;

: FPOPS ( u -- ) ( F: -- r )
\G Move a 32bit pattern from parameter to float stack.
   FTMP1 Q!  FTMP1 SF@ ;

[ELSE]

: FPUSHD ( F: r -- ) ( -- u )
\G Move an IEEE 64bit float from the float to the parameter stack.
   FTMP1 DF!  FTMP1 2@ ;

: FPOPD ( u -- ) ( F: -- r )
\G Move a 64bit pattern from parameter to float stack.
   FTMP1 2!  FTMP1 DF@ ;

AKA FLOAT> FPUSHS
AKA >FLOAT FPOPS

[THEN]

: SF, ( F: r -- )   FPUSHS HERE Q!  4 ALLOT ;
: DF, ( F: r -- )   FPUSHD HERE  !  8 ALLOT ;

\DFLOATS AKA DF, F,
\SFLOATS AKA SF, F,

: SF? ( addr -- )   SF@ F. ;
: DF? ( addr -- )   DF@ F. ;

\DFLOATS AKA DF? F?
\SFLOATS AKA SF? F?

: FS.P. ( F: r -- )
\G Print SFLOAT bit pattern of r in hex.
   FPUSHS H. ;

: FD.P. ( F: r -- )
\G Print bit pattern of r in hex.
   FPUSHD H. ;

 1e 0e F/ FCONSTANT +INF
-1e 0e F/ FCONSTANT -INF

3.1415926535897932384e FCONSTANT PI
2.7182818284590452353e FCONSTANT E
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

: FCLEAR ( F: i*r -- )
\G Empty FP stack.
   FDEPTH 0 ?DO  FDROP  LOOP ;

: FINITE? ( F: r -- ) ( -- ff )
\G True if r is finite.
   FDUP FINFINITE? FNAN? OR 0= ;

: FSAME? ( F: r1 r2 -- ) ( -- ff )
\G Testing FP number equality including NaN/Inf.
   FDUP FINITE? FOVER FINITE? AND  IF F= EXIT THEN
   FTMP1 DF!  FTMP2 DF!
   FTMP1 2@   FTMP2 2@  D= ;

PREVIOUS

[THEN]

\ vim:ts=3:sw=3:et


