.segment "HEADER"
  ; Header identifier
  .byte $4E, $45, $53, $1A   ; "NES" followed by MS-DOS end-of-file marker
  .byte 2                   ; 2x 16KB PRG code
  .byte 1                   ; 1x  8KB CHR data
  .byte $01                 ; Flags 6: mapper/VRAM/mirroring info (vertical mirroring)
  .byte $00                 ; Flags 7: mapper (upper bits)
  .byte $00, $00, $00, $00   ; Flags 8-11: PRG RAM size, TV system, etc.
  .byte $00, $00, $00        ; Unused padding bytes

.segment "VECTORS"
  .addr nmi
  .addr reset
  .addr 0

.segment "STARTUP" ; required by "nes" linker config
  ; Even if empty, this segment must be present per the linker configuration.
  ; Sometimes it’s used to set up the environment before main.
  ; Leave empty if no startup code is needed.

.segment "ZEROPAGE"
  ; Add variables here.
  ; For example:
  ; result: .res 1 ; reserve 1 byte

.segment "CODE" ; main code segment

reset:
  lda #50
  jsr wait_vbl
  ; clear ppuctrl and ppumask
  lda #0
  sta $2000
  sta $2001

  ; Test 1
  ; Steps:
  ;   1. Set VRAM address.
  ;   2. Write two bytes consecutively.
  ;   3. Read VRAM twice and verify that the first read returns a buffered value.
  ;
  jsr set_vram_pos
  lda #$12
  sta $2007
  lda #$34
  sta $2007

forever:
  jmp forever

nmi:
  rti

wait_vbl:
  bit $2002
  bpl wait_vbl
  rts

; ---------------------------------------------------------------------------
; Subroutine: set_vram_pos
; Purpose: Set the VRAM address to a specific location.
;          The target address is formed by setting $2006 twice:
;          First with the high byte, then with the low byte.
;
; Input: A contains the low 8-bits of the address offset.
; Output: VRAM address set to $2F00 + A.
; Registers preserved: A, X, Y.
; ---------------------------------------------------------------------------
set_vram_pos:
      pha               ; Preserve accumulator
      lda   #$2f        ; Load high byte ($2F) for the base address
      sta   $2006       ; Write high byte to PPU VRAM address register
      pla               ; Restore accumulator (low byte)
      sta   $2006       ; Write low byte to complete the address
      rts               ; Return from subroutine
