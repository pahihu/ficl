( Time and Date --- 15sep2018ap )

\ ANEW -timedate

HIDE

: (DD.) ( n -- )   0 <# # # #> ;

: DD. ( n -- )   (DD.) TYPE ;

: 0.R ( n -- )   0 .R ;

SET-CURRENT

( Month names ------------------------------------------------ )

DEFER (MONTHS)

STRING-ARRAY
  $" January"   $" February" $" March"    $" April"
  $" May"       $" June"     $" July"     $" August"
  $" September" $" October"  $" November" $" December"
END-STRING-ARRAY MONTHS-LONG

STRING-ARRAY
  $" Jan" $" Feb" $" Mar" $" Apr"
  $" May" $" Jun" $" Jul" $" Aug"
  $" Sep" $" Oct" $" Nov" $" Dec"
END-STRING-ARRAY MONTHS-SHORT

' MONTHS-SHORT IS (MONTHS)


: MONTH ( #month -- ca u )   1- (MONTHS) ;

: .MONTH ( n -- )   MONTH TYPE ;


( Date formats ----------------------------------------------- )

DEFER (DATE)

: (YYYY-MM-DD) ( day mon yr -- )
   0.R '- EMIT DD. '- EMIT DD. ;

: (DD.MM.YYYY) ( day mon yr -- )
   >R >R 0.R '. EMIT R> 0.R '. EMIT R> . ;

: (MMM_DD,YYYY) ( day mon yr -- )
   >R  .MONTH SPACE  0.R  ', EMIT  R> SPACE 0.R ;

' (MMM_DD,YYYY) IS (DATE)


: .TIME ( sec min hr -- )
   DD. [CHAR] : EMIT DD. [CHAR] : EMIT DD. ;

: .DATE ( day mon yr -- )   (DATE) ;

: .NOW ( -- )   NOW .TIME ;

: .TODAY ( -- )   TODAY .DATE ;

: TIME&DATE ( -- sec min hr day mon yr )
   NOW TODAY ;

PREVIOUS
