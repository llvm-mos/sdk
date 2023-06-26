.include "imag.inc"
.include "cx16.inc"
.text

;
; typedef struct { unsigned int width, height; unsigned char depth; } cx16_fb_info_t;
; void cx16_k_fb_get_info(cx16_fb_info_t *info_ptr);
;                                         rc2/3
;
; https://github.com/X16Community/x16-docs/blob/master/X16%20Reference%20-%2004%20-%20KERNAL.md#function-name-fb_get_info
;
.global cx16_k_fb_get_info
cx16_k_fb_get_info:
	save_X16_scratch
	jsr	__FB_GET_INFO
	ldy	#5-1		; offset to depth
	sta	(__r4),y	; store depth to info_ptr
	dey			; copy remaining 4 bytes
1:	lda	__r0,y		; from r0 & r1
	sta	(__r4),y	; to info_ptr
	dey
	bpl	1b
	restore_X16_scratch
	rts
