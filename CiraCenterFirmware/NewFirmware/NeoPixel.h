/**
 * NeoPixel.h
 * 
 * @Author
 * @date
 * 
 * @note	port of Adafruit NeoPixel Cpp library to C on Atmel ATSAM3X8E
 * 			specifically
 */

#ifndef NEOPIXEL_H
#define NEOPIXEL_H
#include "AtmelArmGpio.h"

typedef struct neopixel_t
{
	const uint16_t numLeds;		//!<
	const uint16_t numBytes;	//!<

	gpio_t *	signalPad;		//!<

	uint8_t 	brightness;		//!<
	uint8_t 	color[3];		//!<


} neopixel_t;

#endif // NeoPixel.h