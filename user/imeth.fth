\ Trying an ANS version of METHODS>    ap 19may16

\ ANEW <ansmethods.fth>

\ HIDE

   VARIABLE last-obj
   0 last-obj !

   \ Creates an immediate word, stores the vt in the 1st cell.
   \ When the word called saves the obj address in last-obj
   \ for further reference, and compiles the address if compiling.
   : MAKE-TYPE ( vt <name> -- )
      CREATE , IMMEDIATE
      DOES>  cell+  
             STATE @ IF
               dup last-obj !
               postpone literal
             THEN ;

   \ Gets the idx-th XT from obj.
Gforth [IF]

   : GET-XT ( obj idx -- obj xt )
      over
      [ -1 cells ] literal + \ cell-
      @
      swap cells +           \ []cell
      @ ;

[ELSE]

   : GET-XT ( obj idx -- obj xt ) over  cell- @  []cell @ ;

[THEN]

   \ Compile the idx-th method inline.
   \ last-obj contains the address of the last referenced obj.
   : COMPILE-METHOD ( idx obj -- )
      ?dup IF   swap get-xt compile,  drop  0 last-obj !
           ELSE drop ." no object" ABORT
           THEN ;

\ SET-CURRENT

   \ Define a virtual table.
   : TYPE: ( <name> -- ) CREATE  DOES>  make-type ;

   : RUNS ( <name> -- ) ' , ;

   \ late binding
Gforth [IF]

   : METHOD1: ( idx <name> -- )
      CREATE cells ,
      DOES>  @ ( obj idx )
             over [ -1 cells ] literal +  \ cell-
             @
             swap +
             @ execute ;

   : (METHOD:) ( obj idx -- )
             over [ -1 cells ] literal +  \ cell-
             @
             swap +
             @ execute ;

   : METHOD: ( idx <name> -- )
      cells postpone : postpone literal postpone (method:) postpone ; ;

[ELSE]

   : METHOD: ( idx <name> -- )
      CREATE ,
      DOES>  @ ( obj idx )
             over cell- @  \ get vt
             []cell        \ get addr of XT
             perform ;     \ execute

[THEN]

   \ early binding
   : [METHOD]: ( idx <name> -- )
      CREATE , IMMEDIATE
      DOES>  @ last-obj @ ( idx obj )
             compile-method ;

\ PREVIOUS


\ tests

1 [IF]

: C? ( ca -- ) C@ . ;

: 2? ( da -- ) 2@ D. ;

0 METHOD: GET     0 [METHOD]: [GET]
1 METHOD: PUT     1 [METHOD]: [PUT]
2 METHOD: VUE     2 [METHOD]: [VUE]

TYPE: MAKE-INTEGER
   RUNS @
   RUNS !
   RUNS ?

TYPE: MAKE-HALF
   RUNS C@
   RUNS C!
   RUNS C?

TYPE: MAKE-DOUBLE
   RUNS 2@
   RUNS 2!
   RUNS 2?

: HALF    ( -- ) make-half 0 c, ;
: INTEGER ( -- ) make-integer 0 , ;
: DOUBLE  ( -- ) make-double 0 , 0 , ;

Half b
Double d
Integer n
n get .
42 n put
n vue

: test-get n [get] . ;
: test-put 31415 n [put] ;
: test-vue n [vue] ;

variable a
: myvar ( <name> -- ) CREATE 0 ,  DOES> ;
myvar aa

\ u = 10M
: t0  ( u )  timer-reset 0 ?DO   a @    drop  LOOP .elapsed ;  \    736ms
: t0b ( u )  timer-reset 0 ?DO  aa @    drop  LOOP .elapsed ;  \    736ms
: t1  ( u )  timer-reset 0 ?DO  n [get] drop  LOOP .elapsed ;  \    694ms
: t1b ( u )  timer-reset 0 ?DO  n  get  drop  LOOP .elapsed ;  \ 20,569ms

[THEN]

