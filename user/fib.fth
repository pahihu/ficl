: fib ( n-m)   dup 2 < if exit then 1- dup recurse swap 1- recurse + ;

: bench ( n -ms)   counter swap fib drop (timer) ;

: run ( -)
   30 0
   BEGIN dup 1000 < WHILE
      drop 1+ dup bench
   REPEAT   CR ." elapsed=" . ." Fib(" . ." )"
;

run bye
