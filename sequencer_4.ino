/* MIDI SEQUENCER.ino
 *   by Lut99
 *
 * Created:
 *   10/11/2020, 13:20:57
 * Last edited:
 *   10/11/2020, 15:12:39
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

#include "sequencer_4.hpp"


/***** GLOBALS *****/

/* Used to keep track where we are in a bar (=all eight notes). */
int8_t bar_pos;
/* Used to keep track where we are in the music. */
uint32_t song_pos;

/* Used to determine whether we need to move to the next beat. */
unsigned long beat_clock;





/***** MAIN FUNCTIONS *****/

/* First function that is run for the Arduino, which should initialize it to a usable state. */
void setup() {
    /* Step 1: Initialize constants. */
    
    // We start at the beginning of the bar & song
    bar_pos = -1;
    song_pos = 0;



    /* Step 2: Setup pin layouts. */

    // Setup out pins for multiplexer 1 and 2
    pinMode(MULT12_OUT1, OUTPUT);
    pinMode(MULT12_OUT2, OUTPUT);
    pinMode(MULT12_OUT3, OUTPUT);
    // Multiplexer 1 is used to read potmeter data
    pinMode(MULT1_DATA, INPUT);
    // Multiplexer 2 is used to write beat led outputs
    pinMode(MULT2_DATA, OUTPUT);

    // Initialize the serial print
    Serial.begin(9600);

}

/* Function that is called repeatedly by the Arduino as main loop. */
void loop() {
    unsigned long now = millis();

    /* Step 1: Increment the beat position if we need to. */
    bool beat = false;
    if (bar_pos == -1 || now - beat_clock >= 1000 / ((120 / 60.0) * 24.0)) {
        ++bar_pos;
        if (bar_pos >= 96) {
            bar_pos = 0;
            ++song_pos;
        }

        // Do led if we had 12 sub-beat timers
        if (bar_pos % 12 == 0) {
            beat = true;
        }

        // Don't forget to advance the clock, syncing on the start of the beat
        if (bar_pos == 0) { beat_clock = now; }
        else { beat_clock += 1000 / ((120 / 60.0) * 24.0); }
    }



    /* Step 2: Turn on the appropriate leds. */
    if (beat) {
        // Turn off the old led
        digitalWrite(MULT2_DATA, LOW);

        // Set the multiplexer position
        digitalWrite(MULT12_OUT1, (bar_pos / 12) & 0x1);
        digitalWrite(MULT12_OUT2, ((bar_pos / 12) >> 1) & 0x1);
        digitalWrite(MULT12_OUT3, ((bar_pos / 12) >> 2) & 0x1);

        // Turn on the new led
        digitalWrite(MULT2_DATA, HIGH);
    }
}
