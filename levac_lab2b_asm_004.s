.include "xc.inc"
.text 

.global _write_0, _write_1, _delay_100us
 
 _write_0:
   bset LATB, #0xD ;RB13 is used
   repeat #3
   nop
   bclr LATB, #0xD 
   repeat #8
   nop
   return
     
 _write_1:
    bset LATB, #0xD
    repeat #8
    nop
    bclr LATB, #0xD
    repeat #3
    nop
    return

_delay_100us:;2 cycles to call
    repeat #1593 ; 1 cycle for repeat
    nop ; nop cycles
    return ; 3 cycles for return



