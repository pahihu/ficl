\ Mentink-benchmark
INCLUDE utils.fs

20000 CONSTANT /iter

: inner ( -- ) /iter 0 DO LOOP ;
: mentink ( -- ) /iter 0 DO inner LOOP ;

: bench ( -- )
		counter  mentink  timer ;
