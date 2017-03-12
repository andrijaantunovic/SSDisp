/*
 * SSDisp.h
 *
 *  Created on: 1. ožu 2017.
 *      Author: Andrija
 */

#ifndef SSDISP_H_
#define SSDISP_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define SEGBYTE_BUFFER 64

#define DP_SEGBYTE 0b10000000
#define SSDMODE_NORMAL 0
#define SSDMODE_TICKER 1

#include <Time.h>

/*
A segbyte is an unsigned byte which contains information about which segments should be lit on a digit.
The Least Significant Bit corresponds to the top segment (A), the next bits by significance correspond to the
next segments clockwise (BCDEF), second-to-most-significant bit corresponds to the center segment (G) and the
most significant bit corresponds to the decimal point (H).

The bits are extracted from the segbyte by taking the modulo of 2, which gives us the LSB or the top segment,
after which the byte is bitshifted to the right (or divided by 2). Now that the next bit is in the LSB position,
modulo of 2 can be taken from it again to get the next bit.
The segbyte is destroyed in the process of extraction, so it first needs to be copied.
*/

typedef uint8_t segbyte;

const segbyte _segbyteArray[64] PROGMEM = {
	0b00000000, 0b00000001, 0b00100010, 0b00110110, 0b01110000, 0b01001001, 0b01111011, 0b00100000, //   ! " # $ % & '
	0b00110001, 0b00000111, 0b01100011, 0b01000110, 0b10000000, 0b01000000, 0b10000000, 0b01010010, // ( ) * + , - . /
	0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00100111, // 0 1 2 3 4 5 6 7
	0b01111111, 0b01101111, 0b00000100, 0b00010000, 0b01011000, 0b01001000, 0b01001100, 0b01010011, // 8 9 : ; < = > ?
	0b01011111, 0b01110111, 0b01111100, 0b00111001, 0b01011110, 0b01111001, 0b01110001, 0b00111101, // @ A B C D E F G
	0b01110110, 0b00110000, 0b00011110, 0b01110010, 0b00111000, 0b00110111, 0b01010100, 0b01011100, // H I J K L M N O
	0b01110011, 0b01100111, 0b01010000, 0b00101101, 0b01111000, 0b00111110, 0b00011100, 0b01111110, // P Q R S T U V W
	0b00100100, 0b01101110, 0b00011011, 0b00001001, 0b01100100, 0b00001111, 0b00100011, 0b00001000  // X Y Z [ \ ] ^ _
};

class SSD {
private:
	//SETUP AND CONTROL VARIABLES
	byte totalDisplayDigits;			//number of digits the display has
	byte digitPins[SEGBYTE_BUFFER];		//pins which are connected to digit pins, starting from Least Significant Digit
	byte segmentPins[8];				//pins which are connected to segment pins, in the order: A B C D E F G H

	//SEGBYTE CONTAINER
	segbyte segbytes[SEGBYTE_BUFFER];

	//TIMING VARIABLES
	byte currentDigit;						//digit which is being currently shown, 0 being the leftmost digit
	unsigned long durationBetweenDigits;	//in us
	unsigned long timeForNextDigit;			//in us
	unsigned long durationBetweenTickerAdv;	//in ms
	unsigned long timeForNextTickerAdv;		//in ms
	int8_t tickerPosition;
	int8_t tickerStringLength;

	//SETTINGS VARIABLES
	byte mode;
	bool leadingZeroes;
	byte overflowHandler;

	//INTERNAL FUNCTIONS
	byte numPlaces(unsigned long);

	//FUNCTIONS FOR EXTRACTING SEGBYTES FROM PROGMEM FOR A GIVEN NUMBER OR ASCII CHARACTER
	segbyte getSegbyte(byte);
	segbyte getSegbyteFromAscii(char);
	segbyte getSegbyteFromNumber(byte);

public:
	//CONSTRUCTOR
	SSD();
	void init(byte, const byte*, const byte*);

	//SETTINGS FUNCTIONS
	void setTickerSpeed(int);
	void setRefreshSpeed(int);
	void setLeadingZeroes(bool);
	void setDecimalPlaces(byte, byte, byte, bool);
	void setOverflowHandler(byte);

	//PRIMARY DISPLAY FUNCTIONS
	void showNumber(long, int8_t = -1);
	void showText(char*, byte = 2);

	//DISPLAY UPDATE FUNCTION
	void run();

	//CLEAR DISPLAY
	void allOff();
};

#endif /* SSDISP_H_ */
