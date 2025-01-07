; CP/M entrypoints for C; the assembler parts.
;
; © 2022 David Given
; This file is part of the llvm-mos-sdk project and is redistributable under the
; terms of the Apache 2.0 license with the LLVM exceptions. See the LICENSE file
; in the project root for the full text.

.macro cpmentry sym:req, num:req, exiter
    .section .text.\sym, "ax", @progbits
    .global \sym
\sym:
    ldy #\num
    .ifb \exiter
        jmp BDOS
    .else
        jsr BDOS
        jmp \exiter
    .endif
.endm

.macro biosentry sym:req, num:req, exiter
    .section .text.\sym, "ax", @progbits
    .global \sym
\sym:
    ldy #\num
    .ifb \exiter
        jmp BIOS
    .else
        jsr BIOS
        jmp \exiter
    .endif
.endm

cpmentry cpm_warmboot,                 0
cpmentry cpm_conin,                    1
cpmentry cpm_conout,                   2
cpmentry cpm_auxin,                    3
cpmentry cpm_auxout,                   4
cpmentry cpm_lstout,                   5
cpmentry cpm_conio,                    6
cpmentry cpm_get_iobyte,               7
cpmentry cpm_set_iobyte,               8
cpmentry cpm_printstring_i,            9
cpmentry cpm_readline_i,              10, __cpmexiter_errno
cpmentry cpm_const,                   11
cpmentry cpm_get_version,             12
cpmentry cpm_reset_disk_system,       13
cpmentry cpm_select_drive,            14
cpmentry cpm_open_file_i,             15, __cpmexiter_errno
cpmentry cpm_close_file_i,            16, __cpmexiter_errno
cpmentry cpm_findfirst_i,             17, __cpmexiter_aorerrno
cpmentry cpm_findnext_i,              18, __cpmexiter_aorerrno
cpmentry cpm_delete_file_i,           19, __cpmexiter_errno
cpmentry cpm_read_sequential_i,       20, __cpmexiter_errno
cpmentry cpm_write_sequential_i,      21, __cpmexiter_errno
cpmentry cpm_make_file_i,             22, __cpmexiter_errno
cpmentry cpm_rename_file_i,           23, __cpmexiter_errno
cpmentry cpm_get_login_vector,        24
cpmentry cpm_get_current_drive,       25
cpmentry cpm_set_dma_i,               26
cpmentry cpm_get_allocation_vector_i, 27
cpmentry cpm_write_protect_drive,     28
cpmentry cpm_get_readonly_vector,     29
cpmentry cpm_set_file_attributes_i,   30, __cpmexiter_errno
cpmentry cpm_get_dpb_i,               31
cpmentry cpm_get_set_user,            32
cpmentry cpm_read_random_i,           33, __cpmexiter_errno
cpmentry cpm_write_random_i,          34, __cpmexiter_errno
cpmentry cpm_seek_to_end_i,           35
cpmentry cpm_seek_to_seq_pos_i,       36
cpmentry cpm_reset_drives,            37, __cpmexiter_errno
cpmentry cpm_write_random_filled_i,   40, __cpmexiter_errno
cpmentry cpm_getzp,                   41
cpmentry cpm_gettpa,                  42
cpmentry cpm_parse_filename_i,        43, __cpmexiter_ptrornull

biosentry cpm_bios_const,             0
biosentry cpm_bios_conin,             1
biosentry cpm_bios_conout,            2
biosentry cpm_bios_seldsk_i,          3, __cpmexiter_ptrornull
biosentry cpm_bios_setsec_i,          4
biosentry cpm_bios_setdma_i,          5
biosentry cpm_bios_read,              6, __cpmexiter_errno
biosentry cpm_bios_write,             7, __cpmexiter_errno
biosentry cpm_bios_relocate,          8
biosentry cpm_bios_gettpa,            9
biosentry cpm_bios_settpa,            10
biosentry cpm_bios_getzp,             11
biosentry cpm_bios_setzp,             12

; On exit from the BDOS (or BIOS), the return value is in A
; and carry is set on error.

.section .text.cpmexiter_errno, "ax", @progbits
__cpmexiter_errno:
    sta cpm_errno
    lda #0
    bcc 1f
    lda #$ff
1:
    rts
    
.section .text.cpmexiter_aorerrno, "ax", @progbits
__cpmexiter_aorerrno:
    sta cpm_errno
    bcc 1f
    lda #$ff
1:
    rts

.section .text.cpmexiter_ptrornull, "ax", @progbits
__cpmexiter_ptrornull:
    bcc 1f
    lda #0
    tax
1:
    rts

.section .bss.cpm_errno
.global cpm_errno
cpm_errno: .fill 1

; vim: ts=4 sw=4 et ft=asm

