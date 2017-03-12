/*
 * SSDisp.cpp
 *
 *  Created on: 1. ožu 2017.
 *      Author: Andrija
 */

#include "SSDisp.h"

SSD::SSD()
{
	durationBetweenDigits = 3000;
	durationBetweenTickerAdv = 300;

	totalDisplayDigits = 0;
	currentDigit = 0;
	timeForNextDigit = 0;
	timeForNextTickerAdv = 0;
	tickerPosition = 0;
	tickerStringLength = 0;
	mode = 0;
	leadingZeroes = false;
	overflowHandler = 1;
}

void SSD::init(byte d, const byte *dp, const byte *sp)
{
	if (d > SEGBYTE_BUFFER)
		return;

	for (int i = 0; i < d; i++)
	{
		digitPins[i] = dp[i];
		pinMode(digitPins[i], OUTPUT);
	}
	for (int i = 0; i < 8; i++)
	{
		segmentPins[i] = sp[i];
		pinMode(segmentPins[i], OUTPUT);
	}
	totalDisplayDigits = d;
}

segbyte SSD::getSegbyte(byte var) {
	return (segbyte)pgm_read_byte_near(&(_segbyteArray[var]));
}

segbyte SSD::getSegbyteFromAscii(char var) {
	if (var < ' ')
		return getSegbyte(0);
	if (var <= '_')
		return getSegbyte(var - ' ');
	if (var >= 'a' && var <= 'z')
		return getSegbyte(var - ' ' - 32);
	return getSegbyte('?' - ' ');
}

segbyte SSD::getSegbyteFromNumber(byte var) {
	return (segbyte)pgm_read_byte_near(&(_segbyteArray[var + 16]));
}

void SSD::run() //TODO: compensate for time overflow
{
	//check if ticker mode and it's time to push the ticker
	if (mode == SSDMODE_TICKER && millis() >= timeForNextTickerAdv)
	{
		//stores the next time, in milliseconds since power-on, when we should push the ticker by one character
		timeForNextTickerAdv = millis() + durationBetweenTickerAdv;
		tickerPosition++;

		if (tickerPosition > tickerStringLength)
			tickerPosition = -totalDisplayDigits;
	}

	//check if it's time to start displaying the next digit
	if (micros() >= timeForNextDigit)
	{
		//stores the next time, in microseconds since power-on, when we should move on to displaying the next digit
		timeForNextDigit = micros() + durationBetweenDigits;

		//turns off the current digit
		digitalWrite(digitPins[currentDigit], LOW);

		//moves to next digit
		currentDigit++; if (currentDigit >= totalDisplayDigits) currentDigit = 0;

		//copies the segbyte of the current digit into a new variable so that we can chop it for bits
		segbyte segbyte;

		if (mode == SSDMODE_TICKER)
		{
			if (tickerPosition + currentDigit < 0 || tickerStringLength - tickerPosition - currentDigit <= 0)
				segbyte = getSegbyteFromAscii(' ');
			else
				segbyte = segbytes[tickerPosition + currentDigit];
		}
		else
			segbyte = segbytes[currentDigit];

		//sets the appropriate segments ON or OFF
		for (byte s = 0; s < 8; s++)
		{
			if (segbyte % 2)							//should this segment be turned on?
				digitalWrite(segmentPins[s], LOW);		//segment is turned ON
			else
				digitalWrite(segmentPins[s], HIGH);		//segment is turned OFF

			//we shift the bits to the right so that we can access the next segbyte bit in the next iteration
			segbyte /= 2;
		}

		//now that all segments are set, turns on the digit
		digitalWrite(digitPins[currentDigit], HIGH);
	}
}

void SSD::showText(char *s, byte ticker = 2)
{
	int aCur = 0; //argument 'cursor'
	int sCur;     //segbyte array 'cursor'

	//ignore any trailing dots from the beginning of the string
	while (s[aCur] == '.')
		aCur++;

	//Go through the string and create the segbytes
	for (sCur = 0; s[aCur] != '\0'; sCur++, aCur++)
	{
		if (s[aCur] == '.')					//if the character is a dot (.)
		{
			sCur--;							//...undo the advancement of the screen 'cursor'
			segbytes[sCur] |= DP_SEGBYTE;	//...and add a dot segment to the previous character
		}
		else								//if any other character
		{
			segbytes[sCur] = getSegbyteFromAscii(s[aCur]);	//put it into the segbyte array
		}
	}

	tickerStringLength = sCur;

	if (tickerStringLength > totalDisplayDigits)	//string is longer than display?
	{
		mode = SSDMODE_TICKER;
		tickerPosition = -totalDisplayDigits;
	}
	else
	{
		mode = SSDMODE_NORMAL;
	}

	if (sCur < totalDisplayDigits)					//If the display is any larger than the provided string
	{
		segbytes[totalDisplayDigits] = '\0';	 	//...terminate the string at the position of the screen length
		for (; sCur < totalDisplayDigits; sCur++)	//...and fill the spaces inbetween with ' ', overwriting the '\0' provided in the parameter.
			segbytes[sCur] = getSegbyteFromAscii(' ');
	}
}

void SSD::showNumber(long n, int8_t decimal = -1)
{
	byte availableDigits = totalDisplayDigits;
	byte negative = n < 0 ? 1 : 0;

	if (negative)
	{
		segbytes[0] = getSegbyteFromAscii('-');
		n = abs(n);
		availableDigits--;
	}

	if (numPlaces(n) > availableDigits && overflowHandler) //handle overflow
	{
		if (overflowHandler == 1) //show the number in scientific notation
		{
			//display the letter E in second-to-last position on display
			segbytes[totalDisplayDigits - 2] = getSegbyteFromAscii('E');

			//calculate and display the exponent
			byte expon = 0;
			for (unsigned long a = n / 10; a > 0; a /= 10) expon++;
			segbytes[totalDisplayDigits - 1] = getSegbyteFromNumber(expon);

			//getting rid of all the least significant digits which cannot fit on the screen
			for (expon--; expon > 0; expon--) n /= 10;
			if (negative) n /= 10;

			//display digits normally in the remaining fields
			for (int8_t d = totalDisplayDigits - 3; d >= 0 + negative; d--)
			{
				segbytes[d] = getSegbyteFromNumber(n % 10);
				n /= 10;
			}

			segbytes[negative] |= DP_SEGBYTE;
		}
		else //basic overflow handler which only displays 'OVRF' as an error message
		{
			showText("0VRF");
		}
	}
	else //no overflow, number can be displayed normally
	{
		for (int8_t d = totalDisplayDigits - 1; d >= totalDisplayDigits - availableDigits; d--)
		{
			//Any of the three conditions means that this digit should be printed:
			// - if leadingZeroes is TRUE, all digits are always printed (the entire screen will be padded with zeroes)
			// - if n is still larger than 0 (ie. TRUE), we still have digits to print (the printing is done right-to-left)
			// - if this is the Least Significant Digit, we always want to print it (even if it's a zero)
			if (leadingZeroes || n || d == totalDisplayDigits - 1)
				segbytes[d] = getSegbyteFromNumber(n % 10);
			else
				segbytes[d] = getSegbyteFromAscii(' ');
			n /= 10;
		}
	}
}

// SETTINGS FUNCTIONS
void SSD::setTickerSpeed(int s) { durationBetweenTickerAdv = s; }
void SSD::setRefreshSpeed(int s) { durationBetweenDigits = s; }
void SSD::setLeadingZeroes(bool lz) { leadingZeroes = lz; }
void SSD::setOverflowHandler(byte oh) { overflowHandler = oh; }

// UTILITY FUNCTIONS
byte SSD::numPlaces(unsigned long n) {
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	return 10;
}

void SSD::allOff()
{
	for (int8_t d = totalDisplayDigits - 1; d >= 0; d--)
		digitalWrite(digitPins[d], LOW);
}
