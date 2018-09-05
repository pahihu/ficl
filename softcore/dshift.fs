( Double shifts --- 5sep18ap )
ANEW -dshift

1 CELLS 8 *     CONSTANT CELLBITS
CELLBITS 1- BIT CONSTANT MSBIT

: U2/ ( u1 -- u2 ) 1 RSHIFT ;
: D2/ ( d1 -- d2 )
  SWAP U2/ OVER 1 AND IF  MSBIT +  THEN
  SWAP 2/ ;
: UD2/ ( ud1 -- ud2 ) D2/ [ MSBIT INVERT ] LITERAL AND ;
: D2* ( d1 - d2 ) 2*  OVER 0< ABS +  >R 2* R> ;
: DLSHIFT ( d1 n -- d2 ) 0 ?DO  D2* LOOP ;
: DRSHIFT ( d1 n -- d2 ) 0 ?DO UD2/ LOOP ;

0
[IF]

TALKING
CR
TESTING D2*
T{ 0 0 D2* -> 0 0 }T
T{ 1 0 D2* -> 2 0 }T
T{ MSBIT 0 D2* -> 0 1 }T
T{ MSBIT 1 D2* -> 0 3 }T

CR
TESTING UD2/
T{ 0 0 UD2/ -> 0 0 }T
T{ 1 0 UD2/ -> 0 0 }T
T{ 0 1 UD2/ -> MSBIT 0 }T
T{ -1 0 UD2/ -> -1 U2/ 0 }T
T{ -1 -1 UD2/ -> -1 -1 U2/ }T

CR
TESTING D2/
T{ 0 0 D2/ -> 0 0 }T
T{ 1 0 D2/ -> 0 0 }T
T{ 0 1 D2/ -> MSBIT 0 }T
T{ -1 0 D2/ -> -1 U2/ 0 }T
T{ -1 -1 D2/ -> -1 -1 }T

CR
TESTING DLSHIFT
T{  0 0           0 DLSHIFT ->     0  0 }T
T{  0 0         128 DLSHIFT ->     0  0 }T
T{  1 0           1 DLSHIFT ->     2  0 }T
T{  1 0 CELLBITS 1- DLSHIFT -> MSBIT  0 }T
T{  1 0    CELLBITS DLSHIFT ->     0  1 }T
T{ -1 0    CELLBITS DLSHIFT ->     0 -1 }T
T{ -1 -1   CELLBITS DLSHIFT ->     0 -1 }T

CR
TESTING DRSHIFT
T{  0 0           0 DRSHIFT ->     0  0 }T
T{  0 0         128 DRSHIFT ->     0  0 }T
T{  1 0           1 DRSHIFT ->     0  0 }T
T{  0 1           1 DRSHIFT -> MSBIT  0 }T
T{  0 MSBIT CELLBITS 1- DRSHIFT -> 0 1 }T
T{  0 MSBIT CELLBITS    DRSHIFT -> MSBIT 0 }T
T{  0 -1    CELLBITS    DRSHIFT -> -1 0 }T
T{ -1 -1    CELLBITS    DRSHIFT -> -1 0 }T

[THEN]
