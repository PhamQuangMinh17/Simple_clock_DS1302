/*
 * Pham_Quang_Minh_simple_clock.c
 * Author : Pham Quang Minh - RMIT
 * Date: 06/07/2020
 */ 


#ifndef F_CPU
#define F_CPU 16000000U
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Define shortcut command
// CLK on B0
#define SCLK_ON PORTB |= (1 << 0)
#define SCLK_OFF PORTB &= ~(1 << 0)
// processed_data (IO) on B1
#define IO_ON PORTB |= (1 << 1)
#define IO_OFF  PORTB &= ~(1 << 1)
// RST on B2
#define CE_ON PORTB |= (1 << 2)
#define CE_OFF PORTB &= ~(1 << 2)

// Button 1 on D7: start time mode
#define BUTTON_1_PRESSED (!(PIND & (1<<PIND7)))
// Button 2 on B3: For time setting mode
#define BUTTON_2_PRESSED (!(PINB & (1<<PINB3)))
// Button 3 on B4. Increase value in setting mode
#define BUTTON_3_PRESSED (!(PINB & (1<<PINB4)))
// Button 4 on B5. Decrease value in setting mode.
#define BUTTON_4_PRESSED (!(PINB & (1<<PINB5)))

// LED 1 on C4: Indicate set mode. (green)
#define LED_1_ON PORTC  |= (1<<4)
#define LED_1_OFF PORTC  &= ~(1<<4)
// LED 2 on C5: Indicate time mode. (red)
#define LED_2_ON PORTC  |= (1<<5)
#define LED_2_OFF PORTC  &= ~(1<<5)

// Based on processed_data sheet, we can identify the address for reading/writing 
// time from RTC. 
int second_W = 0x80, second_R = 0x81;
int minute_W = 0x82, minute_R = 0x83;
int hour_W = 0x84, hour_R = 0x85;

volatile unsigned int second = 0;
volatile int hour = 0 , minute = 0;
volatile int digit_disp[4] = {}; // create array with 4 elements

typedef enum {idle, set_hours, set_minutes, time_mode} STATES;
STATES  clk_state;

void SYSTEM_CONFIGURATION(void)
{
  /*PORT_
   *  D0 ~D6: output. for 7segment of LED
   *  D7: input. button 1. 
   *  B0~B2: DS1301 communication. 
   *  B3,B4, B5: input. button 2, button 3 and button 4
   *  C0~C3: output. control digit 1~4 of the clock.
   *  C4,C5: output. LED 1 and LED 2. 
   */
  DDRD |= (127 <<0);  // 7 segments of LED. 
  DDRD &=~ (1<<7);// button 1
  DDRB &= ~(7<<3); // button 2,3,4. 
  DDRC |= (15<<0); // Digit 1 ~digit 4 of LED. 
  DDRC |= (3<<4); // LED1, LED2 
}
void SEGMENT_PATTERN(int number)
{
	switch(number)
	{
		case 0:
		PORTD = 0b01000000;
		break;
		case 1:
		PORTD = 0b01111001;
		break;
		case 2:
		PORTD = 0b00100100;
		break;
		case 3:
		PORTD = 0b00110000;
		break;
		case 4:
		PORTD = 0b00011001 ;
		break;
		case 5:
		PORTD = 0b00010010;
		break;
		case 6:
		PORTD = 0b00000010;
		break;
		case 7:
		PORTD = 0b01111000;
		break;
		case 8:
		PORTD = 0b00000000;
		break;
		case 9:
		PORTD = 0b00010000;
		break;
	}
}
// Function for displaying on 4 digit - 7 segment LED
void CLOCK_DISPLAY(void) {
    digit_disp[0] = minute%10; 
    digit_disp[1] = minute/10;
    digit_disp[2] = hour%10; 
    digit_disp[3] = hour/10; 

    // for loop for displaying 4 digit.
    for (int i = 0; i<4; i++)
    {
      PORTC &= ~(15<<0); // turn off D1~D4 
      // turn on D1 to D4 respectively to display segment on conresponding digit.
      PORTC |= (1<<i); 
      SEGMENT_PATTERN(digit_disp[i]);
      _delay_ms(1); 
    }
}


/*
 * We can control and communicate with DS1302 via CE, SCLK, IO (processed_data line)
 * CE: Whenever we want to read/write to any address of RTC. CE have 
to be set to HIGH.If the CE input is low, all processed_data transfer terminates and 
the I/O pin goes to a high-impedance state.
* By turn off and on SCLK, we create SCLK cycles used for read/write processed_data byte of each address. 
* Each address contain 8 bytes. => for loop is used to check and read/write processed_data
of each address. 
* For processed_data output:  a processed_data byte is output on the falling edge of the
next eight SCLK cycles
* for processed_data input: a processed_data byte is input on the rising edge of the next
eight SCLK cycles
 */
// Access to each BYTE of the given address to write processed_data from MCU to GS1302
void ADDRESS_LOCATING(unsigned char address_location)
{
  DDRB |=(7<<0); // all SCLK, CE, IO as output.
  for(int i = 0 ; i <= 7; i++)
  {
    if (address_location & (1<<i))
    {
      IO_ON;
    }
    else
    {
      IO_OFF;
    }
    // create SCLK cycle.
    SCLK_ON;
    _delay_us(10);
    SCLK_OFF;
    _delay_us(10);
  }
}
/*
 * As we write value from MCU to clock keeper. PORTB0~B2 need to be ouput for 
write data into desired address.
 */
// Write to the given address
void DS1302_WRITE(unsigned char address_location, unsigned char processed_data)
{
  CE_ON;
  ADDRESS_LOCATING(address_location);
  DDRB |= (7<<0); // CLK, DATA, RST as output.
  // Using for loop to access to each byte.
  for(int i = 0; i <= 7; i++)
  {
    if (processed_data & (1<<i))
    {
      IO_ON;
    }
    else
    {
      IO_OFF;
    }
    SCLK_ON;
    _delay_us(10);
    SCLK_OFF;
    _delay_us(10);
  }
  CE_OFF;
}

// To read data, IO has to be re-configured to be input. 
// Read data in each addresses
unsigned char DS1302_READ(unsigned char address_location)
{
  unsigned char processed_data = 0;
  CE_ON;
  ADDRESS_LOCATING(address_location);
  DDRB &= ~(1<<1); // IO as input to receive data from RTC. 
  for(int i = 0; i <= 7; i++)
  {
    if ((PINB & (1 << PINB1)) != 0) 
    {
      processed_data |= processed_data|(1<<i);
    }
    SCLK_ON;
    _delay_us(10);
    SCLK_OFF;
    _delay_us(10);
  }
  DDRB |= (1<<1); // IO back to output after finishing reading.
  CE_OFF;
  return(processed_data);
}

// convert decimal to binary coded decimal Number to write to address in RTC. 
unsigned int DEC_2_BCD(unsigned int decimal)
{
  decimal = (decimal/10)*16 + (decimal%10);
  return (decimal);
}
// convert binary coded decimal Number decimal to read processed_data from in RTC.
unsigned int BCD_2_DEC(unsigned char processed_data)
{
  processed_data = (processed_data/16)*10 + (processed_data%16);
  return (processed_data);
}

void RTC_TIME_SET()
{
  DS1302_WRITE(second_W,DEC_2_BCD(second));
  DS1302_WRITE(minute_W,DEC_2_BCD(minute));
  DS1302_WRITE(hour_W,DEC_2_BCD(hour));
}

void RTC_TIME_GET()
{
  minute = BCD_2_DEC(DS1302_READ(minute_R));
  hour = BCD_2_DEC(DS1302_READ(hour_R));
}
// Main program
int main(void)
{
  clk_state = idle; // clock initial state
  SYSTEM_CONFIGURATION();
  while(1)
  {
    switch(clk_state)
    {
      case idle:
        LED_1_OFF;
        LED_2_OFF;
        if (BUTTON_2_PRESSED) // if button 2 is pressed
        {
          _delay_ms(150);// delay 150ms for debouncing the button
          clk_state = set_hours;
        }
      case set_hours:
        LED_1_ON;
        LED_2_OFF;
        RTC_TIME_GET();
        if (BUTTON_1_PRESSED) // if button 1 is pressed
        {
          _delay_ms(150);// delay 150ms for debouncing the button
          clk_state = time_mode;
        }
        if (BUTTON_2_PRESSED) // if button 2 is pressed
        {
          _delay_ms(150);// delay 150ms for debouncing the button
          clk_state = set_minutes;
        }
        if ((BUTTON_3_PRESSED) && (hour < 23)) // if button 3 is pressed. Increase
        {
          hour++;
          _delay_ms(150);// delay 150ms for de-bouncing the button
        }
        if ((BUTTON_4_PRESSED)&& (hour > 0)) // if button 4 is pressed. Decrease
        {
          hour--;
          _delay_ms(150);// delay 150ms for de-bouncing the button
        }
         RTC_TIME_SET();
         CLOCK_DISPLAY(); // Display on 4digit-7seg LED.
      break;
      case set_minutes:
        LED_1_ON;
        LED_2_OFF;
        RTC_TIME_GET();
        if (BUTTON_1_PRESSED)// if button 1 is pressed
		{
			_delay_ms(150); // delay 150ms for de-bouncing the button
			clk_state = time_mode;
		}
        if (BUTTON_2_PRESSED) // if button 2 is pressed
        {
          _delay_ms(150);// delay 150ms for de-bouncing the button
          clk_state = set_hours;
        }
        if ((BUTTON_3_PRESSED)&&(minute <59)) // if button 3 is pressed. Increase
        {
          minute++;
          _delay_ms(150);// delay 150ms for debouncing the button
        }
        if ((BUTTON_4_PRESSED)&&(minute > 0)) // if button 4 is pressed. Decrease
        {
          
          minute--;
          _delay_ms(150);// delay 150ms for debouncing the button
        }
         RTC_TIME_SET();
         CLOCK_DISPLAY(); // Display on 4digit-7seg LED.
      break;
      case time_mode:
        LED_1_OFF;
        LED_2_ON;
        if (BUTTON_2_PRESSED) // if button 2 is pressed. Back to set mode.
        {
          _delay_ms(150);
          clk_state = set_hours;
        }
        RTC_TIME_GET();
        // display time.
        CLOCK_DISPLAY(); // Display on 4digit-7seg LED.
      break;
    }
  }
}
