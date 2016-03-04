F_CPU=1000000

all:
	avr-gcc -DF_CPU=$(F_CPU) -mmcu=atmega328p -Wall -Os -o out.elf *.c
	avr-objcopy -j .text -j .data -O ihex out.elf out.hex
	sudo avrdude  -p  m328p -c usbasp  -P usb -B 4 -U flash:w:out.hex
