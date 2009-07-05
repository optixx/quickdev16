;----------------------------------------------------------------------;
;                               Header File                            ;
;                         (Written By MarcTheMER)                      ;
;                      Email: marcthemer@hotmail.com                   ;
;======================================================================;
;    I had written this simple yet useful header file to help SNES     ;
;  programmers with their assembly. This section of code modifies the  ;
; SNES cartridge header so it is compatable for use on zsnes or snes9x ;
;======================================================================;
;  If you use any of my header files to create an awesome game please  ;
;    email me as I am always trying to learn new programming skills.   ;
;----------------------------------------------------------------------;


.SNESNATIVEVECTOR                   ; Defines inturupt vactor table
  COP DoNothing
  BRK DoNothing
  ABORT DoNothing
  NMI VBlank
  IRQ DoNothing
.ENDNATIVEVECTOR

.SNESEMUVECTOR                      ; Defines inturupt vactor table
  COP DoNothing
  ABORT DoNothing
  NMI DoNothing                     ; I wouldn't recomend using emulation mode.
  RESET Main
  IRQBRK DoNothing
.ENDEMUVECTOR


.EMPTYFILL $00                      ; fills the rom with $00

.BANK 0 SLOT 0
.ORG 0
.SECTION "EmptyVectors" SEMIFREE    ; Set up interupt vector coding.

DoNothing:
  RTI

.ENDS
