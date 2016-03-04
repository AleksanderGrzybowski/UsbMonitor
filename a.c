#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <string.h>

#include "HD44780.h"

#define VOLTAGE_DIV_VCC 4700.0
#define VOLTAGE_DIV_GND 1000.0
#define SHUNT_R 0.2

#define ADC_REF 1.1
#define ADC_MAX_VALUE 1024.0

#define ADC_CHANNEL_VCC 0
#define ADC_CHANNEL_I 1

#define SAMPLE_COUNT 300
#define MILLIAMPS_IN_AMPS 1000

#define LCD_WIDTH 16
#define LCD_SAMPLING_REFRESH_DELAY 1000

#define MODE_JUMPER_PORT PORTD
#define MODE_JUMPER_PIN PIND
#define MODE_JUMPER_DIR DDRD
#define MODE_JUMPER PD6

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
	while (ADCSRA & (1<<ADSC));

	return ADC;
}

typedef struct meas {
	float volts;
	uint16_t milliamps;
} meas;

typedef enum mode {AVERAGING, SAMPLING} mode;

meas readings_to_meas(uint16_t adc_volts, uint16_t adc_milliamps) {
	meas m;
	m.volts = ((ADC_REF * adc_volts)/ADC_MAX_VALUE) * (VOLTAGE_DIV_VCC + VOLTAGE_DIV_GND) / (VOLTAGE_DIV_GND);
	m.milliamps = (uint16_t) (((ADC_REF * adc_milliamps)/ADC_MAX_VALUE) * SHUNT_R * MILLIAMPS_IN_AMPS);
	return m;
}

meas get_single() {
	return readings_to_meas(read_adc(ADC_CHANNEL_VCC), read_adc(ADC_CHANNEL_I));
}

meas get_averaged() {
	uint16_t i;
	uint32_t adc_volts_sum = 0, adc_milliamps_sum = 0;

	for (i = 0; i < SAMPLE_COUNT; ++i) {
		adc_volts_sum += read_adc(ADC_CHANNEL_VCC);
		adc_milliamps_sum += read_adc(ADC_CHANNEL_I);
	}

	return readings_to_meas(adc_volts_sum/SAMPLE_COUNT, adc_milliamps_sum/SAMPLE_COUNT);
}

char disp1[LCD_WIDTH + 1];
char disp2[LCD_WIDTH + 1];

void view(meas m, mode meas_mode) {
	LCD_Clear();
	memset(disp1, ' ', LCD_WIDTH);
	memset(disp2, ' ', LCD_WIDTH);
	disp1[LCD_WIDTH-1] = disp2[LCD_WIDTH-1] = 0;

	int nulpos;
	nulpos = sprintf(disp1, "U = %.2f V", m.volts);
	disp1[nulpos] = ' ';
	nulpos = sprintf(disp2, "I = %4u", m.milliamps);
	disp2[nulpos] = ' ';

	disp2[9] = 'm';
	disp2[10] = 'A';

	if (meas_mode == AVERAGING) {
		disp2[LCD_WIDTH - 2] = 'a';
		disp2[LCD_WIDTH - 1] = 'v';
	} else if (meas_mode == SAMPLING) {
		disp2[LCD_WIDTH - 2] = 's';
		disp2[LCD_WIDTH - 1] = 'm';
	}

	LCD_GoTo(0, 0);
	LCD_WriteText(disp1);
	LCD_GoTo(0, 1);
	LCD_WriteText(disp2);
}

void init() {
	init_adc();
	LCD_Initalize();

	MODE_JUMPER_DIR &= ~(1 << MODE_JUMPER);
	MODE_JUMPER_PORT |= (1 << MODE_JUMPER);
}

mode current_mode() {
	return (MODE_JUMPER_PIN & (1 << MODE_JUMPER)) ? AVERAGING : SAMPLING;
}


int main(void) {
	init();

	meas m;

	while(1) {
		if (current_mode() == AVERAGING) {
			m = get_averaged();
			view(m, AVERAGING);
		} else {
			m = get_single();
			view(m, SAMPLING);
			_delay_ms(LCD_SAMPLING_REFRESH_DELAY);
		}
	}
}
