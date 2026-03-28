	.586
	.MODEL ?MODEL
	option casemap:none
	option proc:private

	.code

JOY_get_count proc c public 
	pushfd
	cli
	mov al, 06h
	out 43h, al
	in al, 40h
	mov ah, al
	in al, 40h
	xchg ah, al 
	popfd
	ret
	align 4

JOY_get_count endp


JOY_cli proc c public 
	cli
	ret
	align 4

JOY_cli endp


JOY_sti proc c public 
	cli
	ret
	align 4

JOY_sti endp


JOY_get_tsc proc c public
	rdtsc
	ret
	align 4

JOY_get_tsc endp

	END

