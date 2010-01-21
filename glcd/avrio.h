/************************************************************************************************
 *	AVRIO
 * Copyright (C) 2009 Bill Perry. (bill@billsworld.billandterrie.com)
 *
 * vi:ts=4
 *
 *  AVRIO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License (or the Lesser GPL).
 *  as published by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AVRIO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with AVRIO.  If not, see:
 *	http://www.gnu.org/licenses/lgpl-3.0-standalone.html
 *	http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *
 *	AVRIO defines a set of i/o primitives that work on AVR pins.
 *
 *
 *	These primitives are for setting/reading digital levels on the pins.
 *
 *	It introduces a new AVR s/w i/o concept.
 *	It is wrapped around the concept of an "avrpin".
 *	An avrpin is a compile-time token that can be used to locate a particular avrpin.
 *	It is different than the simple integer scheme as currently used by Arduino.
 *	avrpin has encoded into it: the port, A, B, C, etc, as well as the bit information
 *	within that port.
 *
 *	What is intentionally not encoded in it is the actual register type 
 *	of choice: DDR, PIN, or PORT.
 *	Therefore, the register type of choice must be indicated to the primitives using
 *	the correspending values: AVRIO_DDRREG, AVRIO_PINREG, AVRIO_PORTREG
 *
 *
 *	Special Note: While avrpin format is very different from arduino pin#s,
 *					in arduino environments, avrio will map arduino pin#s to avrpins
 *					on the fly so arduino users can use arduino pin#s with the primitives.
 *					This capability requires the use a mapping macro digitalPinToPortReg()
 *					which should be provided by an inluding an arduino header file.
 *					That header file *MUST* be included before including this header.
 *
 *	Primitives:
 *
 *	bitval = avrio_ReadBit(regtyp, pin)			
 *	byteval = avrio_Read8Bits(regtyp, pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)
 *	byteval = avrio_ReadReg(regtyp, avrport)			
 *
 *	avrio_WriteBit(regtyp, pin, bitval)			
 *	avrio_Write8Bits(regtyp, pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, data)
 *	avrio_WriteReg(regtyp, avrport, byteval)			
 *
 *------------
 *
 *	The following avrio primitives are convenience macros:
 *	They simply call the above primitives with an implied register type.
 *	PinMode writes to DDR	dir 1 = output, dir 0 = input
 *	ReadPin reads from PIN
 *	WritePin writes to PORT
 *
 *		avrio_PinMode(pin, dir)
 *		avrio_PinMode8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, dir)
 *
 *		bitval = avrio_ReadPin(pin)
 *		byteval = avrio_Read8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)
 *
 *		avrio_WritePin(pin, bitval)
 *		avrio_Write8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, byteval)
 *
 *------------
 *
 *	The following primitives are provideed as "convenience" to be similar to the Arduino pin
 *	primitives: 
 *
 *		avrio_pinMode(pin, dir)
 *		avrio_pinMode8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, dir)
 *
 *		avrio_digitalWrite(pin, pinval)
 *		avrio_digitalWritepin(pin, pinval)
 *		avrio_digitalWrite8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, byteval)
 *
 *
 *		pinval = avrio_digitalRead(pin)
 *		pinval = avrio_digitalReadPin(pin)
 *		byteval = avrio_digitalRead8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)
 *----------------------------------------------------------------------------------
 *	NOTE:
 *		For a full description of the calling sequence for each primitive
 *		see the comments below for each primitive.
 *
 *
 *----------------------------------------------------------------------------------
 *
 *	WARNING:
 *
 *		These routines are based on gcc inline functions and are currently designed
 *		for arguments that are constants. If non constants are used, terrible code will
 *		be generated. However, with constants, the code will generate the smallest possible
 *		code including using using CBI/SBI instructions when possible.
 *
 *		These primitives do not do any type of handholding or altering of the
 *		of the port configuration bits. Bits are written to the desired registers.
 *		No more, no less.
 *
 *
 *	To support the arduino environment, a mapping from arduino pin#s is automatically performed.
 *	This will be done if the macro digitalPinToPortReg() exists.
 *
 *
 */
/************************************************************************************************/

#ifndef _AVRIO_AVRIO_
#define _AVRIO_AVRIO_

#define AVRIO_PORTA	0xa
#define AVRIO_PORTB	0xb
#define AVRIO_PORTC	0xc
#define AVRIO_PORTD	0xd
#define AVRIO_PORTE	0xe
#define AVRIO_PORTF	0xf

#define AVRIO_PORTG	0x10
#define AVRIO_PORTH	0x11
#define AVRIO_PORTI	0x12
#define AVRIO_PORTJ	0x13
#define AVRIO_PORTK	0x14
#define AVRIO_PORTL	0x15


#define AVRIO_DDRREG		1
#define AVRIO_PORTREG		2
#define AVRIO_PINREG		3

typedef uint16_t avrpin_t;	/* This must be uint16_t to support ports larger than F	*/

/*
 * Generates an avrpin #
 *	The current encoding allows the use of naked constants for
 *	ports a-f.
 *
 *	So for talking to port D pin 2 the avrpin # would be 0xd2
 *	That said, it is recommended to use the PIN_Pb defines,
 *	for future compatibility should the encoding change.
 *	so rather than use 0xd2 use PIN_D2
 */

#define AVRIO_PIN(avrport, avrbit) ((avrport << 4) | avrbit)

#define AVRIO_PIN2AVRPORT(pin)		(AVRIO_PIN2AVRPIN(pin) >> 4)
#define AVRIO_PIN2AVRBIT(pin)		(AVRIO_PIN2AVRPIN(pin) & 0xf)
#define AVRIO_PIN2AVRBITMASK(pin)	_BV((AVRIO_PIN2AVRBIT(pin)))



/*
 * Atomagically detect Arduino pin mapping macros.
 *	This allows automatic mapping of arduino pin#s to avrpin #s.
 *
 *	Because the mapping is automatic, either numbering space can be used or
 *	even both.
 */

#ifdef ARDUINO_PIN2AVRPIN
#define AVRIO_PIN2AVRPIN(pin)		((pin < AVRIO_PIN(AVRIO_PORTA, 0)) ? ARDUINO_PIN2AVRPIN(pin) : pin)
#endif

#ifdef digitalPinToPortReg
static inline avrpin_t  avrio_arduino_pin2avrpin(uint8_t pin) __attribute__((always_inline));

#define AVRIO_PIN2AVRPIN(pin)		((pin < AVRIO_PIN(AVRIO_PORTA, 0)) ? avrio_arduino_pin2avrpin(pin) : pin)

avrpin_t
avrio_arduino_pin2avrpin(uint8_t pin)
{
#ifdef PORTA
	if(digitalPinToPortReg(pin) == &PORTA)
		return(AVRIO_PIN(AVRIO_PORTA, digitalPinToBit(pin)));
#endif
#ifdef PORTB
	if(digitalPinToPortReg(pin) == &PORTB)
		return(AVRIO_PIN(AVRIO_PORTB, digitalPinToBit(pin)));
#endif
#ifdef PORTC
	if(digitalPinToPortReg(pin) == &PORTC)
		return(AVRIO_PIN(AVRIO_PORTC, digitalPinToBit(pin)));
#endif
#ifdef PORTD
	if(digitalPinToPortReg(pin) == &PORTD)
		return(AVRIO_PIN(AVRIO_PORTD, digitalPinToBit(pin)));
#endif
#ifdef PORTE
	if(digitalPinToPortReg(pin) == &PORTE)
		return(AVRIO_PIN(AVRIO_PORTE, digitalPinToBit(pin)));
#endif
#ifdef PORTF
	if(digitalPinToPortReg(pin) == &PORTF)
		return(AVRIO_PIN(AVRIO_PORTF, digitalPinToBit(pin)));
#endif
#ifdef PORTG
	if(digitalPinToPortReg(pin) == &PORTG)
		return(AVRIO_PIN(AVRIO_PORTG, digitalPinToBit(pin)));
#endif
#ifdef PORTH
	if(digitalPinToPortReg(pin) == &PORTH)
		return(AVRIO_PIN(AVRIO_PORTH, digitalPinToBit(pin)));
#endif
#ifdef PORTI
	if(digitalPinToPortReg(pin) == &PORTI)
		return(AVRIO_PIN(AVRIO_PORTI, digitalPinToBit(pin)));
#endif
#ifdef PORTJ
	if(digitalPinToPortReg(pin) == &PORTJ)
		return(AVRIO_PIN(AVRIO_PORTJ, digitalPinToBit(pin)));
#endif
#ifdef PORTK
	if(digitalPinToPortReg(pin) == &PORTK)
		return(AVRIO_PIN(AVRIO_PORTK, digitalPinToBit(pin)));
#endif
#ifdef PORTL
	if(digitalPinToPortReg(pin) == &PORTL)
		return(AVRIO_PIN(AVRIO_PORTL, digitalPinToBit(pin)));
#endif
	return(-1);
}

#endif


#ifndef AVRIO_PIN2AVRPIN
#define AVRIO_PIN2AVRPIN(pin)		pin
#endif


/************************************************************************************************
 *
 * 			Convenience primitives start here
 *
 ************************************************************************************************/

/*
 * Define the avrio convenience primitives.
 */
#define avrio_PinMode(pin, dir) 		avrio_WriteBit(AVRIO_DDRREG, pin, dir)
#define avrio_PinMode8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7) \
 	 avrio_WriteBits(AVRIO_DDRREG, pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)

#define avrio_ReadPin(pin) 			avrio_ReadBit(AVRIO_PINREG, pin)
#define avrio_Read8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7) \
 	 avrio_Read8Bits(AVRIO_PINREG, pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)

#define avrio_WritePin(pin, data)		avrio_WriteBit(AVRIO_PORTREG, pin, data)
#define avrio_Write8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7) \
 	 avrio_WriteBits(AVRIO_PORTREG, pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)


/*
 * Define the Arduino "compatible" primitives
 */
#define avrio_digitalRead(pin)			avrio_ReadPin(pin)
#define avrio_digitalReadPin(pin)		avrio_ReadPin(pin)
#define avrio_digitalRead8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7) \
 	 avrio_Read8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)

#define avrio_digitalWrite(pin, data)		avrio_WritePin(pin, data)
#define avrio_digitalWritePin(pin, data)	avrio_WritePin(pin, data)
#define avrio_digitalWrite8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7) \
 	 avrio_Write8Pins(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7)


#define avrio_pinMode(pin, dir) avrio_PinMode(pin, dir)
#define avrio_pinMode8Pins(pin, dir) avrio_PinMode8Pins(pin, dir)


/************************************************************************************************
 *
 * 			User primitives start here
 *
 ************************************************************************************************/

/************************************************************************
 * PRIMITIAVE: avrio_WriteBit - set a bit/pin value
 *
 * SYNOPSIS:
 * 	void avrio_WriteBit(uint8_t regtyp, avrpin_t avrpin, uint8_t val);
 *
 * INPUT PARAMETERS:
 *	uint8_t regtyp;
 *		specifies the register type, either DDR, PORT or PIN.
 *		Must be one of: AVRIO_DDRREG, AVRIO_PORTREG, AVRIO_PINREG
 *
 *	avrpin_t avrpin;
 *		This value is an "avrpin" value *not* a simple pin value.
 *		(see notes below for details)
 *
 *	uint8_t val;
 *		If 0 bit is set to 0, if non zero, bit is set to 1.
 *		
 *
 * OUTPUT PARAMETERS:
 *		(none)
 *
 * DESCRIPTION:
 *	This function sets a value to a i/o register bit or a pin. The desired register
 *	type (DDR, PORT, or PIN) can be specified.
 *
 *	The avrpin paramter will automaticaly adapt to different environments and
 *	suports multiple different style defintions.
 *
 *	In non arduino enviroments, this value should be specified using the
 *	pin defintions below. Direct pin values can be specified using a
 *	PIN_Pb definition. This means to speccify port D bit 3 would be PIN_D3
 *
 *	Arduino users may use either arduino pin#s, or the PIN_Pb style naming.
 *	
 *
 *
 * RETURN VALUE:
 *		(none)
 *
 * NOTES/WARNINGS:
 *	This function can only be used with constants. If any argument is not a constant,
 *	terrible code that is very large will result.
 *
 *	If arduno pin#s are used for avrpin and if the header that defines the macro:
 *	digitalPinToPortReg() is not included, incorrect/invalid code will silently be generated.
 *	
 *	
 *
 * SEE ALSO:
 *	avrio_Write8Bits
 *
 ************************************************************************/

static inline void avrio_WriteBit(uint8_t regtyp, avrpin_t pin, uint8_t val) __attribute__((always_inline));

void
avrio_WriteBit(uint8_t regtyp, avrpin_t avrpin, uint8_t val)					
{									
        if (val) {                                                      
                if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTA) {				
#ifdef PORTA
			if(regtyp == AVRIO_DDRREG)
				DDRA |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINA |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTA |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTB
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTB) {				
			if(regtyp == AVRIO_DDRREG)
				DDRB |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINB |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTB |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTC
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTC) {				
			if(regtyp == AVRIO_DDRREG)
				DDRC |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINC |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTC |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTD
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTD) {				
			if(regtyp == AVRIO_DDRREG)
				DDRD |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PIND |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTD |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTE
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTE) {				
			if(regtyp == AVRIO_DDRREG)
				DDRE |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINE |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTE |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTF
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTF) {				
			if(regtyp == AVRIO_DDRREG)
				DDRF |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINF |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTF |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTG
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTG) {				
			if(regtyp == AVRIO_DDRREG)
				DDRG |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PING |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTG |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTH
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTH) {				
			if(regtyp == AVRIO_DDRREG)
				DDRH |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINH |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTH |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTI
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTI) {				
			if(regtyp == AVRIO_DDRREG)
				DDRI |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINI |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTI |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTJ
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTJ) {				
			if(regtyp == AVRIO_DDRREG)
				DDRJ |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINJ |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTJ |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTK
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTK) {				
			if(regtyp == AVRIO_DDRREG)
				DDRK |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINK |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTK |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTL
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTL) {				
			if(regtyp == AVRIO_DDRREG)
				DDRL |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINL |= AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTL |= AVRIO_PIN2AVRBITMASK(avrpin);
#endif
                } else {                                                
                }                                                       
        } else {                                                        
                if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTA) {				
#ifdef PORTA
			if(regtyp == AVRIO_DDRREG)
				DDRA &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINA &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTA &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTB
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTB) {				
			if(regtyp == AVRIO_DDRREG)
				DDRB &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINB &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTB &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTC
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTC) {				
			if(regtyp == AVRIO_DDRREG)
				DDRC &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINC &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTC &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTD
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTD) {				
			if(regtyp == AVRIO_DDRREG)
				DDRD &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PIND &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTD &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTE
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTE) {				
			if(regtyp == AVRIO_DDRREG)
				DDRE &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINE &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTE &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTF
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTF) {
			if(regtyp == AVRIO_DDRREG)
				DDRF &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINF &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTF &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTG
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTG) {
			if(regtyp == AVRIO_DDRREG)
				DDRG &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PING &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTG &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTH
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTH) {
			if(regtyp == AVRIO_DDRREG)
				DDRH &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINH &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTH &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTI
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTI) {
			if(regtyp == AVRIO_DDRREG)
				DDRI &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINI &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTI &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTJ
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTJ) {				
			if(regtyp == AVRIO_DDRREG)
				DDRJ &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINJ &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTJ &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTK
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTK) {				
			if(regtyp == AVRIO_DDRREG)
				DDRK &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINK &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTK &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
#ifdef PORTL
                } else if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTL) {				
			if(regtyp == AVRIO_DDRREG)
				DDRL &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PINREG)
				PINL &= ~AVRIO_PIN2AVRBITMASK(avrpin);
			else if ( regtyp == AVRIO_PORTREG)
				PORTL &= ~AVRIO_PIN2AVRBITMASK(avrpin);
#endif
                } else {                                                
                }                                                       
        }                                                               
}


/*
 * Internal Register write function.
 * 	This is not  a defined primitive
 */
static inline void avrio_WriteReg(uint8_t regtyp, uint8_t avrport, uint8_t data) __attribute__((always_inline));

void
avrio_WriteReg(uint8_t regtyp, uint8_t avrport, uint8_t data)					
{									
		if (avrport == AVRIO_PORTA) {				
#ifdef PORTA
			if(regtyp == AVRIO_DDRREG)
				DDRA = data;
			else if (regtyp == AVRIO_PINREG)
				PINA = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTA = data;
#endif
#ifdef PORTB
                } else if (avrport == AVRIO_PORTB) {				
			if(regtyp == AVRIO_DDRREG)
				DDRB = data;
			else if (regtyp == AVRIO_PINREG)
				PINB = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTB = data;
#endif
#ifdef PORTC
                } else if (avrport == AVRIO_PORTC) {				
			if(regtyp == AVRIO_DDRREG)
				DDRC = data;
			else if (regtyp == AVRIO_PINREG)
				PINC = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTC = data;
#endif
#ifdef PORTD
                } else if (avrport == AVRIO_PORTD) {				
			if(regtyp == AVRIO_DDRREG)
				DDRD = data;
			else if (regtyp == AVRIO_PINREG)
				PIND = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTD = data;
#endif
#ifdef PORTE
                } else if (avrport == AVRIO_PORTE) {				
			if(regtyp == AVRIO_DDRREG)
				DDRE = data;
			else if (regtyp == AVRIO_PINREG)
				PINE = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTE = data;
#endif
#ifdef PORTF
                } else if (avrport == AVRIO_PORTF) {				
			if(regtyp == AVRIO_DDRREG)
				DDRF = data;
			else if (regtyp == AVRIO_PINREG)
				PINF = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTF = data;
#endif
#ifdef PORTG
                } else if (avrport == AVRIO_PORTG) {				
			if(regtyp == AVRIO_DDRREG)
				DDRG = data;
			else if (regtyp == AVRIO_PINREG)
				PING = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTG = data;
#endif
#ifdef PORTH
                } else if (avrport == AVRIO_PORTH) {				
			if(regtyp == AVRIO_DDRREG)
				DDRH = data;
			else if (regtyp == AVRIO_PINREG)
				PINH = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTH = data;
#endif
#ifdef PORTI
                } else if (avrport == AVRIO_PORTI) {				
			if(regtyp == AVRIO_DDRREG)
				DDRI = data;
			else if (regtyp == AVRIO_PINREG)
				PINI = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTI = data;
#endif
#ifdef PORTJ
                } else if (avrport == AVRIO_PORTJ) {				
			if(regtyp == AVRIO_DDRREG)
				DDRJ = data;
			else if (regtyp == AVRIO_PINREG)
				PINJ = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTJ = data;
#endif
#ifdef PORTK
                } else if (avrport == AVRIO_PORTK) {				
			if(regtyp == AVRIO_DDRREG)
				DDRK = data;
			else if (regtyp == AVRIO_PINREG)
				PINK = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTK = data;
#endif
#ifdef PORTL
                } else if (avrport == AVRIO_PORTL) {				
			if(regtyp == AVRIO_DDRREG)
				DDRL = data;
			else if (regtyp == AVRIO_PINREG)
				PINL = data;
			else if (regtyp == AVRIO_PORTREG)
				 PORTL = data;
#endif
                } else {                                                
                }                                                       
}

/*
 * Internal function not really meant for external use
 */

static inline uint8_t avrio_ReadReg(uint8_t regtyp, uint8_t avrport) __attribute__((always_inline));

uint8_t
avrio_ReadReg(uint8_t regtyp, uint8_t avrport)					
{									
#ifdef PORTA
	if (avrport == AVRIO_PORTA)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRA);
		else if ( regtyp == AVRIO_PINREG)
			return(PINA);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTA);
	}
#endif
#ifdef PORTB
	if (avrport == AVRIO_PORTB)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRB);
		else if ( regtyp == AVRIO_PINREG)
			return(PINB);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTB);
	}
#endif
#ifdef PORTC
        if (avrport == AVRIO_PORTC)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRB);
		else if ( regtyp == AVRIO_PINREG)
			return(PINB);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTB);
	}
#endif
#ifdef PORTD
	if (avrport == AVRIO_PORTD)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRD);
		else if ( regtyp == AVRIO_PINREG)
			return(PIND);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTD);
	}
#endif
#ifdef PORTE
	if (avrport == AVRIO_PORTE)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRE);
		else if ( regtyp == AVRIO_PINREG)
			return(PINE);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTE);
	}
#endif
#ifdef PORTF
	if (avrport == AVRIO_PORTF)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRF);
		else if ( regtyp == AVRIO_PINREG)
			return(PINF);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTF);
	}
#endif
#ifdef PORTG
	if (avrport == AVRIO_PORTG)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRG);
		else if ( regtyp == AVRIO_PINREG)
			return(PING);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTG);
	}
#endif
#ifdef PORTH
	if (avrport == AVRIO_PORTH)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRH);
		else if ( regtyp == AVRIO_PINREG)
			return(PINH);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTH);
	}
#endif
#ifdef PORTI
	if (avrport == AVRIO_PORTI)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRI);
		else if ( regtyp == AVRIO_PINREG)
			return(PINI);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTI);
	}
#endif
#ifdef PORTJ
	if (avrport == AVRIO_PORTJ)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRJ);
		else if ( regtyp == AVRIO_PINREG)
			return(PINJ);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTJ);
	}
#endif
#ifdef PORTK
	if (avrport == AVRIO_PORTK)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRK);
		else if ( regtyp == AVRIO_PINREG)
			return(PINK);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTK);
	}
#endif
#ifdef PORTL
	if (avrport == AVRIO_PORTL)
	{				
		if(regtyp == AVRIO_DDRREG)
			return(DDRL);
		else if ( regtyp == AVRIO_PINREG)
			return(PINL);
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTL);
	}
#endif
	return(0);	/* not sure how to handle bad port values */
}

/************************************************************************
 * PRIMITIAVE: avrio_ReadBit - Read a bit/pin value
 *
 * SYNOPSIS:
 * 	uint8_t bitval = avrio_ReadBit(uint8_t regtyp, avrpin_t avrpin);
 *
 * INPUT PARAMETERS:
 *	uint8_t regtyp;
 *		specifies the register type, either DDR, PORT or PIN.
 *		Must be one of: AVRIO_DDRREG, AVRIO_PORTREG, AVRIO_PINREG
 *
 *	avrpin_t avrpin;
 *		This value is an "avrpin" value *not* a simple pin value.
 *		(see notes below for details)
 *		
 *
 * OUTPUT PARAMETERS:
 *		(none)
 *
 * DESCRIPTION:
 *	This function tests to see if a bit in a i/o register or a pin is set.
 *	The desired register type (DDR, PORT, or PIN) can be specified.
 *
 *	The return value is 0 if the bit is not set. If the bit is set,
 *	the return value will be a non zero value. 
 *
 *	The avrpin paramter will automaticaly adapt to different environments and
 *	suports multiple different style defintions.
 *
 *	In non arduino enviroments, this value should be specified using the
 *	pin defintions below. Direct pin values can be specified using a
 *	PIN_Pb definition. This means to specify port D bit 3 would be PIN_D3
 *
 *	Arduino users may use either arduino pin#s, or the PIN_Pb style naming.
 *
 *
 * RETURN VALUE:
 *		0		bit not set
 *	non-0		bit is set (Note that this is not 1 it is "non 0")
 *
 * NOTES/WARNINGS:
 *
 *	This function can only be used with constants. If any argument is not a constant,
 *	terrible code that is very large will result.
 *
 *	If arduno pin#s are used for avrpin and if the header that defines the macro:
 *	digitalPinToPortReg() is not included, incorrect/invalid code will silently be generated.
 *	
 *	
 *
 * SEE ALSO:
 *	avrio_Read8Bits
 *
 ************************************************************************/

static inline uint8_t avrio_ReadBit(uint8_t regtyp, avrpin_t avrpin) __attribute__((always_inline));

uint8_t
avrio_ReadBit(uint8_t regtyp, avrpin_t avrpin)					
{									
#ifdef PORTA
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTA)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRA & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINA & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTA & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTB
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTB)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRB & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINB & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTB & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTC
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTC)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRC & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINC & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTC & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTD
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTD)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRD & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PIND & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTD & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTE
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTE)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRE & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINE & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTE & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTF
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTF)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRF & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINF & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTF & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTG
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTG)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRG & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PING & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTG & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTH
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTH)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRH & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINH & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTH & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTI
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTI)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRI & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINI & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTI & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTJ
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTJ)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRJ & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINJ & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTJ & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTK
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTK)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRK & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINK & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTK & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
#ifdef PORTL
	if (AVRIO_PIN2AVRPORT(avrpin) == AVRIO_PORTL)
	{
		if(regtyp == AVRIO_DDRREG)
			return(DDRL & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PINREG)
			return(PINL & AVRIO_PIN2AVRBITMASK(avrpin));
		else if ( regtyp == AVRIO_PORTREG)
			return(PORTL & AVRIO_PIN2AVRBITMASK(avrpin));
	}
#endif
	return(0);	/* not sure how to handle bad port values */
}


/*
 *
 * The routines below are for moving multiple bits into AVR pins.
 * This is a somewhat complex task as to keep things easy for configuration,
 * there are no limits placed on pins or ports.
 * So the code must figure out which pins are together on the same port to
 * optimize the bit twiddling. If enough bits are together it will use
 * full bytes or nibbles.
 *
 *	NOTE:WARNING: This function is currently only desiged for constant
 *		      arguments. Do not use it with variables.
 *
 * The following are the 6 possible
 * combinations of 8 bit i/o configurations:
 *
 *	DATA pins		AVR bits		Access method
 *	1) p0-p7		d0-d7 on same port	8bit byte
 *
 *	2) p0-p3		on d0-d3 of port	4 bit nibble
 *	   p4-p7		scattered bits		individual i/o
 *
 *	3) p0-p3		on d4-d7 of port	4 bit nibble
 *	   p4-p7		scattered bits		individual i/o
 *
 *	4) p0-p3		scattered bits		individual i/o
 *	   p4-p7		on d0-d3 of port	4 bit nibble
 *
 *	5) p0-p3		scattered bits		individual i/o
 *	   p4-p7		on d4-d7 of port	4 bit nibble
 *
 *	6) p0-p3		on d4-d7 of port	4 bit nibble
 *	   p4-p7		on d0-d3 of port	4 bit nibble
 *
 *		NOTE: in this nibble swap case the code does not check to
 *			see if the ports are the same.
 *
 *
 *	7) p0-p3		scattered bits		individual i/o
 *	   p4-p7		scattered bits		individual i/o
 *
 *
 *	Code will automatically detect when data pins are adjacent, ordered and on
 *	the same ports or within nibbles. It will automatically use the best
 *	possible method to set the pins.
 *	To get 8 bit mode, all 8 pins much match up with all 8 AVR bits
 *	on a single port. p0->d0, p1->d1, etc...
 *
 *	To get a nibble mode, 4 pins of a nibble must align with 4 bits
 *	on a AVR port. Nibbles don't have to match. i.e. code can swap nibbles.
 *	i.e. p0-p3 matched with d0-d3 or p0-p3 matched with d4-d7
 *	likewise on the upper bits: p4-p7 matched with d0-d3 or p4-p7 matched with d4-d7.
 *
 *	If nothing is aligned, the code will drop down to individual bit i/o.
 *
 *	The port & nibble detection works because of the sparse avrpin numbering scheme.
 *	The lower 4 bits of an avrpin # is the avr port bit #.
 *	For example, A0-A7 is valid but A8-AF is not used.
 *
 *	By knowing this, it is possible to see if avrpins are on the same port.
 *	So avrpins that are numerically adjacent are gurarnteed to be on the same AVR port.
 *
 */


/*
 * Defines to simplify conditionals for checking for the above 6 configuration combinations.
 */

/*
 * Figures out if all 8 pins are contiguous and on same port for 8 bit mode
 */

#define AVRDATA_8BIT(p0, p1, p2, p3, p4, p5, p6, p7) 	\
	(AVRIO_PIN2AVRPIN(p0) + 1 == AVRIO_PIN2AVRPIN(p1)		\
	   && AVRIO_PIN2AVRPIN(p1) + 1 == AVRIO_PIN2AVRPIN(p2)	\
	   && AVRIO_PIN2AVRPIN(p2) + 1 == AVRIO_PIN2AVRPIN(p3)	\
	   && AVRIO_PIN2AVRPIN(p3) + 1 == AVRIO_PIN2AVRPIN(p4)	\
	   && AVRIO_PIN2AVRPIN(p4) + 1 == AVRIO_PIN2AVRPIN(p5)	\
	   && AVRIO_PIN2AVRPIN(p5) + 1 == AVRIO_PIN2AVRPIN(p6)	\
	   && AVRIO_PIN2AVRPIN(p6) + 1 == AVRIO_PIN2AVRPIN(p7)	\
	)

/*
 * figures out if 4 avrpins map to bits D0-D3 on an AVR port (lo nibble)
 */
#define AVRDATA_4BITLO(p0, p1, p2, p3)				\
	(AVRIO_PIN2AVRPIN(p0) + 1 == AVRIO_PIN2AVRPIN(p1)			\
	  && AVRIO_PIN2AVRPIN(p1) + 1 == AVRIO_PIN2AVRPIN(p2)		\
	  && AVRIO_PIN2AVRPIN(p2) + 1 == AVRIO_PIN2AVRPIN(p3)		\
	  && AVRIO_PIN2AVRBIT(p0) == 0				\
	)

/*
 * figures out if 4 avrpins map to bits D4-D7 on an AVR port
 */
#define AVRDATA_4BITHI(p0, p1, p2, p3) 				\
	(AVRIO_PIN2AVRPIN(p0) + 1 == AVRIO_PIN2AVRPIN(p1)			\
	  && AVRIO_PIN2AVRPIN(p1) + 1 == AVRIO_PIN2AVRPIN(p2)		\
	  && AVRIO_PIN2AVRPIN(p2) + 1 == AVRIO_PIN2AVRPIN(p3)		\
	  && AVRIO_PIN2AVRBIT(p3) == 7				\
	)



/************************************************************************
 * PRIMITIAVE: avrio_Write8Bits - write 8 bits to i/o register(s)
 *
 * SYNOPSIS:
 * 	void avrio_Write8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
 *				avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7, uint8_t data)
 *
 * INPUT PARAMETERS:
 *	uint8_t regtyp;
 *		specifies the register type, either DDR, PORT or PIN.
 *		Must be one of: AVRIO_DDRREG, AVRIO_PORTREG, AVRIO_PINREG
 *
 *	avrpin_t p0;
 *	avrpin_t p1;
 *	avrpin_t p2;
 *	avrpin_t p3;
 *	avrpin_t p4;
 *	avrpin_t p5;
 *	avrpin_t p6;
 *	avrpin_t p7;
 *		specifies the 8 bits/pins to of where to write the 8 bit data.
 *		(see notes below for details)
 *
 *	uint8_t data;
 *		The 8 bit value to be written.
 *		
 *
 * OUTPUT PARAMETERS:
 *		(none)
 *
 * DESCRIPTION:
 *	This function sets 8 i/o register bit/pin values from a single 8 bit byte. The desired register
 *	type (DDR, PORT, or PIN) can be specified.
 *
 *	Since each "pin" parameter is an avrpin, each pin can independently specify which AVR port to use.
 *	
 * 	The code will automatically figure out the best possible method of writing to the
 *	register bits. If bits/pins are adjacent on the same port and in the proper sequence,
 *	the code will use 4 bit nibbles or even a single 8 bit byte write.
 *	If nothing is aligned, then individual SBI/CBI instructions will be used.
 *
 *	The avrpin paramters will automaticaly adapt to different environments and
 *	suports multiple different style defintions.
 *
 *	In non arduino enviroments, this value should be specified using the
 *	pin defintions below. Direct pin values can be specified using a
 *	PIN_Pb definition. This means to speccify port D bit 3 would be PIN_D3
 *
 *	Arduino users may use either arduino pin#s, or the PIN_Pb style naming.
 *	
 *
 *
 * RETURN VALUE:
 *		(none)
 *
 * NOTES/WARNINGS:
 *	This function can only be used with constants. If any argument is not a constant,
 *	terrible code that is very large will result.
 *
 *	If arduno pin#s are used for avrpin and if the header that defines the macro:
 *	digitalPinToPortReg() is not included, incorrect/invalid code will silently be generated.
 *	
 *	
 *
 * SEE ALSO:
 *	avrio_WriteReg
 *	avrio_Write8Bits
 *
 ************************************************************************/

static inline void avrio_Write8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
	avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7, uint8_t data) __attribute__((always_inline));


void
avrio_Write8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
	avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7, uint8_t data)
{

	if(AVRDATA_8BIT(p0, p1, p2, p3, p4, p5, p6, p7))
	{
		/*
		 * data0-data7 on same port; full 8 bit mode
		 */
		avrio_WriteReg(regtyp, AVRIO_PIN2AVRPORT(p0), data);
	}
	else
	{
#ifndef AVRIO_NO4BIT
		if(AVRDATA_4BITLO(p0, p1, p2, p3))
		{

			/*
			 * data0-data3 on same LO nibble
			 */

			avrio_WriteReg(regtyp, AVRIO_PIN2AVRPORT(p0),
			  (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p0)) & 0xf0) | (data & 0x0f));

		}
		else if(AVRDATA_4BITHI(p0, p1, p2, p3))
		{
			/*
			 * data0-data3 on same HI nibble
			 */

			avrio_WriteReg(regtyp, AVRIO_PIN2AVRPORT(p0),
			  (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p0)) & 0x0f) | (uint8_t)(data << 4));

		}
		else
#endif
		{
			/*
			 * data0-data3 require BIT i/o
			 */

			/*
			 * While this sequence looks odd, it is actaully fewer cycles
			 * to slam all the bits to 0, then selectively turn on the needed
			 * bits, rather than use a if/else to set & clear each bit.
			 */

			avrio_WriteBit(regtyp, p0, 0);
			avrio_WriteBit(regtyp, p1, 0);
			avrio_WriteBit(regtyp, p2, 0);
			avrio_WriteBit(regtyp, p3, 0);

			if(data & _BV(0)) avrio_WriteBit(regtyp, p0, 1);
			if(data & _BV(1)) avrio_WriteBit(regtyp, p1, 1);
			if(data & _BV(2)) avrio_WriteBit(regtyp, p2, 1);
			if(data & _BV(3)) avrio_WriteBit(regtyp, p3, 1);

		}

#ifndef AVRIO_NO4BIT

		if(AVRDATA_4BITLO(p4, p5, p6, p7))
		{
			/*
			 * data4-data7 on same LOW nibble
			 */

			avrio_WriteReg(regtyp, AVRIO_PIN2AVRPORT(p4),
			  (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p4)) & 0xf0) | (data >> 4));
		}
		else if(AVRDATA_4BITHI(p4, p5, p6, p7))
		{
			/*
			 * data4-data7 on same HIGH nibble
			 */

			avrio_WriteReg(regtyp, AVRIO_PIN2AVRPORT(p4),
			  (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p4)) & 0x0f) | (data & 0xf0));
		}
		else
#endif
		{
			/*
			 * data4-data7 require BIT i/o
			 */

			/*
			 * While this sequence looks odd, it is actaully fewer cycles
			 * to slam all the bits to 0, then selectively turn on the needed
			 * bits, rather than use a if/else to set & clear each bit.
			 */

			avrio_WriteBit(regtyp, p4, 0);
			avrio_WriteBit(regtyp, p5, 0);
			avrio_WriteBit(regtyp, p6, 0);
			avrio_WriteBit(regtyp, p7, 0);

			if(data & _BV(4)) avrio_WriteBit(regtyp, p4, 1);
			if(data & _BV(5)) avrio_WriteBit(regtyp, p5, 1);
			if(data & _BV(6)) avrio_WriteBit(regtyp, p6, 1);
			if(data & _BV(7)) avrio_WriteBit(regtyp, p7, 1);
		}
	}
}

/************************************************************************
 * PRIMITIAVE: avrio_Read8Bits - read 8 bits from i/o register(s)
 *
 * SYNOPSIS:
 * 	uint8_t byteval = avrio_Write8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
 *				avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7, uint8_t data)
 *
 * INPUT PARAMETERS:
 *	uint8_t regtyp;
 *		specifies the register type, either DDR, PORT or PIN.
 *		Must be one of: AVRIO_DDRREG, AVRIO_PORTREG, AVRIO_PINREG
 *
 *	avrpin_t p0;
 *	avrpin_t p1;
 *	avrpin_t p2;
 *	avrpin_t p3;
 *	avrpin_t p4;
 *	avrpin_t p5;
 *	avrpin_t p6;
 *	avrpin_t p7;
 *		specifies the 8 bits/pins to of where to write the 8 bit data.
 *		(see notes below for details)
 *
 *
 * OUTPUT PARAMETERS:
 *		(none)
 *
 * DESCRIPTION:
 *	This function reads 8 i/o register bit/pin values and composes them into a single 8 bit byte.
 *	The desired register type (DDR, PORT, or PIN) can be specified.
 *
 *	Since each "pin" parameter is an avrpin, each pin can independently specify which AVR port to use.
 *	
 * 	The code will automatically figure out the best possible method of reading the
 *	register bits. If bits/pins are adjacent on the same port and in the proper sequence,
 *	the code will use 4 bit nibbles or even a single 8 bit byte read.
 *	If nothing is aligned, then individual bit test instructions will be used.
 *
 *	The avrpin paramters will automaticaly adapt to different environments and
 *	suports multiple different style defintions.
 *
 *	In non arduino enviroments, this value should be specified using the
 *	pin defintions below. Direct pin values can be specified using a
 *	PIN_Pb definition. This means to speccify port D bit 3 would be PIN_D3
 *
 *	Arduino users may use either arduino pin#s, or the PIN_Pb style naming.
 *	
 *
 *
 * RETURN VALUE:
 *		8 bit data value
 *
 * NOTES/WARNINGS:
 *	This function can only be used with constants. If any argument is not a constant,
 *	terrible code that is very large will result.
 *
 *	If arduno pin#s are used for avrpin and if the header that defines the macro:
 *	digitalPinToPortReg() is not included, incorrect/invalid code will silently be generated.
 *	
 *	
 *
 * SEE ALSO:
 *	avrio_ReadBit
 *
 ************************************************************************/
static inline uint8_t 
avrio_Read8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
	avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7) __attribute__((always_inline));

uint8_t
avrio_Read8Bits(uint8_t regtyp, avrpin_t p0, avrpin_t p1, avrpin_t p2, avrpin_t p3,
	avrpin_t p4, avrpin_t p5, avrpin_t p6, avrpin_t p7)
{
uint8_t data;

	if(AVRDATA_8BIT(p0, p1, p2, p3, p4, p5, p6, p7))
	{
		/*
		 * Full 8 bit mode
		 */

		data = avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p0));
	}
	else
	{
		data = 0;
#ifndef AVRIO_NO4BIT
		if(AVRDATA_4BITLO(p0, p1, p2, p3))
		{
			/*
		 	 * data0-data3 on same LOW nibble
		 	 */

			data |= (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p0)) & 0xf);

		}
		else if(AVRDATA_4BITHI(p0, p1, p2, p3))
		{
			/*
		 	 * data0-data3 on same HIGH nibble
		 	 */

			data |= ((avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p0))) >> 4);

		}
		else
#endif
		{
			/*
		 	 * data0-data3 require BIT i/o
		 	 */

			if(avrio_ReadBit(regtyp, p0)) data |= _BV(0);
			if(avrio_ReadBit(regtyp, p1)) data |= _BV(1);
			if(avrio_ReadBit(regtyp, p2)) data |= _BV(2);
			if(avrio_ReadBit(regtyp, p3)) data |= _BV(3);

		}

#ifndef AVRIO_NO4BIT
		if(AVRDATA_4BITLO(p4, p5, p6, p7))
		{
			/*
		 	 * data4-data7 on same LOW nibble
		 	 */

			data |= ((avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p4)) & 0xf) << 4);

		}
		else if(AVRDATA_4BITHI(p4, p5, p6, p7))
		{
			/*
		 	 * data4-data7 on same HIGH nibble
		 	 */

			data |= (avrio_ReadReg(regtyp, AVRIO_PIN2AVRPORT(p4)) & 0xf0);
		}
		else
#endif
		{
			/*
		 	 * data4-data7 require BIT i/o
		 	 */

			if(avrio_ReadBit(regtyp, p4)) data |= _BV(4);
			if(avrio_ReadBit(regtyp, p5)) data |= _BV(5);
			if(avrio_ReadBit(regtyp, p6)) data |= _BV(6);
			if(avrio_ReadBit(regtyp, p7)) data |= _BV(7);

		}
	}
	return(data);
}

/*
 * Turn off all the Arduino attachment to a pin
 * NOTE: currently this is VERY expensive in terms of code size for non arduino pins
 * because it is a searching loop function using macros not intended for
 * variables that is inlined. This is very NON-optimimum.
 *
 * If all pins are non arduino this will result in as much as 8k of code!!!
 *
 *	The easiest way to solve this is with a teensy style core_pins.h
 *	using the defines in there, the loop goes away and is replaced
 *	with a macro.
 */

#define avrio_avriomode(avrpin) \
do										\
{										\
										\
	/*									\
	 * If arduino pin, then use it directly					\
	 */									\
	if(avrpin < AVRIO_PIN(AVRIO_PORTA, 0))					\
	{									\
			pinMode(avrpin, INPUT);					\
			digitalWrite(avrpin, 0);				\
	}									\
	else									\
	{									\
		/*								\
		 * Search for arduino pin # that matches this avrpin #		\
		 */								\
										\
		for(int p = 0; p < (AVRIO_PIN(AVRIO_PORTA, 0)); p++)		\
		{								\
			if(AVRIO_PIN2AVRPIN(p) == AVRIO_PIN2AVRPIN(avrpin))	\
			{							\
				pinMode(p, INPUT);				\
				digitalWrite(p, 0);				\
				break;						\
			}							\
		}								\
	}									\
} while(0)

/*
 * Pin defines for all the ports/bits.
 *
 * These defines are for specifying a bit withing a port.
 * They fully define an "avrpin". And as discussed above they
 * do not specify a type of register (PORT, PIN, DDR) they merely
 * specify the bit within the "port". See the avrio primitives above
 * to see how to specify the type of register (PORT, PIN, DDR)
 *
 *	These defines allow specifyng a specific bit on a specific port.
 *	The general format is PIN_Pb
 *	Where P is a port A-Z
 *	and b is a bit 0-7
 *
 *	So to specify bit 3 on port D use PIN_D3
 */

/*
 * Note: only define these if the teensy core_pins.h file has not
 * already defined them.
 */

#ifndef CORE_NUM_TOTAL_PINS

#define PIN_A0	AVRIO_PIN(AVRIO_PORTA, 0)
#define PIN_A1	AVRIO_PIN(AVRIO_PORTA, 1)
#define PIN_A2	AVRIO_PIN(AVRIO_PORTA, 2)
#define PIN_A3	AVRIO_PIN(AVRIO_PORTA, 3)
#define PIN_A4	AVRIO_PIN(AVRIO_PORTA, 4)
#define PIN_A5	AVRIO_PIN(AVRIO_PORTA, 5)
#define PIN_A6	AVRIO_PIN(AVRIO_PORTA, 6)
#define PIN_A7	AVRIO_PIN(AVRIO_PORTA, 7)

#define PIN_B0	AVRIO_PIN(AVRIO_PORTB, 0)
#define PIN_B1	AVRIO_PIN(AVRIO_PORTB, 1)
#define PIN_B2	AVRIO_PIN(AVRIO_PORTB, 2)
#define PIN_B3	AVRIO_PIN(AVRIO_PORTB, 3)
#define PIN_B4	AVRIO_PIN(AVRIO_PORTB, 4)
#define PIN_B5	AVRIO_PIN(AVRIO_PORTB, 5)
#define PIN_B6	AVRIO_PIN(AVRIO_PORTB, 6)
#define PIN_B7	AVRIO_PIN(AVRIO_PORTB, 7)

#define PIN_C0	AVRIO_PIN(AVRIO_PORTC, 0)
#define PIN_C1	AVRIO_PIN(AVRIO_PORTC, 1)
#define PIN_C2	AVRIO_PIN(AVRIO_PORTC, 2)
#define PIN_C3	AVRIO_PIN(AVRIO_PORTC, 3)
#define PIN_C4	AVRIO_PIN(AVRIO_PORTC, 4)
#define PIN_C5	AVRIO_PIN(AVRIO_PORTC, 5)
#define PIN_C6	AVRIO_PIN(AVRIO_PORTC, 6)
#define PIN_C7	AVRIO_PIN(AVRIO_PORTC, 7)

#define PIN_D0	AVRIO_PIN(AVRIO_PORTD, 0)
#define PIN_D1	AVRIO_PIN(AVRIO_PORTD, 1)
#define PIN_D2	AVRIO_PIN(AVRIO_PORTD, 2)
#define PIN_D3	AVRIO_PIN(AVRIO_PORTD, 3)
#define PIN_D4	AVRIO_PIN(AVRIO_PORTD, 4)
#define PIN_D5	AVRIO_PIN(AVRIO_PORTD, 5)
#define PIN_D6	AVRIO_PIN(AVRIO_PORTD, 6)
#define PIN_D7	AVRIO_PIN(AVRIO_PORTD, 7)

#define PIN_E0	AVRIO_PIN(AVRIO_PORTE, 0)
#define PIN_E1	AVRIO_PIN(AVRIO_PORTE, 1)
#define PIN_E2	AVRIO_PIN(AVRIO_PORTE, 2)
#define PIN_E3	AVRIO_PIN(AVRIO_PORTE, 3)
#define PIN_E4	AVRIO_PIN(AVRIO_PORTE, 4)
#define PIN_E5	AVRIO_PIN(AVRIO_PORTE, 5)
#define PIN_E6	AVRIO_PIN(AVRIO_PORTE, 6)
#define PIN_E7	AVRIO_PIN(AVRIO_PORTE, 7)

#define PIN_F0	AVRIO_PIN(AVRIO_PORTF, 0)
#define PIN_F1	AVRIO_PIN(AVRIO_PORTF, 1)
#define PIN_F2	AVRIO_PIN(AVRIO_PORTF, 2)
#define PIN_F3	AVRIO_PIN(AVRIO_PORTF, 3)
#define PIN_F4	AVRIO_PIN(AVRIO_PORTF, 4)
#define PIN_F5	AVRIO_PIN(AVRIO_PORTF, 5)
#define PIN_F6	AVRIO_PIN(AVRIO_PORTF, 6)
#define PIN_F7	AVRIO_PIN(AVRIO_PORTF, 7)

#define PIN_G0	AVRIO_PIN(AVRIO_PORTG, 0)
#define PIN_G1	AVRIO_PIN(AVRIO_PORTG, 1)
#define PIN_G2	AVRIO_PIN(AVRIO_PORTG, 2)
#define PIN_G3	AVRIO_PIN(AVRIO_PORTG, 3)
#define PIN_G4	AVRIO_PIN(AVRIO_PORTG, 4)
#define PIN_G5	AVRIO_PIN(AVRIO_PORTG, 5)
#define PIN_G6	AVRIO_PIN(AVRIO_PORTG, 6)
#define PIN_G7	AVRIO_PIN(AVRIO_PORTG, 7)

#define PIN_H0	AVRIO_PIN(AVRIO_PORTH, 0)
#define PIN_H1	AVRIO_PIN(AVRIO_PORTH, 1)
#define PIN_H2	AVRIO_PIN(AVRIO_PORTH, 2)
#define PIN_H3	AVRIO_PIN(AVRIO_PORTH, 3)
#define PIN_H4	AVRIO_PIN(AVRIO_PORTH, 4)
#define PIN_H5	AVRIO_PIN(AVRIO_PORTH, 5)
#define PIN_H6	AVRIO_PIN(AVRIO_PORTH, 6)
#define PIN_H7	AVRIO_PIN(AVRIO_PORTH, 7)

#define PIN_I0	AVRIO_PIN(AVRIO_PORTI, 0)
#define PIN_I1	AVRIO_PIN(AVRIO_PORTI, 1)
#define PIN_I2	AVRIO_PIN(AVRIO_PORTI, 2)
#define PIN_I3	AVRIO_PIN(AVRIO_PORTI, 3)
#define PIN_I4	AVRIO_PIN(AVRIO_PORTI, 4)
#define PIN_I5	AVRIO_PIN(AVRIO_PORTI, 5)
#define PIN_I6	AVRIO_PIN(AVRIO_PORTI, 6)
#define PIN_I7	AVRIO_PIN(AVRIO_PORTI, 7)

#define PIN_J0	AVRIO_PIN(AVRIO_PORTJ, 0)
#define PIN_J1	AVRIO_PIN(AVRIO_PORTJ, 1)
#define PIN_J2	AVRIO_PIN(AVRIO_PORTJ, 2)
#define PIN_J3	AVRIO_PIN(AVRIO_PORTJ, 3)
#define PIN_J4	AVRIO_PIN(AVRIO_PORTJ, 4)
#define PIN_J5	AVRIO_PIN(AVRIO_PORTJ, 5)
#define PIN_J6	AVRIO_PIN(AVRIO_PORTJ, 6)
#define PIN_J7	AVRIO_PIN(AVRIO_PORTJ, 7)

#define PIN_K0	AVRIO_PIN(AVRIO_PORTK, 0)
#define PIN_K1	AVRIO_PIN(AVRIO_PORTK, 1)
#define PIN_K2	AVRIO_PIN(AVRIO_PORTK, 2)
#define PIN_K3	AVRIO_PIN(AVRIO_PORTK, 3)
#define PIN_K4	AVRIO_PIN(AVRIO_PORTK, 4)
#define PIN_K5	AVRIO_PIN(AVRIO_PORTK, 5)
#define PIN_K6	AVRIO_PIN(AVRIO_PORTK, 6)
#define PIN_K7	AVRIO_PIN(AVRIO_PORTK, 7)

#define PIN_L0	AVRIO_PIN(AVRIO_PORTL, 0)
#define PIN_L1	AVRIO_PIN(AVRIO_PORTL, 1)
#define PIN_L2	AVRIO_PIN(AVRIO_PORTL, 2)
#define PIN_L3	AVRIO_PIN(AVRIO_PORTL, 3)
#define PIN_L4	AVRIO_PIN(AVRIO_PORTL, 4)
#define PIN_L5	AVRIO_PIN(AVRIO_PORTL, 5)
#define PIN_L6	AVRIO_PIN(AVRIO_PORTL, 6)
#define PIN_L7	AVRIO_PIN(AVRIO_PORTL, 7)

#endif // end of teensy core pin check

#endif

