( Mentink-benchmark                                 ap 17may16 )
\ iMac 3.0GHz Core2 results ----------------------------------

\              ms
\ clang -O2     137
\ clang -O0     725
\ iforth        820
\ gforth       1074
\ ficl         2170

\ AirBook 1.6GHz Core2 results -------------------------------

\ clang 700.1.81
\ iForth 4.0.408
\ gforth 0.7.3
\ Squeak 5.0
\ FiCL 4.0.31+

\ clang -O1    1000 100%
\ iForth        229  23%
\ clang -O0     193  19%
\ gforth-fast   173  17%
\ gforth        172  17%
\ squeak        158  16%
\ ficl-fast      86   8%
\ ficl           81   8%

( speed of iForth is 1/ 4th of opt. C )
( speed of C      is 1/ 5th of opt. C )
( speed of gforth is 1/ 6th of opt. C )
( speed of FiCL   is 1/12th of opt. C )
( speed of FiCL   is 1/ 2nd of gforth )

0 constant FiCL
0 constant gforth
1 constant iForth
0 constant PFE

iForth 0= [IF]
INCLUDE utils.fs
[THEN]

20000 CONSTANT /iter

: inner ( -- ) /iter 0 DO LOOP ;
: mentink ( -- ) /iter 0 DO inner LOOP ;

: bench ( -- )
		timer-reset  mentink  .elapsed ;
