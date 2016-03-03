#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "HD44780.h"

void init_adc() {
	// 1.1V reference
	ADMUX |= (1<<REFS0) | (1<<REFS1);

	// prescaler 128 and enable ADC 
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADEN);    
}

uint16_t read_adc(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	// single conversion mode
	ADCSRA |= (1<<ADSC);

	// wait until ADC conversion is complete
	while(ADCSRA & (1<<ADSC));

	return ADC;
}

int main(void) {

	init_adc();
	LCD_Initalize();

	char disp[10];
	int i = 0;

	while(1) {
		uint16_t first = read_adc(0);	
		uint16_t second = read_adc(1);
		sprintf(disp, "%d: %u - %u", i++, first, second);
		LCD_WriteText(disp);
		_delay_ms(1000);
		LCD_Clear();
	}
}
