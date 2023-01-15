\ Circular PADs.
\ FiCL number conversion and EMIT clashes.

.( loading CIRCULAR PAD ) CR

\ ANEW -circ-pad

S" /PAD" ENVIRONMENT? DROP CONSTANT /PAD

HIDE

  7 CONSTANT #SLOTS	   \ no. of circular buffs, should be 2^n-1
    CREATE #PAD  0 ,	   \ current PAD index

CREATE 'PAD    /PAD CHARS  #SLOTS 1+ *  ALLOT

SET-CURRENT

-?
: PAD ( -- caddr )
\G Return current PAD address <caddr>.
   /PAD CHARS  #PAD @  *  'PAD + ;
   
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
   PAD  >BUFFER ;

: >HEAP ( ca u -- addr u )
\G Move string <ca/u> to heap.
   DUP CHARS SAFE-ALLOCATE  >BUFFER ;


PREVIOUS

\ vim:ts=3:sw=3:et
