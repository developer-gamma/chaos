;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  This file is part of the Chaos Kernel, and is made available under
;;  the terms of the GNU General Public License version 2.
;;
;;  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "include/arch/x86/asm.mac"

global x86_jump_userspace:function
global x86_return_userspace:function
global x86_context_switch:function

; void arch_context_switch(void **from_esp, void *esp)
x86_context_switch:
	mov eax, [esp + 4]
	mov edx, [esp + 8]
	pushf
	pusha

	mov [eax], esp
	mov esp, edx

	popa
	popf
	ret
;
; Jumps in userspace and calls the given function.
; Never returns (calls the exit syscall when the function is finished).
;
x86_jump_userspace:
	mov edx, [esp + 4]			; Save the parameter now

	mov ax, USER_DATA_SELECTOR | 0b11	; User data selector with bottom bits sets for ring3
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax				; ss is handled by iret, so don't worry

	mov eax, esp
	push USER_DATA_SELECTOR | 0b11		; push user data segment
	push eax				; push current stack
	pushfd					; push eflags
	push USER_CODE_SELECTOR | 0b11		; push user code segments with bottom bits sets.
	push process_main			; the usermode function to call
	iret					; let the magic happen

;
; Returns from fork to userspace
; This function is a copy of last part of the syscall interrupt handler.
; It pops all the registers from the stack and performs an iret
; Never returns
;
x86_return_userspace:
	mov eax, [esp + 4]
	mov esp, eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret

;
; Very first function running in usermode of every threads.
; Simply calls the thread main function and then the exit syscall.
;
; The main function is passed in edx
process_main:
	call edx		; Call the given main function

	; TODO Exit syscall
	mov eax, 0
	int 0x80		; Syscall


