( Double shifts --- 5sep18ap )
\ ANEW -dshift

/CELL 8 *        CONSTANT #CELLBITS
#CELLBITS 1- BIT CONSTANT HIGH-BIT

: U2/ ( u1 -- u2 )
\G Unsigned number 2/.
   1 RSHIFT ;
: D2/ ( d1 -- d2 )
\G Double number 2/.
  SWAP U2/ OVER 1 AND IF  HIGH-BIT +  THEN
  SWAP 2/ ;
: UD2/ ( ud1 -- ud2 )
\G Unsigned double number 2/.
   D2/ [ HIGH-BIT INVERT ] LITERAL AND ;
: D2* ( d1 - d2 )
\G Double number 2*.
   2*  OVER 0< ABS +  >R 2* R> ;
: DLSHIFT ( d1 n -- d2 )
\G Double number LSHIFT.
   0 ?DO  D2* LOOP ;
: DRSHIFT ( d1 n -- d2 )
\G Double number RSHIFT.
   0 ?DO UD2/ LOOP ;
: SHIFT ( n1 m -- n2 )
\G If m > 0 perform LSHIFT, else RSHIFT.
   DUP 0< IF NEGATE RSHIFT ELSE LSHIFT THEN ;
: ASHIFT ( n1 m -- n2 )
\G If m > 0 perform LSHIFT, else arithmetic right shift.
   DUP 0< IF NEGATE RSHIFTA ELSE LSHIFT THEN ;
: LROTATE ( n1 m -- n2 )
\G Rotate left number m places.
  ?DUP IF
     2DUP LSHIFT >R           \ ; hllll000
     #CELLBITS SWAP - RSHIFT  \ ; 00000hhh
     R> OR
  THEN ;
: RROTATE ( n1 m -- n2 )
\G Rotate right number m places.
   ?DUP IF
     2DUP RSHIFT >R	      \ ; 000hhhhl
     #CELLBITS SWAP - LSHIFT  \ lll00000
     R> OR
   THEN ;
: ROTATE ( n1 m -- n2 )
\G If m > 0 rotate left, else rotate right number.
   DUP 0< IF NEGATE RROTATE ELSE LROTATE THEN ;

0
[IF]

TALKING
CR
TESTING D2*
T{ 0 0 D2* -> 0 0 }T
T{ 1 0 D2* -> 2 0 }T
T{ HIGH-BIT 0 D2* -> 0 1 }T
T{ HIGH-BIT 1 D2* -> 0 3 }T

CR
TESTING UD2/
T{ 0 0 UD2/ -> 0 0 }T
T{ 1 0 UD2/ -> 0 0 }T
T{ 0 1 UD2/ -> HIGH-BIT 0 }T
T{ -1 0 UD2/ -> -1 U2/ 0 }T
T{ -1 -1 UD2/ -> -1 -1 U2/ }T

CR
TESTING D2/
T{ 0 0 D2/ -> 0 0 }T
T{ 1 0 D2/ -> 0 0 }T
T{ 0 1 D2/ -> HIGH-BIT 0 }T
T{ -1 0 D2/ -> -1 U2/ 0 }T
T{ -1 -1 D2/ -> -1 -1 }T

CR
TESTING DLSHIFT
T{  0 0           0 DLSHIFT ->     0  0 }T
T{  0 0         128 DLSHIFT ->     0  0 }T
T{  1 0           1 DLSHIFT ->     2  0 }T
T{  1 0 #CELLBITS 1- DLSHIFT -> HIGH-BIT  0 }T
T{  1 0    #CELLBITS DLSHIFT ->     0  1 }T
T{ -1 0    #CELLBITS DLSHIFT ->     0 -1 }T
T{ -1 -1   #CELLBITS DLSHIFT ->     0 -1 }T

CR
TESTING DRSHIFT
T{  0 0           0 DRSHIFT ->     0  0 }T
T{  0 0         128 DRSHIFT ->     0  0 }T
T{  1 0           1 DRSHIFT ->     0  0 }T
T{  0 1           1 DRSHIFT -> HIGH-BIT  0 }T
T{  0 HIGH-BIT #CELLBITS 1- DRSHIFT -> 0 1 }T
T{  0 HIGH-BIT #CELLBITS    DRSHIFT -> HIGH-BIT 0 }T
T{  0 -1    #CELLBITS    DRSHIFT -> -1 0 }T
T{ -1 -1    #CELLBITS    DRSHIFT -> -1 0 }T

CR
TESTING RSHIFTA
T{ 0 0 RSHIFTA -> 0 }T
T{ 12345 0 RSHIFTA -> 12345 }T
T{ -1 1 RSHIFTA -> -1 }T
T{ -2 1 RSHIFTA -> -1 }T
T{ 23 1 RSHIFTA -> 11 }T

CR
TESTING SHIFT
T{ 0 0 SHIFT -> 0 }T
T{ 1 2 SHIFT -> 4 }T
T{ 23 -1 SHIFT -> 11 }T
T{ 31 BIT BITMASK -1 SHIFT -> 30 BIT BITMASK }T

CR
TESTING LROTATE
T{ 0 0 LROTATE -> 0 }T
T{ 0 23 LROTATE -> 0 }T
T{ HIGH-BIT 1 LROTATE -> 1 }T
T{ -1 1 LROTATE -> -1 }T
T{ $0123456789ABCDEF 4 LROTATE -> $123456789ABCDEF0 }T
T{ $123456789ABCDEF0 4 LROTATE -> $23456789ABCDEF01 }T

CR
TESTING RROTATE
T{ 0 0 RROTATE -> 0 }T
T{ 0 23 RROTATE -> 0 }T
T{ HIGH-BIT 1 RROTATE -> #CELLBITS 2- BIT }T
T{ -1 1 RROTATE -> -1 }T
T{ $0123456789ABCDEF 4 RROTATE -> $F0123456789ABCDE }T
T{ $123456789ABCDEF0 4 RROTATE -> $0123456789ABCDEF }T

[THEN]
