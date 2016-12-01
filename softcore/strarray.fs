\ String-arrays
.( loading STRING-ARRAY ) cr

: STRING-ARRAY ( "name" -- )
	CREATE here 0 dup ,
	DOES>  ( n addr -- ca u )
		@  swap 2* cells +  2@ ;

: $" ( aptr u1 "string" -- aptr ca u u2 )
	1+  [char] " word count
	here over allot ( ca u a)
	swap 2dup >r >r  move
	r> r> rot ;

: END-STRING-ARRAY ( aptr ca1 u1 ... caN uN # -- )
	here >r  dup 2* cells allot
	here 2 cells -  swap 0 ?DO
		i 2* cells negate  over + ( ca u a a')
		swap >r  !+ !  r>
	LOOP  drop  r> swap ! ;
