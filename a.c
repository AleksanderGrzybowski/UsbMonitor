#define F_CPU (8000000L/8)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


int main(void) {

	DDRC = 0xff;

	while (1) {
		PORTC ^= 0xff;
		_delay_ms(1000);
	}
}
