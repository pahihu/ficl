\ Utilities
\ 29apr2015

forth definitions decimal

0 constant FiCL
1 constant Gforth
0 constant PFE

FiCL [IF]
: counter ( -- u ) \ current msec timer
		get-msecs ;
[THEN]

Gforth [IF]
: counter ( -- u )
  cputime d+ 1000 sm/rem swap drop ;
[THEN]

PFE [IF]
: counter ( -- u )
  clock 1000 CLK_TCK */ ;
[THEN]

: timer ( u -- ) \ display elapsed msecs
		counter swap - u. ." ms ";

variable elapsed-timer

: timer-reset ( -- )
		counter elapsed-timer ! ;

: .elapsed ( -- )
		elapsed-timer @ timer ;

: (.base) ( n base -- ) \ display n in base
		base @ 
			swap base ! swap .
		base ! ;

: .b ( n -- )  2 (.base) ;
: .d ( n -- ) 10 (.base) ;
: .h ( n -- ) 16 (.base) ;
