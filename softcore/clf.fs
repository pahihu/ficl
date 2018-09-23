( comp.lang.forth repository --- 21sep18ap )

: OPTION ( ff "name" -- )
\G Create option flag, which turns a line into comment. [M.Gassanenko]
   CREATE , IMMEDIATE DOES> @ 0= IF  POSTPONE \  THEN ;

/CELL 8 = OPTION \64
/CELL 4 = OPTION \32

: +STRING ( src n1 dst n2 -- dst n1+n2 )
\G Append <src/n1> to <dst/n2>, return <dst/n1+n2>. [OTA]
   2DUP 2>R  CHARS+ SWAP  DUP R> + >R  MOVE  2R> ;

: >CELLS ( #bytes -- #cells )
\G Round up <#bytes> to the corresponding <#cells>.
   [ /CELL 1- ] LITERAL +
\64 3 RSHIFT
\32 2 RSHIFT
   ;

: N>R ( xn .. x1 n -- ) ( R: -- x1 .. xn n )
\G Push n+1 elements on the return stack. [C.Haak]
   DUP
   BEGIN  DUP  WHILE
      ROT R> 2>R 1-
   REPEAT  DROP R> 2>R ; COMPILE-ONLY

: NR> ( -- xn .. x1 n ) ( R: x1 .. xn n -- )
\G Pop n+1 elements from the return stack. [C.Haak]
   2R> >R DUP
   BEGIN  DUP  WHILE
      2R> >R -ROT 1-
   REPEAT  DROP ; COMPILE-ONLY


0 [IF]

FALSE OPTION PC:
TRUE  OPTION BOARD:
TRUE  OPTION DBG:

PC: DBG: INCLUDE file1
BOARD: DBG: INCLUDE file2

[THEN]
