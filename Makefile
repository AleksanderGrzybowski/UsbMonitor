all:
	avr-gcc -mmcu=atmega328p -Wall -Os -o out.elf *.c
	avr-objcopy -j .text -j .data -O ihex out.elf out.hex
	sudo avrdude  -p  m328p -c usbasp  -P usb  -U flash:w:out.hex
