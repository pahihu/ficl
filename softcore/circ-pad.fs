\ Circular PADs.
\ FiCL number conversion and EMIT clashes.

.( loading CIRCULAR PAD ) cr

\ ANEW -circ-pad

S" /PAD" ENVIRONMENT? DROP CONSTANT /PAD

hide

  7 CONSTANT #SLOTS	\ number of circular PADs, should be 2^n-1
    CREATE #PAD 0 ,	\ current PAD index

CREATE pad-area #SLOTS /PAD * allot

set-current

-WARNING

: PAD ( -- caddr )
\G Return current PAD address <caddr>.
   pad-area  #pad @ /PAD * + ;
   
+WARNING

: +PAD ( -- caddr )
\G Change to next PAD address <caddr>.
   #pad @  1+ #SLOTS and  #pad !
   pad ;

: >PAD ( caddr1 u -- caddr2 u )
\G Move <caddr>/<u> to next PAD area, return new <caddr2>/<u>.
   +pad swap dup >r move  pad r> ;

previous
