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
SYSCALL			0x01,			exit
SYSCALL			0x02,			fork
SYSCALL			0x03,			write
SYSCALL			0x04,			read
SYSCALL			0x05,			brk
SYSCALL			0x06,			sbrk
SYSCALL			0x07,			getpid
SYSCALL			0x08,			waitpid
SYSCALL			0x09,			execve
SYSCALL			0x0A,			getcwd
SYSCALL			0x0B,			chdir
SYSCALL			0x0C,			getdents
SYSCALL			0x0D,			open
SYSCALL			0x0E,			close
SYSCALL			0x0F,			opendir
SYSCALL			0xA0,			closedir
SYSCALL			0xA1,			readdir
