( Header access --- ap 2aug2018 )

\ Arcane FiCL knowledge the XT is the LFA.

\ normal word (colon)
\  <name>--aligned--hashlink--/hash:2-flags:1-namelen:1/--/ptr-to-name/
\  46 2B ... prim/XT ... 2B

\ do-does
\ DOES>  4D ... prim/XT ...
\ the word created
\  4B--2B--<addr of does part>--<pfa>
\ the PFA contains the DOES> part address

\ Header access.  
HIDE 

: NFA>NAME ( nfa -- ca u )
   DUP @ SWAP  CELL- 3 + C@ ;

 1 CONSTANT ?IMMEDIATE
 2 CONSTANT ?COMPILE-ONLY
 4 CONSTANT ?SMUDGED
 8 CONSTANT ?OBJECT
16 CONSTANT ?INSTRUCTION

: FLAGS ( lfa -- u )   CELL+  2 CHARS + C@ ;

: FLAG? ( xt mask -- f )
   SWAP FLAGS  AND  0= 0= ;

SET-CURRENT

: LFA ( xt -- lfa ) ;
: NFA ( lfa -- nfa )   2 CELLS+ ;
: CFA ( lfa -- cfa )   3 CELLS+ ;
: PFA ( lfa -- pfa )   5 CELLS+ ;
  
: .ID ( lfa -- )   NFA  NFA>NAME  TYPE SPACE ;

: IMMEDIATE? ( xt -- f )   ?IMMEDIATE FLAG? ;
: COMPILE-ONLY? ( xt -- f )   ?COMPILE-ONLY FLAG? ;
: SMUDGED? ( xt -- f )   ?SMUDGED FLAG? ;
: INSTRUCTION? ( xt -- f )   ?INSTRUCTION FLAG? ;

PREVIOUS
