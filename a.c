#define F_CPU (8000000L/8)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "HD44780.h"

int main(void) {

	LCD_Initalize();
	LCD_WriteText("Hello world");

	while(1);
}
