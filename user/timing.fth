( Timing tool                                 M.Hendrix 8mar93 )

include utils.fs

0 VALUE ^&*    \ can not be private

HIDE

   : C+! ( char c-addr -- ) dup >R  c@ +  R> ! ;

   CREATE nowbuf  0d257 ALLOT

   : clear.NB     0 nowbuf c! ;     clear.NB

   : c>NOW        nowbuf count + c!             \ char --
                  nowbuf 1 over c+!
                  c@ 0d254 >= ABORT" NOW buffer overflow" ;

   : $>NOW        0 ?DO  c@+ c>NOW  LOOP drop ; \ c-addr u --

   \ forget the temporary definition.

   : FORGET-TEMP  ^&*  HERE swap - ALLOT ;

   VARIABLE diff0

   : TIMER-STOP   get-msecs elapsed-timer @ -   \ --
                  diff0 ! ;
   : READ-TIMER   diff0 @ ;                     \ -- ms

   \ format the elapsed time : /temps  in microseconds.

   0 VALUE /checks

   : .RESULT      read-timer 0d1000 /checks */mod  \ --
                  base @ >R decimal
                     0 .r [char] . emit
                     0d1000 /checks */ .d ." microseconds / iteration."
                  R> base ! ;

   \ the string to be timed MAY NOT HAVE any stack effects.

SET-CURRENT

   : CHECK-OUT    0d10 TO /checks
                  BEGIN ^&* EXECUTE
                        ?stack
                        read-time  0d1000 u<
                  WHILE /checks 0d10 * TO /checks
                  REPEAT
                  .result  forget-temp ;

   : .TIME"       clear.NB
                  s" :noname  timer-reset  /checks 0 DO " $>now
                  [char] " word count $>now
                  s" loop timer-stop ; TO ^&*  check-out " $>now
                  nowbuf count  EVALUATE ;

PREVIOUS
