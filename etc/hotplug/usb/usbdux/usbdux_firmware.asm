;   usbdux_firmware.asm
;   Copyright (C) 2004 Bernd Porr, Bernd.Porr@cn.stir.ac.uk
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;
; Firmware: usbdux_firmware.asm for usbdux.c
; Description: University of Stirling USB DAQ & INCITE Technology Limited
; Devices: [ITL] USB-DUX (usbdux.o)
; Author: Bernd Porr <Bernd.Porr@cn.stir.ac.uk>
; Updated: 25 Jan 2004
; Status: testing
;
;;;
;;;
;;;

	.inc	fx2-include.asm

	.equ	CHANNELLIST,80h	; channellist in indirect memory
	
	.equ	DIOFLAG,90h	; flag if next IN transf is DIO
		
	.org	0000h		; after reset the processor starts here
	ljmp	main		; jump to the main loop

	.org	0043h		; the IRQ2-vector
	ljmp	jmptbl		; irq service-routine
	
	.org	0100h		; start of the jump table

jmptbl:	ljmp	sudav_isr
	nop
	ljmp	sof_isr
	nop
	ljmp	sutok_isr
	nop
	ljmp	suspend_isr
	nop
	ljmp	usbreset_isr
	nop
	ljmp	hispeed_isr
	nop
	ljmp	ep0ack_isr
	nop
	ljmp	spare_isr
	nop
	ljmp	ep0in_isr
	nop
	ljmp	ep0out_isr
	nop
	ljmp	ep1in_isr
	nop
	ljmp	ep1out_isr
	nop
	ljmp	ep2_isr
	nop
	ljmp	ep4_isr
	nop
	ljmp	ep6_isr
	nop
	ljmp	ep8_isr
	nop
	ljmp	ibn_isr
	nop
	ljmp	spare_isr
	nop
	ljmp	ep0ping_isr
	nop
	ljmp	ep1ping_isr
	nop
	ljmp	ep2ping_isr
	nop
	ljmp	ep4ping_isr
	nop
	ljmp	ep6ping_isr
	nop
	ljmp	ep8ping_isr
	nop
	ljmp	errlimit_isr
	nop
	ljmp	spare_isr
	nop
	ljmp	spare_isr
	nop
	ljmp	spare_isr
	nop
	ljmp	ep2isoerr_isr
	nop
	ljmp	ep4isoerr_isr
	nop
	ljmp	ep6isoerr_isr
	nop
	ljmp	ep8isoerr_isr

	
	;; dummy isr
sudav_isr:	
sutok_isr:	
suspend_isr:	
usbreset_isr:	
hispeed_isr:	
ep0ack_isr:	
spare_isr:	
ep0in_isr:	
ep0out_isr:	
ep1in_isr:	
ep1out_isr:	
ibn_isr:	
ep0ping_isr:	
ep1ping_isr:	
ep2ping_isr:	
ep4ping_isr:	
ep6ping_isr:	
ep8ping_isr:	
errlimit_isr:	
ep2isoerr_isr:	
ep4isoerr_isr:	
ep6isoerr_isr:	
ep8isoerr_isr:

	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw

	;; clear the USB2 irq bit and return
	mov	a,EXIF
	clr	acc.4
	mov	EXIF,a

	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	
	reti

		
;;; main program
;;; basically only initialises the processor and
;;; then engages in an endless loop
main:
	mov	DPTR,#CPUCS	; CPU control register
	mov	a,#000100100b	; 48MHz clock
	movx	@DPTR,a		; do it

	mov	dptr,#INTSETUP	; IRQ setup register
	mov	a,#08h		; enable autovector
	movx	@DPTR,a		; do it

	lcall	initAD		; init the ports to the converters

	lcall	inieplo		; init the isochronous data-transfer

mloop2:	nop

	sjmp	mloop2		; do nothing. The rest is done by the IRQs


;;; initialise the ports for the AD-converter
initAD:
	mov	OEA,#27H	;PortA0,A1,A2,A5 Outputs
	mov	IOA,#22H	;/CS = 1, disable transfers to the converters
	ret




;;; from here it's only IRQ handling...
	
;;; A/D-conversion:
;;; control-byte in a,
;;; result in r3(low) and r4(high)
;;; this routine is optimised for speed
readAD:				; mask the control byte
	anl	a,#01111100b	; only the channel, gain+pol are left
	orl	a,#10000001b	; start bit, external clock
	;; set CS to low
	clr	IOA.1		; set /CS to zero
	;; send the control byte to the AD-converter
	mov 	R2,#8		; bit-counter
S1:     jnb     ACC.7,bitzero	; jump if Bit7 = 0?
	setb	IOA.2		; set the DIN bit
	sjmp	clock		; continue with the clock
bitzero:clr	IOA.2		; clear the DIN bit
clock:	setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
        rl      a               ; next Bit
        djnz    R2,S1

	;; continue the aquisition (already started)
	clr	IOA.2		; clear the DIN bit
	mov 	R2,#5		; five steps for the aquision
clockaq:setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
        djnz    R2,clockaq	; loop
	
	;; read highbyte from the A/D-converter
	;; and do the conversion
	mov	r4,#0 		; Highbyte goes into R4
	mov	R2,#4		; COUNTER 4 data bits in the MSB
	mov	r5,#08h		; create bit-mask
gethi:				; loop get the 8 highest bits from MSB downw
	setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
	mov	a,IOA		; from port A
	jnb	ACC.4,zerob	; the in-bit is zero
	mov	a,r4		; get the byte
	orl	a,r5		; or the bit to the result
	mov	r4,a		; save it again in r4
zerob:	mov	a,r5		; get r5 in order to shift the mask
	rr	a		; rotate right
	mov	r5,a		; back to r5
	djnz	R2,gethi
	;; read the lowbyte from the A/D-converter
	mov	r3,#0 		; Lowbyte goes into R3
	mov	r2,#8		; COUNTER 8 data-bits in the LSB
	mov	r5,#80h		; create bit-mask
getlo:				; loop get the 8 highest bits from MSB downw
	setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
	mov	a,IOA		; from port A
	jnb	ACC.4,zerob2	; the in-bit is zero
	mov	a,r3		; get the result-byte
	orl	a,r5		; or the bit to the result
	mov	r3,a		; save it again in r4
zerob2:	mov	a,r5		; get r5 in order to shift the mask
	rr	a		; rotate right
	mov	r5,a		; back to r5
	djnz	R2,getlo
	setb	IOA.1		; set /CS to one
	;;
	ret
	

	
;;; aquires data from all 8 channels and stores it in the EP6 buffer
convlo:	;;
	mov	AUTOPTRH1,#0F8H	; EP6
	mov	AUTOPTRL1,#00H
	mov	AUTOPTRSETUP,#7
	mov	r0,#CHANNELLIST	; points to the channellist
	mov 	a,@r0		;Ch0
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0		; next channel
	mov 	a,@r0		;Ch1
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch2
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch3
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch4
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch5
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch6
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A

	inc	r0
	mov 	a,@r0		;Ch7
	lcall 	readAD
	mov 	a,R3		;
	mov 	DPTR,#XAUTODAT1
	movx 	@DPTR,A
	mov 	a,R4		;
	movx 	@DPTR,A
	ret




;;; initilise the transfer for full speed
;;; It is assumed that the USB interface is in alternate setting 3
inieplo:
	mov	dptr,#FIFORESET
	mov	a,#0fh		
	movx	@dptr,a		; reset all fifos
	mov	a,#00h		
	movx	@dptr,a		; normal operat
	
	mov	DPTR,#EP2CFG
	mov	a,#10010010b	; valid, out, double buff, iso
	movx	@DPTR,a

	mov	dptr,#EP2FIFOCFG
	mov	a,#00000000b	; manual
	movx	@dptr,a

	mov	dptr,#EP2BCL	; "arm" it
	mov	a,#80h
	movx	@DPTR,a		; can receive data
	movx	@DPTR,a		; can receive data
	movx	@DPTR,a		; can receive data
	
	mov	DPTR,#EP4CFG
	mov	a,#10100000b	; valid
	movx	@dptr,a

	mov	dptr,#EP4FIFOCFG
	mov	a,#00000000b	; manual
	movx	@dptr,a

	mov	dptr,#EP4BCL	; "arm" it
	mov	a,#80h
	movx	@DPTR,a		; can receive data
	movx	@dptr,a		; make shure its really empty

	mov	DPTR,#EP6CFG	; ISO data from here to the host
	mov	a,#11010010b	; Valid
	movx	@DPTR,a		; ISO transfer, double buffering

	mov	DPTR,#EP8CFG	; EP8
	mov	a,#11100000b	; BULK data from here to the host
	movx	@DPTR,a		;

	mov	dptr,#EPIE	; interrupt enable
	mov	a,#10100000b	; enable irq for ep4,8
	movx	@dptr,a		; do it

	mov	dptr,#EPIRQ	; clear IRQs
	mov	a,#10100000b
	movx	@dptr,a

	;; enable interrups
        mov     DPTR,#USBIE	; USB int enables register
        mov     a,#2            ; enables SOF (1ms/125us interrupt)
        movx    @DPTR,a         ; 

	mov	EIE,#00000001b	; enable INT2 in the 8051's SFR
	mov	IE,#80h		; IE, enable all interrupts

	lcall	ep8_arm		; 
	
	ret


;;; interrupt-routine for SOF
;;; is for full speed
sof_isr:
	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw
	push	00h		; R0
	push	01h		; R1
	push	02h		; R2
	push	03h		; R3
	push	04h		; R4
	push	05h		; R5
	push	06h		; R6
	push	07h		; R7
		
	mov	a,EP2468STAT
	anl	a,#20H		; full?
	jnz	epfull		; EP6-buffer is full

	lcall	convlo		; conversion

	mov	DPTR,#EP6BCH	; byte count H
	mov	a,#0		; is zero
	movx	@DPTR,a
	
	mov	DPTR,#EP6BCL	; byte count L
	mov	a,#10H		; is 8x word = 16 bytes
	movx	@DPTR,a
	
epfull:
	;; do the D/A conversion
	mov	a,EP2468STAT
	anl	a,#01H		; empty
	jnz	epempty		; nothing to get

	mov	dptr,#0F000H	; EP2 fifo buffer
	lcall	dalo		; conversion

	mov	dptr,#EP2BCL	; "arm" it
	mov	a,#80h
	movx	@DPTR,a		; can receive data
	movx	@dptr,a

epempty:	
	;; clear INT2
	mov	a,EXIF		; FIRST clear the USB (INT2) interrupt request
	clr	acc.4
	mov	EXIF,a		; Note: EXIF reg is not 8051 bit-addressable
	
	mov	DPTR,#USBIRQ	; points to the SOF
	mov	a,#2		; clear the SOF
	movx	@DPTR,a

nosof:	
	pop	07h
	pop	06h
	pop	05h
	pop	04h		; R4
	pop	03h		; R3
	pop	02h		; R2
	pop	01h		; R1
	pop	00h		; R0
	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	reti





;;; interrupt-routine for ep4
;;; receives the channel list and other commands
ep4_isr:
	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw
	push	00h		; R0
	push	01h		; R1
	push	02h		; R2
	push	03h		; R3
	push	04h		; R4
	push	05h		; R5
	push	06h		; R6
	push	07h		; R7
		
	mov	dptr,#0f400h	; FIFO buffer of EP4
	movx	a,@dptr		; get the first byte

	mov	dptr,#ep4_jmp	; jump table for the different functions
	rl	a		; multiply by 2: sizeof sjmp
	jmp	@a+dptr		; jump to the jump table
ep4_jmp:
	sjmp	storechannellist; a=0
	sjmp	single_da	; a=1
	sjmp	config_digital_b; a=2
	sjmp	write_digital_b	; a=3


;;; Channellist:	
;;; the first byte is zero:
;;; we've just received the channel list
;;; the channel list is stored in the addresses from 80H which
;;; are _only_ reachable by indirect addressing
storechannellist:
	mov	r0,#CHANNELLIST	; the conversion bytes are now stored in 80h
	mov	r2,#8		; counter
	mov	dptr,#0f401h	; FIFO buffer of EP4
chanlloop:	
	movx	a,@dptr		; 
	mov	@r0,a
	inc	dptr
	inc	r0
	djnz	r2,chanlloop
	clr	a		; announce analogue transaction
	mov	r0,#DIOFLAG	; pointer to the command byte
	mov 	@r0,a		; set the command byte
	sjmp	over_da

;;; Single DA conversion. The 2 bytes are in the FIFO buffer
single_da:
	mov	dptr,#0f401h	; FIFO buffer of EP4
	lcall	dalo		; conversion
	sjmp	over_da

;;; configure the port B
config_digital_b:
	mov	dptr,#0f401h	; FIFO buffer of EP4
	movx	a,@dptr		; get the second byte
	mov	OEB,a		; set the output enable bits
	sjmp	over_da
	
;;; Write one byte to the external digital port B
;;; and prepare for digital read
write_digital_b:
	mov	dptr,#0f401h	; FIFO buffer of EP4
	movx	a,@dptr		; get the second byte
	mov	OEB,a		; output enable
	inc	dptr		; next byte
	movx	a,@dptr		; bits
	mov	IOB,a		; send the byte to the I/O port
	mov	a,#0ffh		; announce DIO transaction
	mov	r0,#DIOFLAG	; pointer to the command byte
	mov 	@r0,a		; set the command byte
	sjmp	over_da

;;; more things here to come...
	
over_da:	
	mov	dptr,#EP4BCL
	mov	a,#80h
	movx	@DPTR,a		; arm it
	movx	@DPTR,a		; arm it
	movx	@DPTR,a		; arm it

	;; clear INT2
	mov	a,EXIF		; FIRST clear the USB (INT2) interrupt request
	clr	acc.4
	mov	EXIF,a		; Note: EXIF reg is not 8051 bit-addressable

	mov	DPTR,#EPIRQ	; 
	mov	a,#00100000b	; clear the ep4irq
	movx	@DPTR,a

	pop	07h
	pop	06h
	pop	05h
	pop	04h		; R4
	pop	03h		; R3
	pop	02h		; R2
	pop	01h		; R1
	pop	00h		; R0
	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	reti


	
;;; all channels
dalo:
	movx	a,@dptr		; number of channels
	inc	dptr		; pointer to the first channel
	mov	r0,a		; 4 channels
nextDA:	
	movx	a,@dptr		; get the first low byte
	mov	r3,a		; store in r3 (see below)
	inc	dptr		; point to the high byte
	movx	a,@dptr		; get the high byte
	mov	r4,a		; store in r4 (for writeDA)
	inc	dptr		; point to the channel number
	movx	a,@dptr		; get the channel number
	inc	dptr		; get ready for the next channel
	lcall	writeDA		; write value to the DAC
	djnz	r0,nextDA	; next channel
	ret



;;; D/A-conversion:
;;; control-byte in a,
;;; value in r3(low) and r4(high)
writeDA:			; mask the control byte
	anl	a,#11000000b	; only the channel is left
	orl	a,#00110000b	; internal clock, bipolar mode, +/-5V
	orl	a,r4		; or the value of R4 to it
	;; set CS to low
	clr	IOA.5		; set /CS to zero
	;; send the first byte to the DA-converter
	mov 	R2,#8		; bit-counter
DA1:    jnb     ACC.7,zeroda	; jump if Bit7 = 0?
	setb	IOA.2		; set the DIN bit
	sjmp	clkda		; continue with the clock
zeroda: clr	IOA.2		; clear the DIN bit
clkda:	setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
        rl      a               ; next Bit
        djnz    R2,DA1

	
	;; send the second byte to the DA-converter
	mov	a,r3		; low byte
	mov 	R2,#8		; bit-counter
DA2:    jnb     ACC.7,zeroda2	; jump if Bit7 = 0?
	setb	IOA.2		; set the DIN bit
	sjmp	clkda2		; continue with the clock
zeroda2:clr	IOA.2		; clear the DIN bit
clkda2:	setb	IOA.0		; SCLK = 1
	clr	IOA.0		; SCLK = 0
        rl      a               ; next Bit
        djnz    R2,DA2
	;; 
	setb	IOA.5		; set /CS to one
	;; 
noDA:	ret
	


;;; arm ep6
ep6_arm:
	lcall	convlo
	
	mov	DPTR,#EP6BCH	; byte count H
	mov	a,#0		; is zero
	movx	@DPTR,a
	
	mov	DPTR,#EP6BCL	; byte count L
	mov	a,#10H		; is one
	movx	@DPTR,a
	ret
	

	
;;; get all 8 channels in the high speed mode
;;; not used just now
ep6_isr:	
	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw
	push	00h		; R0
	push	01h		; R1
	push	02h		; R2
	push	03h		; R3
	push	04h		; R4
	push	05h		; R5
	push	06h		; R6
	push	07h		; R7
		
	lcall	convlo		; conversion

	mov	DPTR,#EP6BCH	; byte count H
	mov	a,#0		; is zero
	movx	@DPTR,a
	
	mov	DPTR,#EP6BCL	; byte count L
	mov	a,#10H		; is 8x word = 16 bytes
	movx	@DPTR,a
	
	;; clear INT2
	mov	a,EXIF		; FIRST clear the USB (INT2) interrupt request
	clr	acc.4
	mov	EXIF,a		; Note: EXIF reg is not 8051 bit-addressable

	mov	DPTR,#EPIRQ	; 
	mov	a,#01000000b	; clear the ep6irq
	movx	@DPTR,a

	pop	07h
	pop	06h
	pop	05h
	pop	04h		; R4
	pop	03h		; R3
	pop	02h		; R2
	pop	01h		; R1
	pop	00h		; R0
	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	reti


	
;;; converts one analog/digital channel and stores it in EP8
;;; also gets the content of the digital ports B and D
ep8_adc:
	mov	r0,#DIOFLAG	; pointer to the DIO flag
	mov 	a,@r0		; get the flag
	jnz	ep8_dio		; nonzero means DIO

	mov	r0,#CHANNELLIST	; points to the channellist
	mov 	a,@r0		; Ch0
	
	lcall 	readAD		; start the conversion
		
	mov 	DPTR,#0fc00h	; EP8 FIFO 
	mov 	a,R3		; get low byte
	movx 	@DPTR,A		; store in FIFO
	inc	dptr		; next fifo entry
	mov 	a,R4		; get high byte
	movx 	@DPTR,A		; store in FIFO

	sjmp	ep8_send	; send the data

ep8_dio:	
	mov 	DPTR,#0fc00h	; store the contents of port B
	mov	a,IOB		; in the next
	movx	@dptr,a		; entry of the buffer

	inc	dptr
	clr	a		; high byte is zero
	movx	@dptr,a		; next byte of the EP
	
ep8_send:	
	mov	DPTR,#EP8BCH	; byte count H
	mov	a,#0		; is zero
	movx	@DPTR,a
	
	mov	DPTR,#EP8BCL	; byte count L
	mov	a,#10H		; 16 bytes
	movx	@DPTR,a		; send the data over to the host
	ret


	
;;; arms EP8 with one byte. This signals the Linux driver that
;;; the EP has been armed only with a dummy byte to make the
;;; IRQ work. The byte is not processed by the driver.
ep8_arm:	
	mov	DPTR,#EP8BCH	; byte count H
	mov	a,#0		; is zero
	movx	@DPTR,a
	
	mov	DPTR,#EP8BCL	; byte count L
	mov	a,#1		; 1 byte
	movx	@DPTR,a
	ret	



;;; EP8 interrupt: gets one measurement from the AD converter and
;;; sends it via EP8. The channel # is stored in address 80H.
;;; It also gets the state of the digital registers B and D.
ep8_isr:	
	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw
	push	00h		; R0
	push	01h		; R1
	push	02h		; R2
	push	03h		; R3
	push	04h		; R4
	push	05h		; R5
	push	06h		; R6
	push	07h		; R7
		
	lcall	ep8_adc
	
	;; clear INT2
	mov	a,EXIF		; FIRST clear the USB (INT2) interrupt request
	clr	acc.4
	mov	EXIF,a		; Note: EXIF reg is not 8051 bit-addressable

	mov	DPTR,#EPIRQ	; 
	mov	a,#10000000b	; clear the ep8irq
	movx	@DPTR,a

	pop	07h
	pop	06h
	pop	05h
	pop	04h		; R4
	pop	03h		; R3
	pop	02h		; R2
	pop	01h		; R1
	pop	00h		; R0
	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	reti






;;; high speed mode, IRQ mode. Asynchronous transmission.
;;; not used just now
ep2_isr:
	push	dps
	push	dpl
	push	dph
	push	dpl1
	push	dph1
	push	acc
	push	psw
	push	00h		; R0
	push	01h		; R1
	push	02h		; R2
	push	03h		; R3
	push	04h		; R4
	push	05h		; R5
	push	06h		; R6
	push	07h		; R7
		
	mov	dptr,#0F000H	; EP2 fifo buffer
	lcall	dalo		; conversion

	mov	dptr,#EP2BCL	; "arm" it
	mov	a,#80h
	movx	@DPTR,a		; can receive data
	movx	@dptr,a	

	;; clear INT2
	mov	a,EXIF		; FIRST clear the USB (INT2) interrupt request
	clr	acc.4
	mov	EXIF,a		; Note: EXIF reg is not 8051 bit-addressable
	
	mov	DPTR,#EPIRQ	; points to the usbirq
	mov	a,#00010000b	; clear the usbirq of ep 2
	movx	@DPTR,a

	pop	07h
	pop	06h
	pop	05h
	pop	04h		; R4
	pop	03h		; R3
	pop	02h		; R2
	pop	01h		; R1
	pop	00h		; R0
	pop	psw
	pop	acc 
	pop	dph1 
	pop	dpl1
	pop	dph 
	pop	dpl 
	pop	dps
	reti









.End


