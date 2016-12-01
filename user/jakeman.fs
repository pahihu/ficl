\ C.Jakeman             VD2/99
\ requires Mini-OOF from B.Paysan
object class
   cell var x
   cell var y
end-class position

position class
   cell var text
   cell var len
   method init
   method draw
end-class button

:noname ( &obj -- ) >R
   R@ x @    R@ y @  at-xy
   R@ text @ R> len @ type
; button defines draw

:noname ( x y addr u &obj -- ) >R
   R@ len !  R@ text !  R@ y !  R> x !
; button defines init

button class
end-class bold-button

: bold ( -- )  [char] * emit ;

:noname ( &obj -- ) >R
   R@ x @ 1-  R@ y @ at-xy
   R> bold [ button :: draw ] bold
; bold-button defines draw


button new CONSTANT button1
bold-button new CONSTANT bold-button1

string-array messages
   $" plain message"
   $" bold message"
end-string-array

11 5  0 messages  button1 init
10 6  1 messages  bold-button1 init

page
button1 draw
bold-button1 draw

