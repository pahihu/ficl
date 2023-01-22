anew -point

class point
protected
   \ create buffer: variable cvariable constant
   variable x
   variable y
   defer: show ( -- )   x @ .  y @ . ;
public
   : get ( -- x y )   x @  y @ ;
   : put ( x y -- )   y ! x ! ;
   : dot ( -- )   cr ." Point at " show ;
end-class

point builds origin

5 8 origin put
origin dot

\ 6 pad !  9 pad cell+ !
\ pad using point dot

0 value foo
point new to foo
   8 99 foo using point put
   foo using point dot
foo destroy   0 to foo


class rectangle
   point builds ul
   point builds lr
   : show ( -- )   ul dot  lr dot ;
   : dot ( -- )   ." Rectangle, " show ; 
end-class

point subclass label-point
   : show ( -- )   ." X " x @ .  ." Y " y @ . ;
end-class

label-point builds poo
2 3 poo put
poo dot


