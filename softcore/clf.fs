( comp.lang.forth repository --- 21sep18ap )

: OPTION ( ff "name" -- )
\G Create option flag, which turns a line into comment.
   CREATE , IMMEDIATE DOES> @ 0= IF  POSTPONE \  THEN ;

0 [IF]

FALSE OPTION PC:
TRUE  OPTION BOARD:
TRUE  OPTION DBG:

PC: DBG: INCLUDE file1
BOARD: DBG: INCLUDE file2

[THEN]
