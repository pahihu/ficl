ANEW -ficlex

: noop ;

: defSelect ( n -- ) ." default = " . ;

[switch +select defSelect
   1 run: ." one = " ;
   3 run: ." three = " ;
switch]

string-array $months[]
   $" january"
   $" february"
   $" march"
   $" april"
   $" may"
   $" june"
   $" july"
   $" august"
   $" september"
   $" october"
   $" november"
   $" december"
end-string-array

: .months ( -- )
   12 0 DO  i $months[]  type cr  LOOP ;
