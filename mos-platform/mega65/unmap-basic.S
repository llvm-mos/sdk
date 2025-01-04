; Switch out the BASIC ROM bank from $A000 to $BFFF.
; This opens up 45KB of contiguous RAM from $2001-$CFFF.
; KERNAL ROM at $E000-$FFFF and I/O at $D000-$DFFF are still mapped.



; bank switching needs to complete before any CRT initialization
.section .init.010,"axR",@progbits
	sei
        ldx #$2F ; restore default CPU I/O port data directions
        stx 0    ;
        ldx #$3E ; LORAM = 0
        stx 1    ; switch out C64 BASIC ROM at $A000-$BFFF for RAM
        
        ldx #$44 ; unmap all C65 ROMs except $e000-$ffff
        stx $d030

        lda #$16 ; Switch zero page to $1600 - $16ff
        tab


; restore BASIC ROM after all other exit handlers have completed
.section .fini.990,"axR", @progbits
        ldx #$3F ; LORAM = 1
        stx 1    ; switch in BASIC ROM at $A000-$BFFF

        ; restore C65 ROMs to as before
        ldx #$64
        stx $d030

        lda #$00 ; Switch zero page back to default
        tab
        
	cli
