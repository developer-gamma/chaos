;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  This file is part of the Chaos Kernel, and is made available under
;;  the terms of the GNU General Public License version 2.
;;
;;  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; These are userspace functions to call each syscalls.
; One day they won't be part of the kernel any more and be in a library
; for userland programs. But for now, it's easier to do it that way.
;
; Here is how they work:
; 1) Save edi and esi (C x86 ABI)
; 2) Put in eax the number of the syscall
; 3) Get parameters from the stack and put them in registers (edi, esi, edx and ecx)
; 4) Call the syscall by interrupting
; 5) Restore edi and esi
;
%macro SYSCALL 2
global %2:function
%2:
	push edi
	push esi
	mov eax, %1
	mov edi, [esp + 0xc]
	mov esi, [esp + 0x10]
	mov edx, [esp + 0x14]
	mov ecx, 0x0
	int 0x80
	pop esi
	pop edi
	ret
%endmacro

; Generates all the userspace syscall functions
; Remeber that ID must be the same than the one defined in include/kernel/syscall.h
;
; MACRO			ID			NAME
SYSCALL			0x1,			exit
SYSCALL			0x2,			fork
SYSCALL			0x3,			write
SYSCALL			0x4,			read
SYSCALL			0x5,			brk
SYSCALL			0x6,			sbrk
SYSCALL			0x7,			getpid
SYSCALL			0x8,			waitpid
SYSCALL			0x9,			execve
