/*
  AutoSlide copier
	For DSLR + Slide Projector
	B. A. Bryce 2009
	No warranty expressed or implied
	Fuses are set for internal RC 8 MHz, and level triggered interupts

*/



#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>


//Constants:

#define clock_in_MHz 8



//Helper Macros:

// This writes the bit selected by bitmask of y into x
#define writebit(x, bitmask, y)  ( (x) = (((x) & (~bitmask)) | ((y) & (bitmask))));

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))




//Define functions
//======================
void initialsetup(void);      	// initializes IO
void delay_ms(uint16_t x); 		// general purpose delay

volatile uint8_t flag;
volatile uint8_t data;

uint8_t  EEMEM delayOne = 2;
uint8_t  EEMEM slideOnTime = 2;
uint8_t  EEMEM delayTwo = 2;
uint8_t  EEMEM cameraOnTime = 2;


ISR(INT0_vect)
{ /* signal handler for external interrupt int0 */

	//Start handling code:


	data = PINB;

	if ((data & 0b10000000) != 0)
	{
		if((data & 0b01100000) == 0b00000000)eeprom_write_byte (&slideOnTime, data & 0b00011111);
		else if((data & 0b01100000) == 0b00100000)eeprom_write_byte (&delayOne, data & 0b00011111);
		else if((data & 0b01100000) == 0b01000000)eeprom_write_byte (&cameraOnTime, data & 0b00011111);
		else if((data & 0b01100000) == 0b01100000)eeprom_write_byte (&delayTwo, data & 0b00011111);

		writebit(PORTD, 1<<6, 0x00);
		delay_ms(500);
		writebit(PORTD, 1<<6, 0xff);
		delay_ms(500);
		writebit(PORTD, 1<<6, 0x00);
		delay_ms(500);
		writebit(PORTD, 1<<6, 0xff);
	}
	else flag = 1;

	delay_ms(500); 				//debounce it (and next line)
	EIFR |= (1 << INTF0); 		//clear the extra int that might have occured while here P69 of manual (0b00000001)
}


int main (void)
{


    initialsetup(); //Setup IO pins and defaults

	uint16_t t1;
	uint16_t t2;
	uint16_t t3;
	uint16_t t4;
	uint8_t	slidenumber;


    while(1)
    {


		if (flag == 1)
		{
			t1 = 25*eeprom_read_byte(&slideOnTime);
			t2 = 125*eeprom_read_byte(&delayOne);
			t3 = 25*eeprom_read_byte(&cameraOnTime);
			t4 = 125*eeprom_read_byte(&delayTwo);
			slidenumber = PINB;
			flag = 0;
			writebit(PORTD, 1<<6, 0x00);


			for (uint8_t i=0; i < slidenumber; i++)
			{
				writebit(PORTD, 1<<5, 0xff);
				delay_ms(t1);
				writebit(PORTD, 1<<5, 0x00);
				delay_ms(t2);
				writebit(PORTD, 1<<4, 0xff);
				delay_ms(t3);
				writebit(PORTD, 1<<4, 0x00);
				delay_ms(t4);
			}
			writebit(PORTD, 1<<6, 0xff);

		}





    }

    return(0);
}




void initialsetup (void)
{




	//1 = output, 0 = input
    DDRB = 0b00000000; 	//All inputs
    DDRD = 0b11111011;


	GIMSK = 0b01000000; // Enable INT0
	MCUCR = 0b00000001; // Logic change trigger INT0


	flag = 0;
	writebit(PORTD, 1<<6, 0xff);


    sei();       				// enable interrupts

	delay_ms(1000);



}



//Approx Short Delay function
void delay_ms(uint16_t time_in_ms)
{
  uint16_t x;
  uint8_t y, z;

  x = time_in_ms*clock_in_MHz;

  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}
