.include "xc.inc"
.text ;BP (put the following data in ROM(program memory))
; This is a library, thus it can *not* contain a _main function: the C file will
; deine main(). However, we
; we will need a .global statement to make available ASM functions to C code.
; All functions utilized outside of this file will need to have a leading
; underscore (_) and be included in a comment delimited list below.
.global _example_public_function, _second_public_function

    
 .global _write_0, _write_1, _delay_100us
 
 _write_0:
   inc LATA
   repeat #3
   nop
   clr LATA
   repeat #8
   nop
   return
     
 _write_1:
    inc LATA
    repeat #9
    nop
    clr LATA
    repeat #2
    nop
    return

_delay_100us:;2 cycles to call
    repeat #1593 ; 1 cycle for repeat
    nop ; nop cycles
    return ; 3 cycles for return
