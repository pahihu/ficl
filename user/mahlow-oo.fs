\ the vocabulary "infrastructure" 24/04/98

5 CONSTANT #vocstack          \ max. number of vocabularies in the search order

CREATE CONTEXT ( -- addr )    \ vocabulary "stack"
   #vocstack ALLOT 0 ,        \ alloc vocabulary "stack"
   CONTEXT #vocstack 0 FILL   \ and initialize it

: VOCABULARY ( <name> -- )
   VARIABLE
   DOES> ( -- )   CONTEXT ! ;

VOCABULARY ROOT               \ eventually it will contain FORTH at least...

VARIABLE CURRENT

: DEFINITIONS ( ps: ==> )  CONTEXT @ CURRENT ! ;

: ALSO ( -- )
   CONTEXT [ #vocstack 1- ] LITERAL + @
   ABORT" vocabulary stack full"
   CONTEXT DUP CELL+ 4 CMOVE> ;

: ONLY ( -- )
   CONTEXT [ #vocstack CELL+ ] LITERAL 0 FILL
   ROOT ALSO ;

: PREVIOUS ( -- ) CONTEXT CELL+ CONTEXT #vocstack CMOVE ;

\ the PRELUDE mechanism    24/04/98

VARIABLE PRELUDE                       \ M. Mahlow's variable

: HEADER ( <name> -- )
   PRELUDE @ ?DUP IF  ,  THEN          \ compile PRELUDE cfa when present
   build-dictionary-header
   PRELUDE @ IF set-prelude-bit THEN   \ auxiliary mark bit
   PRELUDE OFF ;

: CREATE ( <name> -- )
   HEADER 1 CELLS ALLOT  DOES> ( -- addr ) ;

VARIABLE CHECK_METHOD   CHECK_METHOD OFF  \ turns error checking on/off

: CLASS ( <name> -- )   VOCABULARY IMMEDIATE ;

CLASS a-class                          \ we need a first class definition...

: CLASS-CONTEXT? ( -- f )
   CONTEXT @ 1- @  ['] a-class @ = ;

: DOPRELUDE ( cfa -- )              \ execute prelude code
                                    \ when prelude bit is set
   test-prelude-bit                 \ this is very implementation dependent...
   IF  get-prelude-code EXECUTE
       CHECK_METHOD ON  EXIT
   THEN  DROP
   STATE @ 0=EXIT
   CHECK_METHOD @ 0=EXIT
   CONTEXT @ CURRENT @ = ?EXIT
   CLASS-CONTEXT? ABORT" not a method" ;

\ Objects and Methods      24/04/98
: \V ( -- )                         \ remove class-context from search order
   CONTEXT CELL+ @  CONTEXT ! ;

: (v>p ( -- )  CONTEXT @ 1- PRELUDE !  \V ;

: METHOD ( -- )
   STATE @ ABORT" method can not be compiled"
   ['] \V PRELUDE !
; IMMEDIATE

' NOOP PRELUDE !
: OBJECT ( -- )
   (v>p  STATE @  0§=EXIT
   PRELUDE @ ,  PRELUDE OFF  COMPILE (v>p
; IMMEDIATE

: [] ( -- )                         \ disables method-error-checking
                                    \ until a method is compiled
   CLASS-CONTEXT? 0= ABORT" no class context"
   CHECK_METHOD OFF ; IMMEDIATE

: INHERIT ( -- )                    \ definition class inherits
                                    \ context class words
   CONTEXT @ @  CURRENT @  BEGIN  DUP @ WHILE  @ REPEAT  !  \V ;

\ like MSDOS DEFINITIONS  DISK-ACCESS INHERIT

\ Class NV "Non-Volatile" in E2PROM memory   24/04/98

: EROM! ( 32b erom.addr -- )
   some complex code to store 32-bits into E2PROM ;

"shadow"-ram-address CONSTANT MEM_NV
a-quantity CONSTANT #NVMAX

CLASS NV  NV DEFINITIONS  FORTH

VARIABLE PTR   PTR OFF  \ NV-area allocation pointer

METHOD : ALLOT ( n -- )
   NV PTR +!  PTR @ #NVMAX U> ABORT" E2PROM full" ;

: CREATE ( ds: -- )( ib: name )
   NV OBJECT PTR @ CONSTANT ;

: VARIABLE ( ds: -- )( ib: name )
   NV CREATE 1 ALLOT ;

METHOD : ADDR ( nv.addr -- addr )   MEM_NV + ;

METHOD : @ ( rel.addr -- n )  MEM_NV + @ ;

METHOD : ! ( n nv.addr -- )
   DUP #NVMAX U< 0= ABORT" NV address out of range"
   2DUP NV ADDR 2DUP @ =
   IF  2DROP 2DROP  EXIT THEN    \ value at this location unchanged
       !                         \ store in "shadow" ram-area
       EROM! ;                   \ store in E2PROM

METHOD : ON ( nv.addr -- )   True SWAP NV ! ;

METHOD : OFF ( nv.addr -- )   False SWAP NV ! ;

METHOD : >MOVE ( addr nv.addr count -- )  \ move string into nv-area
   BOUNDS ?DO  DUP @ I NV !  CELL+  LOOP  DROP ;

METHOD : MOVE> ( nv.addr addr count -- )  \ move string from nv-area to memory
   ROT NV ADDR -ROT CMOVE ;

FORTH DEFINITIONS

\ Class BAT "BATtery-backed"  24/04/98

"battery"-ram-address CONSTANT RAM_BAT
end-of-battery-area   CONSTANT RAM_BUF

CLASS BAT  BAT DEFINITIONS  FORTH

VARIABLE PTR  RAM_BAT 2/ PTR !   \ BAT-area allocation pointer

METHOD : ADDR ( bat.addr -- addr )  2* ;

METHOD : ALLOT ( n -- )
   PTR @ OVER + BAT ADDR RAM_BUF
   > ABORT" BAT area full"
   PTR +! ;

: CREATE ( <name> -- b.addr )
   BAT OBJECT  PTR @  CONSTANT ;

: VARIABLE ( <name> -- b.addr )
   BAT CREATE  1 ALLOT ;

METHOD : @ ( bat.addr -- 32b )
   BAT ADDR  DUP CELL+ @ SWAP @ JOIN ;

METHOD : ! ( 32b bat.addr -- )
   BAT ADDR  >R  SPLIT  R@ !  R> CELL+ ! ;

METHOD : OFF ( b.addr -- )   False SWAP BAT ! ;
METHOD : ON ( b.addr -- )    True  SWAP BAT ! ;

METHOD : >MOVE ( addr b.addr quan -- )
   BOUNDS ?DO  COUNT I BAT !  LOOP  DROP ;

METHOD : MOVE> ( b.addr addr quan -- )
   BOUNDS ?DO  DUP BAT @  I !  1+  LOOP  DROP ;

FORTH DEFINITIONS
