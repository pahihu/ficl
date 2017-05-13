\ Utilities
\ 29apr2015

forth definitions decimal

1 constant FiCL
0 constant Gforth
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

: (timer) ( u -- elapsed )
\ *G Return elapsed time since u.
   counter swap - ;

: timer ( u -- )
\ *G Display elapsed msecs.
   (timer)  u. ." ms" ;

variable diff0

: timer-reset ( -- )
\ *G Reset elapsed timer.
   counter  diff0 ! ;

: .elapsed ( -- )
\ *G Display elapsed msecs since timer-reset.
   diff0 @  timer ;

: .elapsed" ( <cmd> -- )
   timer-reset  [char] " word count  evaluate  .elapsed ;

: (.base) ( n base -- ) \ display n in base
		base @ 
			swap base ! swap .
		base ! ;

: .b ( n -- )  2 (.base) ;
: .d ( n -- ) 10 (.base) ;
: .h ( n -- ) 16 (.base) ;
