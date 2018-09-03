\ Skip source line --- 3sep18ap
HIDE

: EOL ( -- ca )   SOURCE CHARS + ;

: SRCSCAN ( ca1 b -- ca2 )
   BEGIN  OVER EOL     <> >R
          OVER C@ OVER <> R> AND
   WHILE  >R CHAR+ R>
   REPEAT DROP ;

SET-CURRENT

: SRCLINE ( -- ca # )
\G Returns the remaining source line.
   SOURCE DROP >IN @ CHARS + \ source pos
   DUP 10 SRCSCAN  OVER - ;

PREVIOUS

