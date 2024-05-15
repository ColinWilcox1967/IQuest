;******************************************************************
;***                                                            ***
;***    Secret remove Utiltiy                                   ***
;***                                                            ***
;******************************************************************

	include "sysin.equ"
	include "code.equ"

_ustack_top     equ     2d000h
_ustack_bottom  equ     2dfffh
_sstack_top     equ     2e000h
_sstack_bottom  equ     2efffh


	cseg    code                    ; code start from 40000h

;       Card Header

card_top:
	dw      1210h
	dw      0020h                   ; card size ( /2KByte) ##
	db      00                      ; version
	db      00000001b               ; type (single card)
	db      "PERSONAL   "           ; card name ##
	db      21h                     ; card id
	dp      block_header - card_top ; first block top address
	dp      0ffffffh                ;

;       Application Header Table

	dp      appli_header            ; application header top address
	dp      000000h                 ; end mark

;       Block Header

block_header:
	db      0fbh
	db      "Secret     "           ; application name
	db      21h                     ; application attribute
	dw      0000h
	dw      0000h
	dp      00ffe2h                 ; card end + 1 relative address ##
	dw      0000h
	dp      00ffe2h                 ; card end + 1 relative address ##
	dp      00ffe2h                 ; card end + 1 relative address ##
	dp      000000h
	dp      000000h
	db      0ffh
	db      00h
	db      0ah
	db      00h
version_no:
	db      00
maker_code:
	db      0
	db      00h                     ; language code
	dw      0000h

;       Application Header

appli_header:
	dw      0                       ; application number ##
	dp      program_top             ; application entry address
	dp      000000h
	dp      000000h

select_key:
	dw      KC3_2_CARD*256          ; CARD key @@
	dw      KC3_2_SHIFT_CARD*256    ; SHIFT+CARD key @@
	dw      0000h

program_top:

	pushu   ba
	mv      a,il
	cmp     a,11
	popu    ba
	jrnc    card_error
	mv      y,card_table
	add     y,il
	add     y,il
	add     y,il
	mv      y,[y]
	jp      y

card_error:
	mv      a,0feh
	sc
	retf

card_table:
	dp      sys_request     ;i=0 system requests
	dp      exec_card       ;i=1 execute card software
	dp      app_name        ;i=2 set application name
	dp      mem_free        ;i=3 calculate the amount of free memory
	dp      data_clear      ;i=4 clear data request
	dp      return          ;i=5 ?
	dp      data_send       ;i=6 send data
	dp      data_load       ;i=7 load data
	dp      checksum        ;i=8 calculate checksums
	dp      data_verify     ;i=9 verify data
	dp      directory_send  ;i=10 send data in pocketlink mode

;***********************************************************
;       process system requests, IN, OUT, RESET etc.

sys_request:
	cmp     a,0
	jrz     reset_all       ;a=0 reset all data area
	dec     a
	jrz     reset           ;a=1 reset card status
	dec     a
	jrz     power_off       ;a=2 get ready for power off
	dec     a
	jrz     power_on        ;a=3 get ready for power on
	dec     a
	jrz     in_card         ;a=4 IN process for card
	dec     a
	jpz     out_card        ;a=5 OUT process for card
	jr      card_error

power_on:
power_off:
return:
	retf
	
reset:
reset_all:
	call    display
	retf

;*********************************************************
;       set stacks to card ram, get ready to execute card

in_card:
	retf

;****************************************************************
;       reset stacks to system ram, get ready to return to system

out_card:
	retf

;********************************************************
;       execute card program

exec_card:
	call    display
	mv      (bx),ba
	retf

app_name:
	rc
	retf

mem_free:
	rc
	retf

data_clear:
	mvw     (bx),1e00h
	rc
	retf

data_send:
data_load:
data_verify:
	mvw     (bx),1f00h
	mv      a, 0
	retf

checksum:
	mv      x,0b100h
	mv      y,x
	retf

directory_send:
	rc
	retf

	extern  _password_buf
	
display:
	mvw     (cx), 0000
	mv      i, 51h
	callf   iocs_entry_f            ; clear screen

	mvw     (bx), 0000h
	mv      x, msg1
	mv      y, 16
	mvw     (cx), 0
	mv      i, 42h
	callf   iocs_entry_f            ; display message

	mvw     (bx), 0403h
	mv      x, msg2
	mv      y, 9
	mvw     (cx), 0
	mv      i, 42h
	callf   iocs_entry_f            ; display message


	mvw     (bx), 0404h
	mv      x, _password_buf
	mv      y, 7
	mvw     (cx), 0
	mv      i, 42h
	callf   iocs_entry_f            ; display message

	pushu   imr
	mv      (IMR),masteri+mti+sti+exi
	
	mvw     (cx), 0001h
	mv      i, 49h
	callf   iocs_entry_f            ; key_get_type_3

	popu    imr

	ret

msg1:
	db      "Password Display"
msg2:
	db      "[*******]"
