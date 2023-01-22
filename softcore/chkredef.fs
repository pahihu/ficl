( Check redefinition --- 6sep18ap )

VARIABLE WARNING
VARIABLE SUPPRESS  FALSE SUPPRESS !

: +WARNING ( -- )
\G Enable redefinition warnings.
   TRUE WARNING ! ;
   
: -WARNING ( -- )
\G Disable redefinition warnings.
   FALSE WARNING ! ;

: PEEK-WORD ( -- ca # )
\G Get next word, don't advance >IN.
   >IN @ >R  PARSE-WORD  R> >IN ! ;
   
: .REDEF ( ca # -- )
   WARNING @ SUPPRESS @ 0= AND  IF
      ." reDef " 2DUP TYPE CR
   THEN  2DROP
   FALSE SUPPRESS ! ;
   
: (REDEF) ( "name" -- )
   PEEK-WORD 2DUP SFIND NIP ( ca # 0 | ca # -1/1 )
   IF  .REDEF  ELSE  2DROP  THEN  :
;

: -? ( -- )
\G Suppress reDef warning for next definition only.
   TRUE SUPPRESS ! ;

: : ( "name" -- )   (REDEF) ;

+WARNING
