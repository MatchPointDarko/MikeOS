global read_port
global write_port

section .text

; arg: int, port number.
read_port:
	mov edx, [esp + 4]
	in al, dx	
	ret

; arg: int, (dx)port number
;      int, (al)value to write
write_port:
	mov   edx, [esp + 4]
	mov   al, [esp + 4 + 4]
	out   dx, al  
	ret
