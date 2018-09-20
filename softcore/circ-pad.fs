\ Circular PADs.
\ FiCL number conversion and EMIT clashes.

.( loading CIRCULAR PAD ) cr

\ ANEW -circ-pad

hide

128 CONSTANT #SIZE
  7 CONSTANT #SLOTS	\ number of circular PADs, should be 2^n-1
    VARIABLE #PAD	\ current PAD index

CREATE pad-area #SLOTS #SIZE * allot

set-current

: /PAD ( -- )
\G Reset PAD area.
   0 #pad ! ;

-WARNING

: PAD ( -- caddr )
\G Return current PAD address <caddr>.
   pad-area  #pad @ #SIZE * + ;
   
+WARNING

: +PAD ( -- caddr )
\G Change to next PAD address <caddr>.
   #pad @  1+ #SLOTS and  #pad !
   pad ;

: >PAD ( caddr1 u -- caddr2 u )
\G Move <caddr>/<u> to next PAD area, return new <caddr2>/<u>.
   +pad swap dup >r move  pad r> ;

previous

/PAD
