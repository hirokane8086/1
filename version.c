
#define ONE_OF_ELEVEN_VER

#ifdef GIT_HASH
	#define VER     GIT_HASH
#else
	#define VER     "0.0.1-ahpla"
#endif

#ifndef ONE_OF_ELEVEN_VER
	const char Version[]      = "OEFW-"VER;
	const char UART_Version[] = "UV-K5 Firmware, Open Edition, OEFW-"VER"\r\n";
#else
	const char Version[]      = "BG8LGP-"VER;
	const char UART_Version[] = "UV-K5 Firmware, Open Edition, bg8lgp-"VER" from 1o11 and egzumer\r\n";
#endif
