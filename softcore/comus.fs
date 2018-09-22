.( loading COMUS -- L.Wong ) CR

: MACRO ( "name <char> ccc<char>" -- )
\G Define a macro.
   : CHAR PARSE
      POSTPONE SLITERAL  POSTPONE EVALUATE POSTPONE ;
   DIRECTIVE ;

MACRO #DO " 0 ?DO"

FLOATS? [IF]
: 1/F ( F: r1 -- r2 )   1  I/F ;
: F**2 ( F: r1 -- r2 )   FDUP  F* ;
: FNIP ( F: r1 r2 -- r2 )   FSWAP  FDROP ;
: FTUCK ( F: r1 r2 -- r2 r1 r2 )   FSWAP  FOVER ;
[THEN]

: 4DUP ( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 x3 x4 )
\G Duplicatetop 4 items.
   2OVER 2OVER ;

: ?? ( ff <word> -- )
\G Execute following word if flag is true.
   S" IF" EVALUATE
   BL WORD COUNT EVALUATE
   S" THEN" EVALUATE
; DIRECTIVE

: M- ( d1 n -- d2 )
\G Subtract <n> from <d1>.
   NEGATE  M+ ;

: M/ ( d n1 -- n2 )
\G Divide <d> by <n1>, giving signed quotient <n2>.
   SM/REM  NIP ;

: CHOOSE ( u -- 0..u-1 )
\G Return random integer between u and u-1.
   RANDOM UM* NIP ;

: RANDOMIZE ( -- )
\G Initialize random number generator.
   TIME&DATE 5 0 DO 65599 * + LOOP
   SEED-RANDOM ;


\ vim:ts=3:sw=3:et
