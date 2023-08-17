# Simple_clock_DS1302 

**Description**: 

Simple clock with Arduino UNO and Real Time Clock DS1302. This project aim to explore I2C communication protocol between MCU and prripheral device. 

**Components**.

- Arduino UNO:
- DS1302 : Clock keeper
- 4-digit 7 segment LED: Display time.
- 4 buttons: Controlling clock: 1 button to start clock. 1 button to swtich digit. 2 buttons for increment and decrement value.  
- 2 LED: 1 LED indicate clock in time adjusting state and 1 LED indicate clock in operating state (clock counting).

**Software Platform**

Atmel Studio

**Functionality**: 

The clock system have basic functionalities of which expected from usual clock. User can: 
- Pause and start clock by button switch. 
- Adjusting time: Hours, Minutes, Seconds with input button switch.

Note: Project don't use internal built-in clock of arduino UNO.

**Hardware setup**: 

Arduino port: 
- D0~D6: Segment display for 4digit-7segment LED 
- D7: button 1. 
- B0~B2: DS1301 communication port. 
- B3,B4, B5: Button 2, button 3 and button 4 respectively.
- C0~C3: Control digit 1 ~ 4 of 4-digit-7segment LED.
- C4,C5: LED 1 and LED 2. 

**Usage** 

After compiling and uploading code to Arduino Uno, the clock system will be in idle state when it will wait for user to set up the clock. At idle state, both LED is OFF
- Press button 2 to start setting up the desired time. User can switch between hours and minutes by pressing button 2.
- Button 3 for increment the value of the clock.
- Button 4 for increment the value of the clock.
- After finishing setting up the clock to the desired time, user can start the counting of the clock by pressing button 1.
- User can press button 2 to edit hours-minutes anytime by pressing button 2.

