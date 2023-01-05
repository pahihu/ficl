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

: 'FLAGS ( lfa -- caddr )   CELL+ 2 CHARS + ;
: FLAGS ( lfa -- u )   'FLAGS C@ ;
: SET-FLAGS ( or and xt -- )
   'FLAGS ( or and caddr)
   DUP >R C@  AND OR  R> C! ;

: FLAG? ( xt mask -- f )
   SWAP FLAGS  AND  0= 0= ;

SET-CURRENT

: LFA ( xt -- lfa )
\G Gives back the link field address.
   ;
: NFA ( lfa -- nfa )
\G Gives back the name field address.
   2 CELLS+ ;
: CFA ( lfa -- cfa )
\G Gives back the code field address.
   3 CELLS+ ;
: PFA ( lfa -- pfa )
\G Gives back the parameter field address.
   5 CELLS+ ;
  
: .ID ( lfa -- )
\G Print the name of the word at `lfa'.
   NFA  NFA>NAME  TYPE SPACE ;

: IMMEDIATE? ( xt -- f )
\G Gives back true if `xt' is immediate.
   ?IMMEDIATE FLAG? ;
: COMPILE-ONLY? ( xt -- f )
\G Gives back true, if `xt' is compile only.
   ?COMPILE-ONLY FLAG? ;
: SMUDGED? ( xt -- f )
\G Gives back true, if `xt' is smudged.
   ?SMUDGED FLAG? ;
: INSTRUCTION? ( xt -- f )
\G Gives back true, if `xt' is a VM instruction.
   ?INSTRUCTION FLAG? ;
: -SMUDGE ( -- )
\G Clear smudge bit on last word defined.
   0 27 LAST-WORD SET-FLAGS ;
: +SMUDGE ( xt -- )
\G Set smudge bit on last word defined.
   ?SMUDGED 27 LAST-WORD SET-FLAGS ;


PREVIOUS
