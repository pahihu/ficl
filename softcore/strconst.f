( String constant --- ap 27jul2018 )

: PLACE ( from n to -- )
   2DUP C! CHAR+ SWAP MOVE ;

: PACK ( from n to -- to )
\G Like place but returns <to>.
   DUP >R  PLACE  R> ;

: $SAVE ( from n to -- to+1 n )
\G Place <from/n> to to, return as counted string.
   DUP >R  PLACE  R> COUNT ;

: APPEND ( from len to -- )
   2DUP 2>R  COUNT + SWAP MOVE  2R> C+! ;

: APPEND-CHAR ( c to -- )
   1 OVER C+!  COUNT 1- CHARS+ C! ;

: PRINT ( ca -- )
\G Print counted string <ca>.
   COUNT TYPE ;

: ZPRINT ( cstr -- )
\G Print C-string <cstr>.
   ZCOUNT TYPE ;

: $CONSTANT ( ca u "name" -- )
   HERE OVER ALLOT  SWAP ( ca H u)	\ allot space
   2DUP 2>R MOVE 2R>			\ save H u, move string
   CREATE , , IMMEDIATE			\ layout u ca
   DOES>  2@
	  STATE @ IF  POSTPONE 2LITERAL  THEN ;

\ vim:ts=3:sw=3:et
