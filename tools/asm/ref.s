; ---------------------------------------------------------------------------
; File: test_vram_access.asm
; Description: Tests PPU VRAM read/write operations and the internal read
;              buffer behavior in the NES PPU. Each numbered test verifies a
;              specific VRAM operation, similar to the approach in Blargg's tests.
;
; Note: The tests assume that the PPU registers ($2006, $2007) and the VRAM
;       addressing/mirroring logic are implemented correctly.
;
; The tests will:
;   1. Check the internal read buffer delay for VRAM reads.
;   2. Verify basic VRAM write/read functionality.
;   3. Ensure that VRAM writes do not inadvertently affect the internal read
;      buffer.
;   4. Confirm that palette writes do not interfere with the buffered data.
;   5. Test that palette read operations load VRAM into the read buffer.
;   6. Validate that "shadow" VRAM reads remain unaffected by palette mirroring.
;
; The file includes a common PPU prefix with utility routines.
; ---------------------------------------------------------------------------
.include "prefix_ppu.a"

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

; ---------------------------------------------------------------------------
; Subroutine: reset
; Purpose: Perform a series of tests verifying VRAM access and read buffer
;          behavior. Each test sets a result code and then runs a sequence
;          of VRAM accesses to check specific functionality.
; ---------------------------------------------------------------------------
reset:
      lda   #50
      jsr   delay_msec       ; Wait a bit for stability

      jsr   wait_vbl         ; Wait for vertical blanking period
      lda   #0
      sta   $2000          ; Disable NMI and clear control register
      sta   $2001          ; Clear mask register

; ---------------------------------------------------------------------------
; Test 2: VRAM reads should be delayed by one read (internal read buffer)
;
; Steps:
;   1. Set VRAM address.
;   2. Write two bytes consecutively.
;   3. Read VRAM twice and verify that the first read returns a buffered value.
;
      lda   #2               ; Set result code to "Test 2"
      sta   result
      lda   #$00
      jsr   set_vram_pos     ; Set VRAM addr to $2F00
      lda   #$12
      sta   $2007           ; Write a value to VRAM (will go to $2F00)
      lda   #$34
      sta   $2007           ; Write another value (to $2F01)
      lda   #$00
      jsr   set_vram_pos     ; Reset address to $2F00
      lda   $2007           ; Read first time (should load buffer, not $34)
      lda   $2007           ; Second read should produce $34 now
      cmp   #$34            ; Compare against expected value
      jsr   error_if_eq      ; Report error if the read value equals $34

; ---------------------------------------------------------------------------
; Test 3: Basic Write/Read Operation
;
; Steps:
;   1. Write a byte to VRAM.
;   2. Read it back and verify that the data matches.
;
      lda   #3               ; Set result code to "Test 3"
      sta   result
      lda   #$00
      jsr   set_vram_pos     ; Set VRAM addr to $2F00
      lda   #$56
      sta   $2007           ; Write $56 into VRAM
      lda   #$00
      jsr   set_vram_pos     ; Reset VRAM addr to $2F00
      lda   $2007           ; Read the written value (buffered read)
      lda   $2007           ; Read to actually get the correct data
      cmp   #$56            ; Verify the value matches
      jsr   error_if_ne      ; Report error if not equal

; ---------------------------------------------------------------------------
; Test 4: Ensure VRAM write does not affect read buffer.
;
; Steps:
;   1. Write a value to VRAM.
;   2. Read it to load the buffer.
;   3. Write a new value; ensure the buffer still holds the old value.
;
      lda   #4               ; Set result code to "Test 4"
      sta   result
      lda   #$00
      jsr   set_vram_pos     ; Set VRAM addr to $2F00
      lda   #$78
      sta   $2007           ; Write initial value ($78)
      lda   #$00
      jsr   set_vram_pos     ; Reset VRAM addr to $2F00
      lda   #$00
      lda   $2007           ; Load read buffer with $78
      lda   #$12
      sta   $2007           ; Write new value; should not change the buffer
      lda   $2007           ; Read the buffered value
      cmp   #$78            ; Confirm the buffer still contains $78
      jsr   error_if_ne      ; Report error if not equal

; ---------------------------------------------------------------------------
; Test 5: Palette writes should not affect the read buffer.
;
; Steps:
;   1. Write a value to VRAM.
;   2. Read it to load the buffer.
;   3. Change VRAM address to a palette location and perform a write.
;   4. Ensure the buffer remains unchanged.
;
      lda   #5               ; Set result code to "Test 5"
      sta   result
      lda   #$00
      jsr   set_vram_pos     ; Set VRAM addr to $2F00
      lda   #$9a
      sta   $2007           ; Write $9a into VRAM
      lda   #$00
      jsr   set_vram_pos     ; Reset VRAM addr to $2F00
      lda   $2007           ; Read to load buffer with $9a
      lda   #$3f
      sta   $2006           ; Set VRAM addr to palette region
      lda   #$00
      sta   $2006           ; Complete setting palette addr
      lda   #$34
      sta   $2007           ; Write to palette (should not alter buffer)
      lda   #$01        ; Change back to non-palette address to re-enable buffer reads
      jsr   set_vram_pos
      lda   $2007           ; Read buffered value
      cmp   #$9a            ; Verify the buffered value is still $9a
      jsr   error_if_ne      ; Report error if not equal

; ---------------------------------------------------------------------------
; Test 6: Palette read should also load VRAM into the read buffer.
;
; Steps:
;   1. Write a value to VRAM.
;   2. Perform a palette read to fill the buffer with a hidden VRAM value.
;   3. Change back to non-palette addressing and verify the buffered value.
;
      lda   #6               
      sta   result
      lda   #$12
      jsr   set_vram_pos    ; Set VRAM addr to $2F12
      lda   #$9a
      sta   $2007           ; Write $9a into VRAM
      lda   $2007           ; Read to load buffer with $9a

      ; Change to palette addressing
      lda   #$3f
      sta   $2006
      lda   #$12
      sta   $2006

      lda   $2007       ; Read from palette
      lda   #$13        ; Set an arbitrary value to indicate change (not used directly)
      jsr   set_vram_pos    ; Change back to non-palette addressing to enable buffer reads
      lda   $2007           ; Read
      cmp   #$9a            ; Buffer should contain $9a
      jsr   error_if_ne      ; Report error if not equal

; ---------------------------------------------------------------------------
; Test 7: "Shadow" VRAM read unaffected by palette mirroring.
;
; Steps:
;   1. Write values to VRAM.
;   2. Change addresses to trigger palette mirroring effects.
;   3. Verify that the buffered read reflects the correct VRAM data.
;
      lda   #7               ; Set result code to "Test 7"
      sta   result
      lda   #$04
      jsr   set_vram_pos     ; Set VRAM addr to $2F04
      lda   #$12
      sta   $2007           ; Write $12 into VRAM at $2F04
      lda   #$14
      jsr   set_vram_pos     ; Set VRAM addr to $2F14
      lda   #$34
      sta   $2007           ; Write $34 into VRAM at $2F14
      ; Palette memory
      lda   #$3f
      sta   $2006
      lda   #$04
      sta   $2006
      
      ; read
      lda   $2007           
      
      ; back to non-palette addressing
      lda   #$13           
      jsr   set_vram_pos

      lda   $2007           ; Read buffered value
      cmp   #$12            ; Verify correct value from $2F04
      jsr   error_if_ne      ; Report error if not equal

      ; Second part of Test 7: Verify the other tile value.
      lda   #$34
      sta   $2007           ; Write $34 into VRAM (should remain consistent)
      lda   #$3f
      sta   $2006           ; Set to palette region
      lda   #$14
      sta   $2006           ; Change back to non-palette addressing for read buffer
      lda   $2007           ; Load buffer with value from $2F14
      lda   #$13        ; Indicate change
      jsr   set_vram_pos
      lda   $2007           ; Read buffered value
      cmp   #$34            ; Verify correct value from $2F14
      jsr   error_if_ne      ; Report error if not equal

; ---------------------------------------------------------------------------
; All tests passed
; If execution reaches here, all VRAM access and read buffer tests have succeeded.
; ---------------------------------------------------------------------------
      lda   #1               ; Set result code to indicate success
      sta   result
      jmp   report_final_result  ; Jump to routine to output final test result
