;;************************************************************
;esr-l                                  IOCS
;       
;syswork
;                       personal div. engineering dev. 3
;                                       yasuharu tanaka
;       fixed address system work.
;       for application control
;       start address = 
;
;ver. 15        1988.2.4        PC-E500
;ver. 20        1988.4.29
;ver. 21        1988.7.29
;       mb_flag                         y.wada
;;************************************************************

;------------------------------------------------------------
; segment definition
;------------------------------------------------------------
        cseg    syswork

;------------------------------------------------------------
        public  ui

ui      equ     0e0h            ;virtual register for C compiler
;------------------------------------------------------------
        public  syswork_top
syswork_top:

;=============================================================
;data block controll works
;=============================================================
        public  main_ram_top
        public  main_ram_bottom
        public  main_rom_top
        public  card_ram_top
        public  card_ram_bottom
        public  card_rom_top
        public  main_header_tbl
        public  card_header_tbl
        public  appli_code
        public  last_appli_code
        public  appli_key
        public  last_appli_key
        public  current_ymd_buf
        public  current_time_buf
        public  current_drive
        public  current_mode
        public  current_rel_adr
        public  card_ram_checksum
        public  card_rom_checksum
        public  main_u_backup
        public  sub_u_backup

;------------------------------------------------------------
;
;------ memory block , data packet system area (syswork)
;
main_ram_top            ds      3
main_ram_bottom         ds      3
main_rom_top            ds      3

card_ram_top            ds      3
card_ram_bottom         ds      3
card_rom_top            ds      3
;
;------ apm system area (syswork)
;
main_header_tbl         ds      3       ;main appli entry adr table
card_header_tbl         ds      3       ;card appli entry adr table

appli_code              ds      2       ;present appli Nr.
last_appli_code         ds      2       ;previous appli Nr.
appli_key               ds      2       ;present appli start key code
last_appli_key          ds      2       ;previous appli start key code

current_ymd_buf         ds      12      ;common Year Month Date buffer
current_time_buf        ds      4+4     ;common hour minute buffer
;                                       ;+4 is reserved size. by Wada.
current_drive           ds      1       ;common block's drive Nr.
current_mode            ds      1       ;common block's mode
current_rel_adr         ds      3       ;common block relative address

card_ram_checksum       ds      3       ;power off checksum buffer
card_rom_checksum       ds      3

main_u_backup           ds      4       ;appli in stack level keep buffer
sub_u_backup            ds      4       ;gosub type appli in stack
                                        ;keep buffer

;
;============================================================
; lcd graphic work
;============================================================
        public  linptn
;------ 
linptn          ds      2       ;line pattern for line,box and boxfill


;=============================================================
;key work       (iocs)
;=============================================================
        public  editor_work_ptr
        public  repeat_key
        public  fp_tbl_ptr
        public  type2_table_top_adr
        public  key_23_1st_tbl_adr
        public  key_23_2nd_tbl_adr
        public  umlaut_convert_hook
        public  symbol_data_tbl_adr

;------ 
editor_work_ptr ds      1       ;editor exit or not exit
                                ; =ffh  = not exit
                                ; <>ffh = exit

;------ 
repeat_key      ds      1       ;last pushed key matrix code. 0 is empty
                                ;use in key_look_type 2 
                                ;       key_get_type 2 routine

fp_tbl_ptr              ds      3       ;card side front processor table
                                        ;    top address

type2_table_top_adr     ds      3       ;matrix code --> type 2
                                        ;  table top address

;------ 
key_23_1st_tbl_adr      ds      3       ;type 2 --> type 3
                                        ;  table top address
                                        ;1st byte 0 - 7fh table
                                        ; from front processor 

key_23_2nd_tbl_adr      ds      3       ;type 2 --> type 3
                                        ;  table top address
                                        ;1st byte =0
                                        ;2nd byte 0 -7fh table
                                        ; from front process

;------ 
umlaut_convert_hook     ds      3       ;front process
                                        ; umlaut process
                                        ;umlaut convert hook adr.

;------ 
symbol_data_tbl_adr     ds      3       ;front process
                                        ; symbol data top address

;
;============================================================
; system routine work
;============================================================
        public  machine_auto_slot
        public  off_u_stack
        public  power_mode
        public  application_vector
        public  bp_btm
        public  menu_ctrl

;------ 
machine_auto_slot       ds      1       ;for auto start machine language
                                ;start slot number to search 
                                ;auto run program
                                ; msb=1 : not auto run.
                                ; = 2 : search program in slot 2 -> 1 -> 0
                                ; = 1 : slot 1 -> 0
                                ; = 0 : slot 0 only

off_u_stack             ds      3       ;off time u reg.

power_mode      ds      1       ;power continius mode controll
                                ;bit 1: sio inuse
                                ;bit 0: timer inuse

application_vector      ds      3       ;jump to application adr

bp_btm          ds      1       ;bp area bottom adr.

menu_ctrl       ds      1       ;main menu control
                                ;msb= 1 : not disp menu and auto select
                                ;bit 6 - 0: auto select pf key No.

;============================================================
; counter work
;============================================================
        public  _counter_16

;------ 
_counter_16     ds      3       ;interbal counter of 16 msec.
                                ;for time out, timer

;
;============================================================
; cg pointer work
;============================================================
        public  insert_mark_ptr
        public  insert_mark_ptr8
        public  insert_mark1_ptr
        public  insert_mark1_ptr8
        public  insert_mark2_ptr
        public  insert_mark2_ptr8
        public  ctrl_cg_top
        public  lower_cg_top
        public  upper_cg0_top
        public  upper_cg1_top
        public  upper_cg2_top
        public  ctrl_cg_top8
        public  lower_cg_top8
        public  upper_cg0_top8
        public  upper_cg1_top8
        public  upper_cg2_top8

;------ 
insert_mark_ptr ds      3       ;brink insert mark font(6bytes) top adr.
                                ;               (for 5*7)
insert_mark_ptr8        ds      3       ;brink insert mark
                                        ; font (16 bytes) top adr. (for 8*16)

insert_mark1_ptr        ds      3       ;brink insert mark font(6bytes)
                                        ;        top adr. (for 5*7)
insert_mark1_ptr8       ds      3       ;brink insert mark
                                        ; font (16 bytes) top adr. (for 8*16)

insert_mark2_ptr        ds      3       ;brink insert mark font(6bytes)
                                        ;        top adr. (for 5*7)
insert_mark2_ptr8       ds      3       ;brink insert mark
                                        ; font (16 bytes) top adr. (for 8*16)
;------------------------------------------------------------

ctrl_cg_top     ds      3       ;5*7 dots cg font for code 00h-1fh
lower_cg_top    ds      3       ;5*7 dots cg font for code 20h-7fh
upper_cg0_top   ds      3       ;5*7 dots cg font for code 80h-9fh
upper_cg1_top   ds      3       ;5*7 dots cg font for code a0h-dfh
upper_cg2_top   ds      3       ;5*7 dots cg font for code e0h-ffh

ctrl_cg_top8    ds      3       ;8*16 dots cg font for code 00h-1fh
lower_cg_top8   ds      3       ;8*16 dots cg font for code 20h-7fh
upper_cg0_top8  ds      3       ;8*16 dots cg font for code 80h-9fh
upper_cg1_top8  ds      3       ;8*16 dots cg font for code a0h-dfh
upper_cg2_top8  ds      3       ;8*16 dots cg font for code e0h-ffh

;------------------------------------------------------------
; tempolary area top
;------------------------------------------------------------
        public  temp_syswork_top
        public  temp_u_stack_btm
        public  temp_s_stack_btm

temp_s_stack_btm:

temp_u_stack_btm        equ     temp_s_stack_btm -35
temp_syswork_top        equ     temp_u_stack_btm -30

;------------------------------------------------------------

;
;=============================================================
;lcd device work
;=============================================================
        public  graphic_mode, dotsop
        public  lcd_symbol0
        public  lcd_symbol1
        public  lcd_symbol2
        public  lcd_symbol3
        public  lcd_crsr_x
        public  lcd_crsr_y
        public  lcd_width
        public  lcd_height
        public  lcd_width8
        public  lcd_height8
        public  lcd_mode
        public  lcd_cg_mode

        public  lcd_contrast

;------ text chacter disp
dotsop:
graphic_mode    ds      1       ;graphic pattern output operation mode
                                ;bit 7-4: 0
                                ;bit 3-0: 2 - exclusived or
                                ;       : 1 - or
                                ;       : 0 - set

;------ lcd

lcd_crsr_x      ds      1       ;phisical cursor position x
lcd_crsr_y      ds      1       ;phisical cursor position y

lcd_width       ds      1       ;lcd width by 5*7 size. (default= 16)
lcd_height      ds      1       ;lcd hight by 5*7 size.(default= 8)

lcd_width8      ds      1       ;lcd width by 8*16 size.(default= 12)
lcd_height8     ds      1       ;lcd hight by 8*16 size.(default= 4)

lcd_mode        ds      1       ;lcd display mode
                                ;bit 7: 0 
                                ;bit 6: reverse character mode
                                ;bit 5: character size ( 8*16 / 5*7)

lcd_cg_mode     ds      1       ;lcd cg control
                                ;bit 0: cg_emergency_bit
                                ;       0 according cg work
                                ;       1 make sze original cg

;------------------------------------------------------------
;------ symbol

lcd_symbol0     ds      1       ;lcd symbol bit NO HIKAE 0
                                ;bit 7: low battery
                                ;bit 6: card
                                ;bit 5: edit
                                ;bit 4: shift
                                ;bit 3: caps
                                ;bit 2: *
                                ;bit 1: ONPU
                                ;bit 0: ^ (up sign)

lcd_symbol1     ds      1       ;lcd symbol bit NO HIKAE
                                ;bit 7-3: 0
                                ;bit 2: secret sign
                                ;bit 1: bell sign
                                ;bit 0: v (down sign)

lcd_symbol2     ds      1       ;lcd symbol bit NO HIKAE
                                ;bit 7: <-- (backword sing)
                                ;bit 6-0: 0

lcd_symbol3     ds      1       ;lcd symbol bit NO HIKAE
                                ;bit 7: --> (foward sign)
                                ;bit 6-0: 0

lcd_contrast            ds      1       ;lcd contrast 0--1fh
;
;============================================================
; iocs main work
;============================================================
        public  iocs_header_adr

;------ 
iocs_header_adr ds      3       ;iocs header list top adr.

;=============================================================
;key scan work  (irq)
;=============================================================
        public  keymatrix_mask
        public  onkey_matrix_mask
        public  real_key
        public  last_key
        public  key_irq_flag
        public  repeat_ctr
        public  repeat_wait
        public  repeat_pitch
        public  auto_power_off_time
        public  key_irq_flag2
        public  k_scan_mode
        public  key_matrix_tbl
        public  key_look_hook
        public  card_key_matrix_tbl_adr
        public  set_keybuf_hook
;------ 
keymatrix_mask  ds      16      ;key scan matrix disable bits
                                ;       bit on: key enable
                                ;          off: key disable
                                ;+0 strobe no.0
                                ;+1 strobe no.1
                                ;+2 strobe no.2
                                ;+3 strobe no.3
                                ;+4 strobe no.4
                                ;+5 strobe no.5
                                ;+6 strobe no.6
                                ;+7 strobe no.7
                                ;+8 strobe no.8
                                ;+9 strobe no.9
                                ;+a strobe no.10
                                ;+b strobe no.11
                                ;+c strobe no.12
                                ;+d strobe no.13
                                ;+e strobe no.14
                                ;+f strobe no.15

onkey_matrix_mask       ds      1       ;bit 0: onkey_matrix_mask
                                        ;       bit on: key enable
                                        ;          off: key disable

;------ can't exchange
real_key        ds      2       ;now pushed key matrix code (for 2 key)
                                ;=0 is empty

last_key        ds      1       ;last pushed key matrix code. 0 is empty
                                ;use in repeat routine

key_irq_flag    ds      1       ;key irq routine flags
                                ;bit 1: real key 1 NO baunce flag
                                ;bit 0: real key 0 NO baunce flag
                                ;       =1 : now baunce wait
                                ;caution! can't read or write
                                ;         from normal routine

repeat_ctr      ds      1       ;repeat counter
                                ;<>0: repeat start time (*16msec.)
                                ;= 0: repeat stop now

;------ 
repeat_wait     ds      1       ;repeat start wait time preset (*16 msec.)
repeat_pitch    ds      1       ;repeat pitch time preset (*16 msec.)

auto_power_off_time     ds      2       ;auto power off time

key_irq_flag2   ds      1       ;key irq routine flags 2
                                ;bit 7: on/break key now pushed
                                ;bit 6: low battery fatal

k_scan_mode     ds      1       ;key scan mode byte
                                ;bit 7-6: repeat area
                                ;       3:all key repeat
                                ;bit 5:0
                                ;bit 4: click
                                ;bit 3: repeat click on/off

key_matrix_tbl  ds      3       ;key matrix code table top adr.

key_look_hook   ds      3       ;look scan buffer routine hook

card_key_matrix_tbl_adr ds      3       ;card key matrix table top address
set_keybuf_hook ds      3       ;set keybuf routine hook

;============================================================
; irq routine vector
;============================================================
;------ IRQ vecter HA KAKU IRQ YOUKYUU NO SYORI WO OKONAU routine NO
;------ entry adr. WO SHIMESU pointer DESU.

        public  irq0_vect
        public  irq1_vect
        public  irq2_vect
        public  irq3_vect
        public  irq4_vect
        public  irq5_vect
        public  irq6_vect
        public  soft_irq_vect

;------ 
irq0_vect       ds      3       ;m sec.
irq1_vect       ds      3       ;sec.
irq2_vect       ds      3       ;key
irq3_vect       ds      3       ;key
irq4_vect       ds      3       ;key
irq5_vect       ds      3       ;key
irq6_vect       ds      3       ;key
soft_irq_vect   ds      3       ;software break

;------------------------------------------------------------
; tempolary work bottom
;------------------------------------------------------------
        public  temp_syswork2_top
        public  temp_syswork1_end
        public  temp_syswork1_size

temp_syswork1_end:
temp_syswork1_size      equ     temp_syswork1_end-temp_syswork_top
temp_syswork2_top:              ;tempolary syswork2 area top adr.

;============================================================
;stack works pointers
;============================================================
        public  u_stack_top
        public  u_stack_btm
        public  s_stack_top
        public  s_stack_btm
        public  df_u_stack_size
        public  df_s_stack_size

;------ 
u_stack_top     ds      3
                ;user stack top address
u_stack_btm     ds      3
                ;user stack bottom address+1
s_stack_top     ds      3
                ;system stack top address
s_stack_btm     ds      3
                ;system stack botom address+1

;------ default size

df_u_stack_size         equ     1000h
df_s_stack_size         equ     1000h

;;-----------------------------------------------------------
;
;       |               |
;       +---------------+ <-- u_stack_top
;       |               |
;       | user stack    |
;   u-> |               |
;       |               |
;       +---------------+ <-- u_stack_btm
;       |               |
;       |               |
;       +---------------+ <-- s_stack_top
;       |               |
;       | system stack  |
;       |               |
;   s-> |               |
;       +---------------+ <-- s_stack_btm
;       |               |
;
;;-----------------------------------------------------------

;============================================================
; application pointers
;============================================================
        public  flowting_area_top

        public  appl_ptr20
        public  appl_ptr19
        public  appl_ptr18
        public  appl_ptr17
        public  appl_ptr16
        public  appl_ptr15
        public  appl_ptr14
        public  appl_ptr13
        public  appl_ptr12
        public  appl_ptr11
        public  appl_ptr10
        public  appl_ptr9
        public  appl_ptr8
        public  appl_ptr7
        public  appl_ptr6
        public  appl_ptr5
        public  appl_ptr4
        public  appl_ptr3
        public  appl_ptr2
        public  appl_ptr1
        public  appl_ptr0
        public  syswork3_top
        public  syswork2_top
        public  machine_lang_top

        public  flowting_ptr_num
        public  df_flowting_area_size

;------ flowting area top adr pointer

flowting_area_top       ds      3

;------ for apprication flowting work pointer

appl_ptr20      ds      3
                ;application work area top address pointer
appl_ptr19      ds      3
                ;application work area top address pointer
appl_ptr18      ds      3
                ;application work area top address pointer
appl_ptr17      ds      3
                ;application work area top address pointer
appl_ptr16      ds      3
                ;application work area top address pointer
appl_ptr15      ds      3
                ;application work area top address pointer
appl_ptr14      ds      3
                ;application work area top address pointer
appl_ptr13      ds      3
                ;application work area top address pointer
appl_ptr12      ds      3
                ;application work area top address pointer
appl_ptr11      ds      3
                ;application work area top address pointer
appl_ptr10      ds      3
                ;application work area top address pointer
appl_ptr9       ds      3
                ;application work area top address pointer
appl_ptr8       ds      3
                ;application work area top address pointer
appl_ptr7       ds      3
                ;application work area top address pointer
appl_ptr6       ds      3
                ;application work area top address pointer
appl_ptr5       ds      3
                ;application work area top address pointer
appl_ptr4       ds      3
                ;application work area top address pointer
appl_ptr3       ds      3
                ;application work area top address pointer
appl_ptr2       ds      3
                ;application work area top address pointer
appl_ptr1       ds      3
                ;application work area top address pointer
appl_ptr0       ds      3
                ;application work area top address pointer

;------ for system flowting work pointer

syswork3_top    ds      3
                ;system work area 3 top pointer
syswork2_top    ds      3
                ;system work area2 top pointer
machine_lang_top        ds      3
                ;machine language area top address pointer

;------ flowting work equ

flowting_ptr_num        equ     24
df_flowting_area_size   equ     1150

;;-----------------------------------------------------------
;
;       |               |
;       +---------------+ <-- flowting_area_top
;       |           ^   |
;       | free area |   |
;       |               |
;       +---------------+ <-- appl_work_ptr1 - 14
;       | other work    |
;       |               |
;       | (flowting)    |
;       +---------------+ <-- appl_work_ptr0
;       | tel work      |
;       |               |
;       | (flowting)    |
;       +---------------+ <-- syswork2_top, syswork3_top
;       | syswork 2     |
;       |               |
;       | (flowting)    |
;       +---------------+ <-- machn_area_top
;       | machine       |
;       | language area |
;       | (flowting)    |
;       +---------------+
;       | system work1  |
;       |               |
;       | (fixed)       |       
;       +---------------+ <-- RAM end
;
;;-----------------------------------------------------------

;============================================================
; lcd work
;============================================================
        public  effective_for_ctrl_code
        public  lcd_extra_crx
        public  lcd_extra_cry
        public  cursor_size_x
        public  cursor_size_y
        public  lcd_extra_mode
        public  cursor_extra_adr
        public  lcd_cg_type
;------ 
;------ for print_chr & print_n_chr
effective_for_ctrl_code ds      1
                                ;effective for null & tab code
                                ;bit 0: null code  0  carriage return 
                                ;                  1  lcd disp
                                ;bit 1: tab code   0  carriage return
                                ;                  1  lcd disp

;------------------------------------------------------------
;------    graphic (extra) chracter disp
lcd_extra_crx   ds      2       ;graphic cursor x coordinate
lcd_extra_cry   ds      2       ;graphic cursor y coordinate

cursor_size_x   ds      1       ;graphic cursor size x
cursor_size_y   ds      1       ;graphic cursor size y

lcd_extra_mode  ds      1       ;lcd display mode
                                ;bit 7: 0 
                                ;bit 6: reverse character mode
                                ;bit 5: character size ( 8*16 / 5*7)
                                ;bit 3-0: 2 - exclusived or
                                ;       : 1 - or
                                ;       : 0 - set

cursor_extra_adr        ds      3
                                ;lcd pattern pop positoin address
                                ;  for graphic cursor

lcd_cg_type     ds      1       ;cg type
                                ; =0  sze original cg
                                ; =1  IBM 1
                                ; =2  IBM 2
                                ; if bit 7 =1  modify cg set

;===========================================================
; prt system work
;===========================================================
        public  prt_width
        public  prt_width8
        public  prt_mode
        public  prt_spacing
        public  prt_effective_for_ctrl_code

;-----
prt_width       ds      1       ;1 line chr.No.[byte] 5*7 mode
prt_width8      ds      1       ;1 line chr.No.[byte] 8*16 mode
prt_mode        ds      1       ;bit2|0:none       |1:auto cr
                                ;bit5|0:5*7 mode   |1:8*16 mode
                                ;bit6|0:normal     |1:reverce
prt_spacing     ds      1       ;space of both sides [dots]
prt_effective_for_ctrl_code     ds      1
                                ;effective for null & tab code
                                ;bit 0: null code  0  carriage return 
                                ;                  1  print
                                ;bit 1: tab code   0  carriage return
                                ;                  1  print

;============================================================
; pa_net communication
;============================================================
        public  pnet_type
;------ 
pnet_type       ds      1       ;00h:ce50p          |01h:set com

;
;============================================================
; sio work
;============================================================
        public  sio_buf_status          
        public  sio_kanjin_buf          
        public  sio_kanjout_buf         
        public  sio_timer               
        public  sio_timer_master        
        public  sio_baud_rate           
        public  sio_setup               
        public  sio_receive_port_condition
        public  sio_receive_port_control
        public  sio_send_port_condition 
        public  sio_send_port_control   
        public  sio_send_delay          
        public  sio_receive_wait                
        public  sio_crlf                
        public  sio_eof_code            
        public  sio_error               
        public  sio_flag                
        public  sio_flag_receive        
        public  sio_open_close_wait
        public  sio_open_port_control
        public  sio_send_nbyte_wait
        public  sio_open_send_data
        public  sio_close_send_data
        public  sio_delimit_code

;------ 
sio_buf_status  ds      1       ;sio flow control status for IRQ
                                ;bit 0: xoff received
                                ;       ( send xon/off mode only )
                                ;bit 1-3: 0
                                ;bit 4: buffer overrun error
                                ;bit 5: receive flow is stop mode
                                ;       ( receive xon/off mode only )

sio_kanjin_buf  ds      3       ; KANJI in code (00h is end mark)
                                ; all 00h is not use mode.

sio_kanjout_buf ds      3       ; KANJI in code (00h is end mark)
                                ; all 00h is not use mode.

sio_timer       ds      2       ;0.5sec. decrement counter
                                ;65535 -> 0 ( if 0 then not decrement)

sio_timer_master:               ;sio timer preset value
                ds      2

sio_baud_rate   ds      1       ;sio baud rate register image.
                                ;bit 0:stop bit
                                ;       0= 1stop bit /1= 2stop bit
                                ;bit 1:chr. length
                                ;       0= 8bit / 1= 7bit
                                ;bit 3-2: parity
                                ;       00= even parity
                                ;       01= odd parity
                                ;       1*= non parity
                                ;bit 6-4: baud rate
                                ;       000=    sio stop
                                ;       001=  300 baud
                                ;       010=  600 baud
                                ;       011= 1200 baud
                                ;       100= 2400 baud
                                ;       101= 4800 baud
                                ;       110= 9600 baud
                                ;       111=19200 baud

sio_setup       ds      1       ;sio mode setup
                                ;bit 0: shift in/out mode
                                ;bit 1: send xon/off mode
                                ;bit 2: receive xon_off mode
                                ;bit 3: KANJI in/out mode
                                ;bit 4: send close data enable
                                ;bit 5: national version
                                ;       0= over sea
                                ;       1= japan
                                ;bit 6: send open data enable
                                ;bit 7: open mode
                                ;       0= not open
                                ;       1= open now

sio_receive_port_condition:     ;receive data available port condition
                ds      1       ;bit 0: 0
                                ;bit 1: check cd
                                ;       0= don't care
                                ;       1= data available when cd HI
                                ;bit 2: check cs
                                ;       0= don't care
                                ;       1= data available when cs HI

sio_receive_port_control:       ;receive flow controled port
                ds      1       ;bit 0-3: 0
                                ;bit 4: rs  0= flow controled
                                ;           1= don't care
                                ;bit 5: rr
                                ;bit 6: er

sio_send_port_condition:        ;send data start port condition
                ds      1       ;bit 0: 0
                                ;bit 1: cd      0= don't care
                                ;               1= send start when cd HI
                                ;bit 2: cs      0= don't care
                                ;               1= send start when cs HI

sio_send_port_control:          ;send flow controled port
                ds      1       ;bit 0-3:
                                ;bit 4: rs  0= don't care
                                ;           1= output HI when data send
                                ;bit 5: rr
                                ;bit 6: er

sio_send_delay  ds      1       ;data WO SOUSHIN SURU ZENGO NI
                                ;IRERU wait time NO NAGASA.
                                ;unit is 2 msec.

sio_receive_wait:               ;sio WO file control DE TUKAU BAWAI NO work
                ds      1       ;data receive NO ATONO late data wait time
                                ;unit is 2 msec.

sio_crlf        ds      1       ;CR-LF code control
                                ;bit 1-0: 00= cr code convert to null
                                ;         01= cr code convert to 0dh
                                ;         10= cr code convert to 0ah
                                ;         11= cr code convert to 0dh+0ah

sio_eof_code    ds      1       ;sio file eof code

sio_error       ds      1       ;sio error copy
                                ;bit 0: parity error
                                ;bit 1: over run error
                                ;bit 2: framing error
                                ;bit 3: illegal condition
                                ;bit 4: buffer overflow
                                ;bit 5: time out
                                ;bit 6: low battery
                                ;bit 7: on/break key

sio_flag        ds      1       ;manage stack1,2/kanji status

sio_flag_receive:
                ds      1       ;manage eof/stack3

sio_open_close_wait:            ;sio WO file control DE TUKAU BAWAI NO work
                ds      1       ;open DE control line Hi NO ATO &
                                ;close DE control line Low NO MAE NO wait
                                ;unit is 5 msec.

sio_open_port_control:
                ds      1       ;open JI NI active NI SURU line
                                ;close JI NIWA non active NI SURU
                                ;bit 0-3:
                                ;bit 4: rs  0= don't care
                                ;           1= output HI when open
                                ;bit 5: rr
                                ;bit 6: er

;------ 
sio_send_nbyte_wait:            ;for iocs n_byte send routine.
                ds      1       ;2msec. * n wait after 1byte send

sio_open_send_data:             ;send data when sio open
                ds      1

sio_close_send_data:            ;send data when sio close
                ds      1

;------ 
sio_delimit_code   ds   1       ;internal delimit-code
                                ;code -> 0dh,0ah when sending
                                ;code <- 0dh,0ah when receiving
                                ;in case of specified [sio_crlf]bit7=1

;============================================================
;       system ds table by shintaku
;       time control work
;============================================================
;------------------------------------------------------------
        public  system_time_mode
        public  system_calendar_mode
        public  system_country_mode
;------------------------------------------------------------
system_time_mode        ds      1       ;clock_mode_stime(01h)
                                        ; 0:24h mode ,1:12h mode
                                        ;ymd_order_stime(02h)
                                        ; 0:MM/DD/YYYY ,1:DD/MM/YYYY

system_calendar_mode    ds      1       ;calender display format
                                        ; [left]  day of week
                                        ; 00h   : Sunday
                                        ; 01h   : Monday
                                        ;       v
                                        ; 06h   : Saturday

system_country_mode     ds      1       ;country code
                                        ; 0     :America
                                        ; european_mode(10h)    :Europe
                                        ;       10h - 1fh       :country
;------------------------------------------------------------
;
;============================================================
; alaram work
;                               by wada
;============================================================
        public  alarm_mark
        public  alarm_ymd
        public  alarm_time
;------------------------------------------------------------
alarm_mark      ds      1               ;alarm flag 80h=schedule alarm
                                        ;           40h=daily alarm
alarm_ymd       ds      8               ;alarm ymd(year,month,date)
alarm_time      ds      4               ;alarm time(hour,minute)

;============================================================
;       password & secret flag      by wada
;============================================================
        public  system_secret_mode
        public  _password_buf
;------ 
system_secret_mode      ds      1       ;secret mode.
;                                       ;b7=1:secret on
;                                       ;  =0:secret off

_password_buf           ds      7       ;password string 7 bytes.

;------------------------------------------------------------
;  data end hook  ( in data store time )
;
;        SUBETENO application NI OITE data WO NYUURYOKU
;       SURUBAMENDE enter key WO OSARETA TOKINI KONO hook
;       WO callf SURU.
;        KONO hook NI YOTTE, KAKU application GA memory HE
;       KAKUNOU SHIYOUTO SHITEIRU data WO monitering SURU KOTO
;       GA DEKIRU. (ex.  for the saurus)
;        hook NIWA 2 pattern ARI, SORE ZORE NI hook GA 1 TUZUTU ARI,
;               1. on buffer controled by Editor Core
;               2. on memory no-controled by Editor Core
;       1. NO BAWAI HA editor core WO KIDOU SHITAMAMA KONO hook WO
;       call SURU. 2. NO BAWAI HA KAKUNOU SURU data NO SENTOU WO
;       x reg. DE SASHITE, KONO hook WO call SURU.
;       
;                               shiraishi.N
;------------------------------------------------------------
        public  enter_ed_hook
        public  enter_x_hook
;------ 
enter_ed_hook   ds      4               ;editor controled data
enter_x_hook    ds      4               ;no-controled data

;
;============================================================
; main ram stack pointer push work
;               for stack pointer pop on reset
;============================================================
        public  main_u_stack_top
        public  main_u_stack_btm
        public  main_s_stack_top
        public  main_s_stack_btm
        public  mb_flag

;------ 
main_u_stack_top        ds      3       ;last set u_stack
main_u_stack_btm        ds      3
main_s_stack_top        ds      3       ;last set s_stack
main_s_stack_btm        ds      3
mb_flag                 ds      1       ;mb.iocs mode flag.

