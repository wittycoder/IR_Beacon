/*****************************************************************************
   Copyright (C) 2014 Jeff Karau
   All rights reserved

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


   IR Transmitter beacon code for AIM and alternate testing pattern for
   Kart/Moto/Car timer system.

   Timing information from James Eli @ http://ucexperiment.wordpress.com/
   Measured AIM beacon pattern [inverted by PNA4602M]:
   high 6ms/low 624us/high 1.2ms/low 624us/high 1.2ms/low 624us [repeat]

   Alternative Private Beacon Code:
   300us ON / 1200us OFF / 300us ON / 1200us OFF / 300us ON / 6000us OFF [repeat]

   Hardware
   Tested on Arduino Pro Mini
   See hardware info @ http://wittycoder.com
*****************************************************************************/
// Includes - None for this sketch

// Globals
int pwr_led = 13; // pro mini/Uno LED power pin
int ir_led =  4; // pro mini IR LED pin
int button_int = 0; // pro mini button interrupt, PIN 2

volatile int buttonPress = 0;

// Setup routine, setup pin-outs
// TODO: Determine mode on startup based on button, allow AIM only/PRIVATE only/AIM+PRIVATE
void setup()
{
//   Serial.begin(9600);
   pinMode(ir_led, OUTPUT); // IR pin to output
   pinMode(pwr_led, OUTPUT); // Power LED to output
   pinMode(3, OUTPUT); // red
//   pinMode(5, OUTPUT); // green
//   pinMode(6, OUTPUT); // blue
//   pinMode(button_int, INPUT); // button as input
   attachInterrupt(button_int, buttonISR, LOW);
   buttonPress = 0;
   digitalWrite(3, LOW);
}

void buttonISR()
{
   static unsigned long last_interrupt_time = 0;
   unsigned long interrupt_time = millis();
   // If interrupts come faster than 200ms, assume it's a bounce and ignore
   if (interrupt_time - last_interrupt_time > 200) 
   {
      buttonPress++;
   }
   last_interrupt_time = interrupt_time;
}

void loopXmitAim()
{
   Serial.write("Transmitting AIM Code\n");
   while (0 == buttonPress)
   {
      // AIM Timing:
      // low 6ms, low 624us, high 1.2ms, low 624us, high 1.2ms, low 624us [repeat]
      pulseIR(5989);
      delayMicroseconds(622);
      pulseIR(1195);
      delayMicroseconds(622);
      pulseIR(1195);
      delayMicroseconds(622);
   }
}

void loopXmitPrivate()
{
   Serial.write("Transmitting Private Code\n");
   while (0 == buttonPress)
   {
      // Alternative private code 
      // low 6ms, high 300us, low 1.2ms, high 300us, low 1.2ms, high 300us [repeat]
      delayMicroseconds(5994);
      pulseIR(298);
      delayMicroseconds(1195);
      pulseIR(298);
      delayMicroseconds(1195);
      pulseIR(298);
   }
}

void loopXmitBoth()
{
   Serial.write("Transmitting both codes\n");
   while (0 == buttonPress)
   {
      __asm__("nop\n\t"); // 62.5ns delay, just for the hell of it and to put something in the loop
   }
}
#define BLINK_DELAY 75
void loop()
{
   // Each time the function comes back, blink the LED again
   for (int i = 0; i < 5; i++)
   {
      digitalWrite(pwr_led, HIGH); // Turn on the power indication for 5 seconds while we wait for button press
      delay(500); // Wait for button state  -  TODO ADD read somewhere after this
      digitalWrite(pwr_led, LOW); // Turn off the power indication/button press for mode
      delay(500);
      // Indicate on the LED which one is currently selected
      if ((buttonPress % 3) == 0) // Both
      {digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);}
      else if ((buttonPress % 2) == 0) // Private
      {digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);}
      else
      {digitalWrite(3, HIGH);delay(BLINK_DELAY);digitalWrite(3, LOW);delay(BLINK_DELAY);}
      Serial.write("\n");
   }
   digitalWrite(3, LOW);
   
   if ((buttonPress % 3) == 0)
   {
      buttonPress = 0;
      loopXmitBoth();
   }
   else if ((buttonPress % 2) == 0)
   {
      buttonPress = 0;
      loopXmitPrivate();
   }
   else
   {
      buttonPress = 0;
      loopXmitAim();
   }
}

// Thanks to adafruit for this method that is public domain:
// http://learn.adafruit.com/ir-sensor/making-an-intervalometer
//
// This procedure sends a 38KHz pulse to the IRledPin 
// for a certain # of microseconds. We'll use this whenever we need to send codes

void pulseIR(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait
 
  cli();  // this turns off any background interrupts
 
  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
   digitalWrite(ir_led, HIGH);  // this takes about 3 microseconds to happen
   delayMicroseconds(10);         // hang out for 10 microseconds, 16mhz is 10
   digitalWrite(ir_led, LOW);   // this also takes about 3 microseconds
   delayMicroseconds(10);         // hang out for 10 microseconds, 16mhz is 10
 
   // so 26 microseconds altogether
   microsecs -= 26;
  }
 
  sei();  // this turns them back on
}
