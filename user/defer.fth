\ Deferred words ------------------------------------ ap 23may16

ANEW <defer.fth>

\ Define a word which does nothing. Later it could
\ be defined to execute any xt.
: DEFER ( <name>) CREATE ['] noop , DOES> @ EXECUTE ;


\ Order `name' to execute xt.
: IS ( xt <name>)
   bl word find  0= ABORT" ?"
   >body  STATE @ IF  postpone literal postpone !
   ELSE !
   THEN ; IMMEDIATE


\ tests
1 [IF]

defer alpha
alpha

: hello1 ." hello, world" ;
: hello2 ." hello, FORTH world" ;

' hello1 is alpha
alpha

: test ['] hello2 is alpha  alpha ;
test

[THEN]
