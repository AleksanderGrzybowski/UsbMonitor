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

typedef struct meas {
	float volts;
	uint16_t milliamps;
} meas;

meas get_single() {
	meas m;
	m.volts = ((1.1 * read_adc(0))/1024.0) * (1000.0 + 4700.0) / (1000.0);
	m.milliamps = (uint16_t) (((1.1 * read_adc(1))/1024.0) * 0.2 * 1000);
	return m;
}

meas get_averaged() {
	meas m;
	int i;
	uint32_t adc_volts, adc_milliamps;
	adc_volts = adc_milliamps = 0;
	for (i = 0; i < 300; ++i) {
		adc_volts += read_adc(0);
		adc_milliamps += read_adc(1);
	}
	m.volts = ((1.1 * adc_volts/300)/1024.0) * (1000.0 + 4700.0) / (1000.0);
	m.milliamps = (uint16_t) (((1.1 * (adc_milliamps/300))/1024.0) * 0.2 * 1000);
	return m;
}

int main(void) {

	init_adc();
	LCD_Initalize();

	DDRD &= ~(1 << PD6);
	PORTD |= (1 << PD6);

	char disp1[17];
	char disp2[17];

	meas m;

	while(1) {
		if (PIND & (1 << PD6)) { // averaging
			m = get_averaged();
			LCD_Clear();
			sprintf(disp1, "U = %.2f V", m.volts);
			sprintf(disp2, "I = %u", m.milliamps);
			LCD_GoTo(0, 0);
			LCD_WriteText(disp1);
			LCD_GoTo(0, 1);
			LCD_WriteText(disp2);
			LCD_GoTo(9, 1);
			LCD_WriteText("mA   av");
		} else { // sampling every second
			m = get_single();
			sprintf(disp1, "U = %.2f V", m.volts);
			sprintf(disp2, "I = %u", m.milliamps);
			LCD_GoTo(0, 0);
			LCD_WriteText(disp1);
			LCD_GoTo(0, 1);
			LCD_WriteText(disp2);
			LCD_GoTo(9, 1);
			LCD_WriteText("mA   sm");
			_delay_ms(1000);
			LCD_Clear();
		}
	}
}
