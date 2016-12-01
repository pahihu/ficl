\ METHODS> benchmarks

\                     t0      t1      t1b
\ ficl methods        330     350     10140
\ ficl ansmethods     330     335      9620
\                                      9185 inlined
\ gforth ans           85      85       338
\                                       264 inlined
\ iforth ansmethods    24      51       132
\   access penalty             2.1x      5.5x  compared to native
\ iforth faster times  13.75x   6.56x    72.87x
\ iforth optimized     24      54        71
\   access penalty             2.1x      2.95x

\ iforth VARIABLE vs MYVAR ( DOES> ) has no penalty
\ ficl                               330 vs 350 ms ( 6% )

\ FiCL
\ simple ?DO        LOOP    58
\ simple ?DO 1 DROP LOOP   225

Integer n

VARIABLE a

\ u = 10M
: t0  ( u )  timer-reset 0 ?DO  a @     drop  LOOP .elapsed ;
: t1  ( u )  timer-reset 0 ?DO  n [get] drop  LOOP .elapsed ;
: t1b ( u )  timer-reset 0 ?DO  n  get  drop  LOOP .elapsed ;
