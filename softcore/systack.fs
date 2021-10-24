.( loading IFCOMPAT -- iForth ) CR

\ ANEW -systack

HIDE

DECIMAL

VARIABLE diff0

128 CELLS CONSTANT |SACK

|SACK SAFE-ALLOCATE CONSTANT SACK

VARIABLE S

SET-CURRENT

: SCLEAR ( -- )
\G Clear system stack.
   |SACK SACK +  S !
;


: >S ( n -- ) ( S: -- n)
\G Push value to system stack.
   [ /CELL NEGATE ] LITERAL S +!  S @!
;


: S@ ( -- n ) ( S: n -- n )
\G Copy value from system stack.
   S @@
;


: S> ( -- n ) ( S: n -- )
\G Pop value from system stack.
   S@  /CELL S +!
;

: SDEPTH ( -- n )
\G Depth of system stack.
   |SACK SACK +  S @ - /CELL / ;

( --- timers ------------------------------------------------- )

: TIMER-RESET ( -- )
\G Reset timer.
   GET-MSECS diff0 ! ;

: ?MS ( -- u )
\G Return milliseconds elapsed since TIMER-RESET.
   GET-MSECS diff0 @  - ;

: .ELAPSED ( -- )
\G Display milliseconds elapsed since TIMER-RESET.
   ?MS . ." ms" ;

: =:         CONSTANT ;

: 2^x ( n -- 2^n )   1 SWAP LSHIFT ;

: []CELL ( u a-addr -- c-addr )   S" SWAP CELLS+ " EVALUATE ; IMMEDIATE
: CELL[] ( a-addr u -- c-addr )   S"      CELLS+ " EVALUATE ; IMMEDIATE

PREVIOUS

SCLEAR

0
[IF]

\ TALKING
TESTING System stack.
T{ SDEPTH -> 0 }T
T{ 1 >S   ->   }T
T{ SDEPTH -> 1 }T
T{   S@  ->  1 }T
T{   S>  ->  1 }T
T{ SDEPTH -> 0 }T
T{ 1 >S 2 >S  ->  }T
T{ SDEPTH -> 2 }T
T{   S@  ->  2 }T
T{ SDEPTH -> 2 }T
T{   S>  ->  2 }T
T{ SDEPTH -> 1 }T
T{   S>  ->  1 }T
T{ SDEPTH -> 0 }T

[THEN]

