( Check redefinition --- 6sep18ap )

VARIABLE WARNING

: +WARNING ( -- )
\G Enable redefinition warnings.
   TRUE WARNING ! ;
   
: -WARNING ( -- )
\G Disable redefinition warnings.
   FALSE WARNING ! ;

: PEEKWORD ( -- cstr )
\G Get next word, don't advance >IN.
   >IN @ >R  BL WORD  R> >IN ! ;
   
: .REDEF ( cstr -- )
   WARNING @ IF
      ." reDef " DUP COUNT TYPE CR
   THEN  DROP ;
   
: (REDEF) ( "name" -- )
   PEEKWORD DUP FIND NIP ( ca 0 | ca -1/1 )
   IF  .REDEF  ELSE  DROP  THEN :
;

: : ( "name" -- )   (REDEF) ;

+WARNING