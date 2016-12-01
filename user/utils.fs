\ Utilities
\ 29apr2015

decimal

: counter ( -- u ) \ current msec timer
		get-msecs ;

: timer ( u -- ) \ display elapsed msecs
		get-msecs swap - u. ." ms ";

variable elapsed-timer

: timer-reset ( -- )
		get-msecs elapsed-timer ! ;

: .elapsed ( -- )
		elapsed-timer @ timer ;

: (.base) ( n base -- ) \ display n in base
		base @ 
			swap base ! swap .
		base ! ;

: .b ( n -- )  2 (.base) ;
: .d ( n -- ) 10 (.base) ;
: .h ( n -- ) 16 (.base) ;
