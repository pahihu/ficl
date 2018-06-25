\ Circular PADs.
\ FiCL number conversion and EMIT clashes.

.( loading CIRCULAR PAD ) cr

ANEW -circ-pad

hide

7 CONSTANT /pad-area	\ number of circular PADs
  VARIABLE #pad		\ current PAD index

CREATE pad-area /pad-area 80 * allot

set-current

: RESET-PAD ( -- ) 0 #pad ! ;

: PAD ( -- ca ) \ current PAD
	pad-area  #pad @ 80 * + ;

: +PAD ( -- ca ) \ PAD increment
	#pad @  1+ /pad-area mod  #pad !
	pad ;

: +PAD, ( ca n -- ca' n ) \ move sc to PAD increment
	+pad swap dup >r move  pad r> ;

previous

reset-pad
