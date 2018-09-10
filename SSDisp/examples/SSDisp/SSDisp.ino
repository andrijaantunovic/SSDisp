#include "Arduino.h"
#include "SSDisp.h"

SSD disp = SSD();

void setup() {
	//Declaring digit pins, segment pins, and creating a display object
	const byte dp[] = { 7, 6, 5, A5 };
	const byte sp[] = { 9, 4, 3, 11, 12, 8, 2, 10 };
	disp.init(4, dp, sp);

	Serial.begin(9600);
	disp.showText("DEM0 5TART");
	Serial.println(F("Send all commands with NL ending.\n"
			"Send a string to display it, or one of the commands to change the settings:\n"
			"/ts  ticker delay in ms (example: /ts 500)\n"
			"/rs  screen refresh delay in us (example: /rs 3000)\n"
			"/oh  overflow handler (0, 1 or 2) (example: /oh 1)\n"
			"/lz  leading zeroes (0 or 1) (example: /ls 0)\n"
			"/ followed by an integer to display an integer (example: /1337)\n"));
}

void loop() {
	while(Serial.available() > 0)
	{
		disp.allOff();
		static int pos = 0;
		static char s[64] = {0};
		s[pos] = Serial.read();

		if (s[pos] == '\n') //a complete command was sent over serial.
		{
			s[pos] = '\0';
			pos = 0;
			Serial.println(s);

			if (s[0] == '/') //command
			{
				if (s[1] == 't' && s[2] == 's' && s[3] == ' ')
					disp.setTickerSpeed(atoi(s+4));
				else if (s[1] == 'r' && s[2] == 's' && s[3] == ' ')
					disp.setRefreshSpeed(atoi(s+4));
				else if (s[1] == 'o' && s[2] == 'h' && s[3] == ' ')
					disp.setOverflowHandler(atoi(s+4));
				else if (s[1] == 'l' && s[2] == 'z' && s[3] == ' ')
					disp.setLeadingZeroes(atoi(s+4));
				else
					disp.showNumber(atoi(s+1));
			}
			else
				disp.showText(s);
		}
		else
			pos++;
	}

	disp.run();
}
