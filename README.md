# OLL - Own Little Language *(more like a VM IR)*
## Commands
`#` - number
`@` - string/number
`%` - label
```oll
#! shebang
; comment
PUSH #      ; pushes number to stack
POP         ; pops from stack or something idk
ADD         ; adds last to numbers of stack and pushes them
SUB         ; subtracts last to numbers of stack and pushes them
PRINT @     ; print string/number to console
JUMP.EQ.0 % ; jumps to label if top of stack is 0
JUMP.GT.0 % ; jumps to label if top of stack is greater than 0
HALT #      ; return number as exit code (stops program)
```