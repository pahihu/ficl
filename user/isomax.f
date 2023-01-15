( IsoMax state machine --------------------------- 11jan2023ap )

anew -isomax

vocabulary isomax   also isomax definitions

hide

variable the-machine
variable the-state

: current-state ( -- addr )   the-machine @ ;

: empty-rule ( -- )   ;

: rules ( -- addr )   the-state @ ;

set-current

: machine ( "name" -- )
\G Define machine <name>.
   create 0 , ( current-state)
   does>  dup the-machine !
            @ dup the-state !
            @ execute ;

: on-machine ( "machine" -- )
\G Select <machine>.
   ' >body the-machine ! ;

: set-state ( state -- )
\G Set initial state of the current machine.
   current-state ! ;

: append-state ( "name" -- )
\G Define new state for current machine.
   create ['] empty-rule , ;   \ rules

: in-state ( "state" -- )
   ' >body the-state ! ;

: condition ( -- xt )
   :noname
   rules @ postpone literal postpone execute ;

: causes ( xt -- xt )
   postpone if ; immediate

: then-state ( xt "name" -- xt addr )
   ' >body ; immediate

: next-time ( xt state -- )
\G State transition to <state>.
   ( literal set-state then ; )
   postpone literal postpone set-state
   postpone then postpone ;
   rules !  ; immediate

: this-time ( xt state -- )
\G Execute the new state immediately upon entry
\G in that state.
   postpone literal postpone dup postpone set-state
   postpone @ postpone execute
   postpone then postpone ;
   rules !  ; immediate

: to-happen ( xt state -- )
\G Same as NEXT-TIME.
   postpone next-time ; immediate

: is-state? ( state -- ff )
\G Returns true if current state is <state>
   the-state @ = ;


( runtime ---------------------------------------------------- )
16 constant /machines
create machines /machines cells allot
variable #machines  0 #machines !

-1 1 rshift constant max-signed
clocks/sec 100 / constant 100hz
clocks/sec  60 / constant  60hz
variable tcfperiod  100hz tcfperiod !
variable tcfiter    -1 tcfiter !
variable tcfticks
variable tcfovflo
variable tcfalarm
variable tcfalarmvector
variable tcfmin
variable tcfmax
variable tcfavg

variable tcfrun

: installed ( xt -- )
   #machines @ /machines = abort" too much MACHINEs"
   #machines @ cells machines + !
   1 #machines +! ;

: install ( "machine" -- )
\G Install machine.
   ' installed ;

: uninstall ( -- )
\G Uninstall last INSTALLed machine.
   #machines @ 1- 0 max  #machines ! ;

: no-machines ( -- )
\G Removes all INSTALLed machines.
   0 #machines ! ;

: period ( n -- )
\G Set scheduling period.
   tcfperiod ! ;

: stop-timer ( -- )
\G Halts processing.
   false tcfrun ! ;


: perfmon ( clk -- )
\G Performance monitoring. Update performance counters.
\G Checks processing time overflow.
   tcfmin @ over min tcfmin !
   tcfmax @ over max tcfmax !
   tcfavg @ dup
   IF   over + 2/
   ELSE drop dup
   THEN
   tcfavg !
   tcfperiod @ > IF
      1 tcfovflo +!
      tcfalarm @ dup IF
         tcfovflo @ = IF
            tcfalarmvector @execute
         THEN
      ELSE  drop
      THEN
   THEN
   1 tcfticks +! ;

: isomax-start ( -- )
\G Starts IsoMax processing.
   0 tcfticks !  0 tcfovflo !
   0 tcfalarm !  0 tcfalarmvector !
   max-signed tcfmin !  0 tcfmax !  0 tcfavg !

   true tcfrun !
   BEGIN tcfrun @
   WHILE
      clock >R
      #machines @ 0 ?DO
         I cells machines + @ execute
      LOOP
      clock R> -
      \ cr ." clk=" dup .

      dup perfmon
      tcfperiod @ swap -  0 max  us

      tcfticks @ tcfiter @ = IF  false tcfrun !  then
   REPEAT ;


( EVERY n CYCLE SCHEDULE-RUNS word --------------------------- )

: every ( -- )   ;
: cycles ( n -- )   period ;
: schedule-runs ( "word" -- )
   0 #machines !  install  isomax-start ;


0 [if] ---------------------------------------------------------
macine-chain <name>
   <machine1>
   ...
   <machineN>
end-machine-chain
--------------------------------------------------------- [then]

: machine-chain ( "name" -- )   : ;
: end-machine-chain ( -- )   postpone ; ; immediate

: .perf ( -- )
   cr ." min=" tcfmin ? ." avg=" tcfavg ? ." max=" tcfmax ?
   cr ." #ticks=" tcfticks ? ." #ovflo=" tcfovflo ? ;

1 [if] ( ----------------------------------------------------- )

1000 tcfiter !

machine m0
on-machine m0
   append-state s0

variable cnt  0 cnt !

in-state   s0
condition  true
causes
then-state s0
to-happen

s0 set-state

variable grnled

MACHINE ZIPGRN
  ON-MACHINE ZIPGRN
    APPEND-STATE ZIPON
    APPEND-STATE ZIPOFF
IN-STATE ZIPON CONDITION TRUE CAUSES GRNLED OFF THEN-STATE ZIPOFF
TO-HAPPEN
IN-STATE ZIPOFF CONDITION TRUE CAUSES GRNLED ON THEN-STATE ZIPON
TO-HAPPEN
ZIPON SET-STATE

\ install zipgrn
\ 5000 period
\ isomax-start .perf
every 5000 cycles schedule-runs zipgrn

no-machines

machine-chain ch10
   m0 m0 m0 m0 m0
   m0 m0 m0 m0 m0
end-machine-chain

machine-chain ch100
   ch10 ch10 ch10 ch10 ch10
   ch10 ch10 ch10 ch10 ch10
end-machine-chain

machine-chain ch1k
   ch100 ch100 ch100 ch100 ch100
   ch100 ch100 ch100 ch100 ch100
end-machine-chain

machine-chain ch10k
   ch1k ch1k ch1k ch1k ch1k
   ch1k ch1k ch1k ch1k ch1k
end-machine-chain

[then]

previous

previous definitions
