#include "ibus.h"
#include <PS2X_lib.h>


// //////////////////
// Edit here to customize

// PS2X
PS2X ps2x; 


// How often to send data?
#define UPDATE_INTERVAL 10 // milliseconds

// To disable reading a specific type of pin, set the count to 0 and remove all items from the pins list

// 1. Analog channels. Data can be read with the Arduino's 10-bit ADC.
// This gives values from 0 to 1023.
// Specify below the analog pin numbers (as for analogRead) you would like to use.
// Every analog input is sent as a single channel.
// Arduino Mega has 16 analog pins, however if your device has fewer you'll need to modify the count and pin list below

#define ANALOG_INPUTS_COUNT 6
byte analogPins[] = {PSS_LX, PSS_LY, PSAB_L2, PSS_RX, PSS_RY, PSAB_R2}; // element count MUST be == ANALOG_INPUTS_COUNT


// 2. Digital channels. Data can be read from Arduino's digital pins.
// They could be either LOW or HIGH.
// Specify below the digital pin numbers (as for digitalRead) you would like to use.
// Every pin is sent as a single channel. LOW is encoded as 0, HIGH - as 1023
// Arduino Mega has 54 digital only pins and the ability to read the analog pins as digital via pin numbers 55-68. If your device has fewer you'll need to modify the count and pin list below
#define DIGITAL_INPUTS_COUNT 0
bool digitalPins[] = {}; // element count MUST be == DIGITAL_INPUTS_COUNT


// 3. Digital bit-mapped channels. Sending a single binary state as a 16-bit
// channel is pretty wasteful. Instead, we can encode one digital input
// in each of the 16 channel bits.
// Specify below the digital pins (as for digitalRead) you would like to send as
// bitmapped channel data. Data will be automatically organized in channels.
// The first 16 pins will go in one channel (the first pin goes into the LSB of the channel).
// The next 16 pins go in another channel and so on
// LOW pins are encoded as 0 bit, HIGH - as 1.
#define DIGITAL_BITMAPPED_INPUTS_COUNT 10
long int digitalBitmappedPins[] = {PSB_BLUE, PSB_RED, PSB_PINK, PSB_GREEN, PSB_L1, PSB_R1, PSB_SELECT, PSB_START, PSB_L3, PSB_R3}; // element count MUST be == DIGITAL_BITMAPPED_INPUTS_COUNT


// Define the appropriate analog reference source. See
// https://www.arduino.cc/reference/en/language/functions/analog-io/analogreference/
// Based on your device voltage, you may need to modify this definition
#define ANALOG_REFERENCE DEFAULT

// Define the baud rate
#define BAUD_RATE 115200

// /////////////////





#define NUM_CHANNELS ( (ANALOG_INPUTS_COUNT) + (DIGITAL_INPUTS_COUNT) + (15 + (DIGITAL_BITMAPPED_INPUTS_COUNT))/16 )


IBus ibus(NUM_CHANNELS);

void setup()
{
  pinMode(13, OUTPUT);

  if (ps2x.config_gamepad(12, 10, 9, 11, true, true)) { // clock, command, attention, data

    while (1) {
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(500);
    }
    
  }

  analogReference(ANALOG_REFERENCE); // use the defined ADC reference voltage source
  Serial.begin(BAUD_RATE);           // setup serial
}

void loop()
{
  int i, bm_ch = 0;
  unsigned long time = millis();

  ps2x.read_gamepad();
  ibus.begin();

  // read analog pins - one per channel
  for(i=0; i < ANALOG_INPUTS_COUNT; i++)
    ibus.write(ps2x.Analog(analogPins[i]));

  // read digital pins - one per channel
  for(i=0; i < DIGITAL_INPUTS_COUNT; i++)
    ibus.write(ps2x.Button(digitalPins[i]) == HIGH ? 1023 : 0);

  // read digital bit-mapped pins - 16 pins go in one channel
  for(i=0; i < DIGITAL_BITMAPPED_INPUTS_COUNT; i++) {
  	int bit = i%16;
  	if(ps2x.Button(digitalBitmappedPins[i]) == HIGH)
  		bm_ch |= 1 << bit;

  	if(bit == 15 || i == DIGITAL_BITMAPPED_INPUTS_COUNT-1) {
  		// data for one channel ready
  		ibus.write(bm_ch);
  		bm_ch = 0;
  	}
  }

  ibus.end();

  time = millis() - time; // time elapsed in reading the inputs
  if(time < UPDATE_INTERVAL)
    // sleep till it is time for the next update
    delay(UPDATE_INTERVAL  - time);

}
