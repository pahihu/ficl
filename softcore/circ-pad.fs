\ Circular PADs.
\ FiCL number conversion and EMIT clashes.

.( loading CIRCULAR PAD ) CR

\ ANEW -circ-pad

S" /PAD" ENVIRONMENT? DROP CONSTANT /PAD

HIDE

  7 CONSTANT #SLOTS	   \ no. of circular buffs, should be 2^n-1
    CREATE #PAD  0 ,	   \ current PAD index
    CREATE #SBUF 0 ,    \ current S" buffer index

CREATE 'PAD    /PAD CHARS  #SLOTS 1+ *  ALLOT
CREATE 'SBUF   /PAD CHARS  #SLOTS 1+ *  ALLOT        ( S" buffer)

: +SBUF ( -- ca )
\G Next S" buffer.
   #SBUF @  1+ #SLOTS AND  DUP #SBUF !
   /PAD CHARS *  'SBUF + ;

SET-CURRENT

-WARNING

: PAD ( -- caddr )
\G Return current PAD address <caddr>.
   /PAD CHARS  #PAD @  *  'PAD + ;
   
+WARNING

: +PAD ( -- caddr )
\G Change to next PAD address <caddr>.
   #PAD @  1+ #SLOTS AND  #PAD !
   PAD ;

: >BUFFER ( ca u buf -- buf u )
\G Move string to buffer. Returns buffer address and string
\G length.
   SWAP 2DUP 2>R  MOVE  2R> ;

: >PAD ( caddr1 u -- caddr2 u )
\G Move <caddr>/<u> to next PAD area, return new <caddr2>/<u>.
   +PAD  >BUFFER ;

: >HEAP ( ca u -- addr u )
\G Move string <ca/u> to heap.
   DUP CHARS SAFE-ALLOCATE  >BUFFER ;


-WARNING

: S" ( "ccc" -- )
\G Return the double quote delimited string.
\G Supports unlimited number of temporary numbers.
   [CHAR] " PARSE  +SBUF >BUFFER
   STATE @
   IF   POSTPONE SLITERAL
   ELSE >HEAP
   THEN ; IMMEDIATE

+WARNING


PREVIOUS

\ vim:ts=3:sw=3:et
