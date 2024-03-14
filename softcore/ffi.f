( Native C calls --- 14mar2024ap )

HIDE

VARIABLE LIBS  0 LIBS !

: LINK ( ptr)
   HERE OVER @ , SWAP ! ;

: (LIBRARY) ( ca u)
   (DLOPEN) LIBS LINK , ;

: MY-DLSYM ( ca u hnd - a)
   ." hnd=" DUP .x >R
   2DUP TYPE R> (DLSYM)
   ." dlsym=" DUP . ;

: RESOLVE ( ca u - a)
   LIBS
   BEGIN @ DUP WHILE
      >R 2DUP  R@ CELL+ @ (DLSYM)
      DUP
      IF   R> DROP >R 2DROP R> EXIT
      ELSE DROP
      THEN
      R>
   REPEAT  DROP TYPE ." undefined" ABORT ;

0 0 (LIBRARY)

SET-CURRENT

: FUNCTION: ( n "name")
   PEEK-WORD RESOLVE
   CREATE , ,
   DOES>  2@ (C-CALL) ;

: LIBRARY ( "name")
   PARSE-WORD (LIBRARY) ;

: +NULL ( ca)
   COUNT CHARS +  0 SWAP C! ;

: ZSTR ( ca u - za)
   PAD PLACE  PAD +NULL  PAD CHAR+ ;

PREVIOUS
