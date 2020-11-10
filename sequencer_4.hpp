/* MIDI SEQUENCER.hpp
 *   by Lut99
 *
 * Created:
 *   10/11/2020, 13:23:21
 * Last edited:
 *   10/11/2020, 14:50:39
 * Auto updated?
 *   Yes
 *
 * Version: 4.0.0
 *
 * Description:
 *   This file implements a Midi Sequencer on a Arduino Uno. It assumes a
 *   specific pin layout, but is designed that these should be easily
 *   changeable.
 * 
 *   For a complete list of features, refer to the wiki in the GitHub repo
 *   (https://github.com/Lut99/ArduinoSequencer/wiki). A complete changelog can
 *   be found there as well.
**/

#ifndef MIDI_SEQUENCER
#define MIDI_SEQUENCER

/***** CONSTANTS *****/



/***** PIN LAYOUTS *****/

// First output pin for multiplexers 1 and 2
#define MULT12_OUT1 2
// Second output pin for multiplexers 1 and 2
#define MULT12_OUT2 3
// Third output pin for multiplexers 1 and 2
#define MULT12_OUT3 4
// First output pin for multiplexers 3 and 4
#define MULT34_OUT1 5
// Second output pin for multiplexers 3 and 4
#define MULT34_OUT2 6
// Third output pin for multiplexers 3 and 4
#define MULT34_OUT3 7
// Data pin for the first multiplexer, which is here used to read the potmeters
#define MULT1_DATA A0
// Data pin for the second multiplexer, which is here used to write LED values
#define MULT2_DATA A1
// Data pin for the third multiplexer, which is here used to read the control potmeters
#define MULT3_DATA A2
// Data pin for the fourth multiplexer, which is here used flexibly
#define MULT4_DATA A3



/***** HELPER FUNCTIONS *****/



/***** MAIN *****/

/* First function that is run for the Arduino, which should initialize it to a usable state. */
void setup();
/* Function that is called repeatedly by the Arduino as main loop. */
void loop();

#endif
