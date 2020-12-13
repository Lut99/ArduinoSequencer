/* MIDI SEQUENCER.hpp
 *   by Lut99
 *
 * Created:
 *   10/11/2020, 13:23:21
 * Last edited:
 *   13/12/2020, 17:14:08
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

/* Index of the note lengths in the array of note statusses. */
#define LENGTH 0
/* Index of the note pitch in the array of note statusses. */
#define PITCH 1
/* Index of the note volume in the array of note statusses. */
#define VELOCITY 2

/* The default tempo (in BPM). */
#define DEFAULT_TEMPO 120
/* The standard length of each note (in the MIDI steps) */
#define DEFAULT_LENGTH 1
/* The standard pitch of each note (0-127) */
#define DEFAULT_PITCH 48
/* The standard velocity of each note (0-127) */
#define DEFAULT_VELOCITY 127


/***** PIN LAYOUTS *****/

/* Pin of the green led. */
#define GREEN_LED 11

/* First output pin for multiplexers 1 and 2. */
#define MULT12_OUT1 2
/* Second output pin for multiplexers 1 and 2. */
#define MULT12_OUT2 3
/* Third output pin for multiplexers 1 and 2. */
#define MULT12_OUT3 4
/* First output pin for multiplexers 3 and 4. */
#define MULT34_OUT1 5
/* Second output pin for multiplexers 3 and 4. */
#define MULT34_OUT2 6
/* Third output pin for multiplexers 3 and 4. */
#define MULT34_OUT3 7
/* Data pin for the first multiplexer, which is here used to read the potmeters. */
#define MULT1_DATA A0
/* Data pin for the second multiplexer, which is here used to write LED values. */
#define MULT2_DATA A1
/* Data pin for the third multiplexer, which is here used to read the control potmeters. */
#define MULT3_DATA A2
/* Data pin for the fourth multiplexer, which is here used flexibly. */
#define MULT4_DATA A3



/***** HELPER FUNCTIONS *****/

/* Sets a given multiplexer to the given index. */
#define SET_MULTI(NAME, I) \
    digitalWrite(MULT2_DATA, LOW); \
    digitalWrite(NAME ## _OUT1, (I) & 0x1); \
    digitalWrite(NAME ## _OUT2, ((I) >> 1) & 0x1); \
    digitalWrite(NAME ## _OUT3, ((I) >> 2) & 0x1); \
    digitalWrite(MULT2_DATA, HIGH);

/* Converts a given number in BPM to a timeout in milliseconds. */
#define BPM_TO_MS(BPM) \
    (1000 / (((BPM) / 60.0) * 24.0))

/* Helper macro that allows us to read N different booleans from a single N-bit number. */
#define GET_BOOL(BOOL, I) \
    ((bool) (((BOOL) >> (I)) & 0x1))
/* Helper macro that allows us to write to N different booleans in a single N-bit number. */
#define SET_BOOL(BOOL, I, VALUE) \
    ((bool) ((BOOL) = (VALUE) ? (BOOL) | (1 << (I)) : (BOOL) & (~(1 << (I)))))



/***** MAIN *****/

/* First function that is run for the Arduino, which should initialize it to a usable state. */
void setup();
/* Function that is called repeatedly by the Arduino as main loop. */
void loop();

#endif
