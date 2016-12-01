\ colorForth
: com invert ;
: nop ;
: or xor ;
: push postpone >r ; immediate
: pop postpone r> ; immediate
: for postpone 1+ 1 postpone literal postpone do ; immediate
: next postpone loop ; immediate
: if postpone dup postpone if ; immediate
: end postpone again ; immediate

: time clock ;
: h.n ( n w)
	base @ push hex
	push s>d <# pop for # next #> type
	pop base ! ;
: h. ( n) 8 h.n ;

: counter ( -- u )   clock ;
: timer ( u -- )   clock swap - . ." ms" ;
: .elapsed ( str -- )   0 word count  clock push  evaluate  pop timer ;

\ Bernie Mentink
\ FiCL 			2907	  34.4MHz
\ GForth 0.7.2	 452
\ SwiftForth	 358	 279.3MHz
\ GCC 3.4		  95	1052.6MHz
: inner ( -- ) 10000 for 34 drop next ;
: bench ( -- ) 10000 for   inner next ;

\ Colors
: white 0xffffff color ;
: red 0xff0000 color ;
: green 0xff00 color ;
: blue 0xff color ;
: silver 0xbfbfbf color ;
: black 0 color ;
: screen 0 dup at 1024 768 box ;
: logo black screen 800 710 blue box 600 50 at 1024 620 red
box 200 100 at 700 500 green box ;

\ Hexagon demo
variable col	0 col !
variable del	2105376 del !

\ : line ." line " . . cr ;
: lin dup 2/ 2/ dup 2* line ;
: hex xy 7 and over 2/ for lin 7 + next over for lin next swap
2/ for -7 + lin next drop ;
: petal and col @ + 0xf8f8f8 and color 100 hex ;
: +del del @ nop petal ;
: -del del @ 0xf8f8f8 or 0x80808 + ;
: rose 0 +del -176 -200 +at 0xf80000 -del petal 352 -200 +at
0xf80000 +del -264 -349 +at 0xf800 -del petal 176 -200 +at 0xf8 +del
-176 98 +at 0xf8 -del petal 176 -200 +at 0xf800 +del ;
: show black screen 512 282 at rose ;

: gfx ( xt -- )
	1024 768 gw-open
	' execute
	gw-wait-key  key drop
	gw-close ;

\ : .pixel ." y = " . ." x = " . ;
: bar ( h x - h) over for dup i 1- set-pixel next drop gw-do-events ;
: pixel-fill ( h w) for i 1- bar next drop ;

\ color bars with increasing component values
variable shmt
variable lw		200 lw !
: color-bar ( shmt -- )
	shmt !
	256 for
		i shmt @ lshift color
		0 lw @ line
	next ;

: bars ( -- )
	0 0 at 16 color-bar
	lw @ 0 at  8 color-bar
	lw @ 2 * 0 at  0 color-bar ;

: test
	black screen
	400 300 at black 100 hex
	400 200 at red    80 hex
	500 200 at blue   50 hex ;

\ cvt to 4:5:4
: low ( n - n/256 n%256) dup 255 and push 256 / pop ;
: rgb ( rgb24 - r4 g5 b4)
	low 2/ 2/ 2/ 2/ push
	low 2/ 2/ 2/    push
	low 2/ 2/ 2/ 2/ push
	drop
	pop pop pop ;
: .rgb rgb rot . swap . . ;
