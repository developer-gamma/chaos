;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  This file is part of the Chaos Kernel, and is made available under
;;  the terms of the GNU General Public License version 2.
;;
;;  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global gdt_tss_entry
global gdtptr_phys
global gdtptr

%include "include/arch/x86/asm.mac"


section .rodata
align 16
gdtptr_phys:
	dw gdt_end - gdt_start - 1
	dd PHYS(gdt)

section .rodata
align 16
gdtptr:
	dw gdt_end - gdt_start - 1
	dd gdt

section .data
align 16
gdt:
gdt_start:
	; Null entry
	dd 0
	dd 0

	; Kernel Code selector
	dw 0xFFFF	; limit 15:0
	dw 0x0000	; base 15:0
	db 0x00		; base 23:16
	db 0b10011010	; P(1) DPL(00) (1) C(1) D(0) R(1) A(0)
	db 0b11001111	; G(1) S(1) (0) (0) limit 19:16
	db 0x00		; base 31:24

	; Kernel data selector
	dw 0xFFFF	; limit 15:0
	dw 0x0000	; base 15:0
	db 0x00		; base 23:16
	db 0b10010010	; P(1) DPL(00) (1) C(0) E(0) W(1) A(0)
	db 0b11001111	; G(1) S(1) (0) (0) limit 19:16
	db 0x00		; base 31:24

	; User Code selector
	dw 0xFFFF	; limit 15:0
	dw 0x0000	; base 15:0
	db 0x00		; base 23:16
	db 0b11111010	; P(1) DPL(11) (1) C(1) D(0) R(1) A(0)
	db 0b11001111	; G(1) S(1) (0) (0) limit 19:16
	db 0x00		; base 31:24

	; User data selector
	dw 0xFFFF	; limit 15:0
	dw 0x0000	; base 15:0
	db 0x00		; base 23:16
	db 0b11110010	; P(1) DPL(11) (1) C(0) E(0) W(1) A(0)
	db 0b11001111	; G(1) S(1) (0) (0) limit 19:16
	db 0x00		; base 31:24

gdt_tss_entry:
	; Tss selector
	dw 0x0000	; limit 15:0
	dw 0x0000	; base 15:0
	db 0x00		; base 23:16
	db 0b11101001	; P(1) DPL(11) 0 10 B(0) 1
	db 0b10000000	; G(1) 0 0 AVL(0) limit 19:16
	db 0x00		; base 31:24

	; Null entry
	dd 0
	dd 0
gdt_end:
