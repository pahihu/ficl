( String constant --- ap 27jul2018 )

: PLACE ( from len to -- )
\G Place string <from/len> as counted string <to>.
   2DUP C! CHAR+ SWAP MOVE ;

: APPEND ( from len to -- )
\G Append string <from/len> to counted string <to>.
   2DUP C+!  COUNT CHARS+ OVER - SWAP MOVE ;

: APPEND-CHAR ( c to -- )
\G Append char <c> to counted string <to>.
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
