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

: TNEGATE ( t .. - -t . . )
\G Negate a triple number.
   >R  2DUP OR DUP IF DROP  DNEGATE 1  THEN
   R> +  NEGATE ;

: T* ( d . n -- t . . )
\G Multiply a double number by a single number to get a
\G triple number.
                        ( d0 d1 n)
   2DUP XOR >R          ( R: sign)
   >R DABS R> ABS
   2>R                  ( d0)( R: sign d1 n)
   R@ UM* 0             ( t0 d1 0)
   2R> UM*              ( t0 d1 0 d1*n .)( R: sign) 
   D+                   ( t0 t1 t2)
   R> 0< IF TNEGATE THEN ;

: T/ ( t . . u -- d . )
\G Divide a triple number by an unsigned number to get a
\G double number.
                        ( t0 t1 t2 u)
   OVER >R >R           ( t0 t1 t2)( R: t2 u)
   DUP 0< IF TNEGATE THEN
   R@ UM/MOD            ( t0 rem d1)
   ROT ROT              ( d1 t0 rem)
   R> UM/MOD            ( d1 rem' d0)( R: t2)
   NIP SWAP             ( d0 d1)
   R> 0< IF DNEGATE THEN ;

: M*/ ( d . n u -- d . )
\G Multiply <d> by <n> to triple result; divide by <u>
\G to double result.
   >R T*  R> T/ ;

: CHOOSE ( u -- 0..u-1 )
\G Return random integer between u and u-1.
   RANDOM UM* NIP ;

: RANDOMIZE ( -- )
\G Initialize random number generator.
   TIME&DATE 5 0 DO 65599 * + LOOP
   SEED-RANDOM ;


\ vim:ts=3:sw=3:et
