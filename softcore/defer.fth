\ Deferred words ------------------------------------ ap 23may16
.( loading DEFER ) CR

\ ANEW <defer.fth>

: @EXECUTE ( addr -- )
\G Execute xt at addr, if not zero.
   @ ?DUP IF  EXECUTE  THEN ;

: DEFER ( <name>)
\G Define a word which does nothing. Later it could
\G be defined to execute any xt.
   CREATE ['] NOOP ,
   DOES>  @EXECUTE ;

: DEFER@ ( xt1 -- xt2 )
\G Retrieve <xt2> the word currently associated with the
\G deferred word <xt1>.
   >BODY @ ;

: DEFER! ( xt2 xt1 -- )
\G Set the word <xt1> to execute <xt2>.
   >BODY ! ;

: BEHAVIOR ( xt1 -- xt2 )
\G Synonym for DEFER@.
   DEFER@ ;

0 [IF] =========================================================

Extend deferred word. Make NONAME def calling previous
action of the deferred word, replace action with NONAME def.

========================================================= [THEN]

: +IS ( xt <name> )
\G Chain <xt> to deferred <name>.
   >R :NONAME R> COMPILE,
   ' >BODY DUP @ COMPILE,
   >R POSTPONE ;  R> ! ;

: IS ( xt <name> )
\G Order <name> to execute xt.
   STATE @
   IF   POSTPONE ['] POSTPONE DEFER!
   ELSE ' DEFER!
   THEN ; IMMEDIATE

: ACTION-OF ( <name> -- xt )
\G Return the <xt> associated with the deferred word <name>.
   STATE @
   IF   POSTPONE ['] POSTPONE DEFER@
   ELSE ' DEFER@
   THEN ; IMMEDIATE

\ tests
0 [IF]

defer alpha
alpha

: hello1 ." hello, world" ;
: hello2 ." hello, FORTH world" ;

' hello1 is alpha
alpha

: test ['] hello2 is alpha  alpha ;
test

defer beta
: x ." world" ;
: y ." hello," ;
x is beta
beta
y +is beta
beta

[THEN]
