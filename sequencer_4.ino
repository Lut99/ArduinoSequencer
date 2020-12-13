/* MIDI SEQUENCER.ino
 *   by Lut99
 *
 * Created:
 *   10/11/2020, 13:20:57
 * Last edited:
 *   13/12/2020, 17:26:05
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

/* Led that is currently on. */
uint8_t led;
/* Used to keep track where we are in a bar (=all eight notes). */
int8_t bar_pos;
/* Used to keep track where we are in the music. */
uint32_t song_pos;

/* Used to determine whether we need to move to the next beat. */
unsigned long beat_clock;

/* Used to keep track if a node is on. */
uint8_t note_on;
/* Used to keep track of how long a node is on. */
uint8_t note_duration[8];
/* Used to remember the status of the per-note potmeters, for the three different modes. */
uint8_t note_stat[3][8];





/***** MAIN FUNCTIONS *****/

/* First function that is run for the Arduino, which should initialize it to a usable state. */
void setup() {
    /* Step 1: Initialize constants. */
    
    // We start at the beginning of the bar & song
    led = 0;
    beat_clock = 0;
    bar_pos = -1;
    song_pos = 0;



    /* Step 2: Initialize the note arrays. */

    // Set all notes to off, initially
    note_on = 0;
    // Set each of the note_stat nested arrays
    memset(note_stat[LENGTH], DEFAULT_LENGTH, 8);
    memset(note_stat[PITCH], DEFAULT_PITCH, 8);
    memset(note_stat[VELOCITY], DEFAULT_VELOCITY, 8);



    /* Step 3: Setup pin layouts. */

    // Setup the green led
    pinMode(GREEN_LED, OUTPUT);

    // Setup out pins for multiplexer 1 and 2
    pinMode(MULT12_OUT1, OUTPUT);
    pinMode(MULT12_OUT2, OUTPUT);
    pinMode(MULT12_OUT3, OUTPUT);
    // Multiplexer 1 is used to read potmeter data
    pinMode(MULT1_DATA, INPUT);
    // Multiplexer 2 is used to write beat led outputs
    pinMode(MULT2_DATA, OUTPUT);

    // Setup out pins for multiplexer 3 and 4
    pinMode(MULT34_OUT1, OUTPUT);
    pinMode(MULT34_OUT2, OUTPUT);
    pinMode(MULT34_OUT3, OUTPUT);
    // Multiplexer 3 is used to read control potmeters
    pinMode(MULT3_DATA, INPUT);



    /* Step 4: Set the LEDS on */
    SET_MULTI(MULT34, 2);
    digitalWrite(MULT2_DATA, HIGH);

    // Initialize the serial print
    Serial.begin(9600);

}

/* Function that is called repeatedly by the Arduino as main loop. */
void loop() {
    // Get the current time
    unsigned long now = millis();

    /* Step 1: Increment the beat position if we need to. */
    bool pos = false;
    bool beat = false;
    if (bar_pos == -1 || now - beat_clock >= BPM_TO_MS(DEFAULT_TEMPO)) {
        // Don't forget to advance the clock
        if (bar_pos >= 0) {
            beat_clock += BPM_TO_MS(DEFAULT_TEMPO);
        }
        
        // Increment out position in the beat & song
        ++bar_pos;
        if (bar_pos >= 96) {
            bar_pos = 0;
            ++song_pos;
        }

        // Mark that we need to turn then next led on if we had 12 sub-beat timers
        pos = true;
        if (bar_pos % 12 == 0) {
            beat = true;
        }
    }



    /* Step 2: Possibly turn any led off. */
    if (pos) {
        for (int i = 0; i < 8; i++) {
            if (GET_BOOL(note_on, i)) {
                // Check if that caused it to turn off
                if (--note_duration[i] == 0) {
                    // Turn it off
                    SET_BOOL(note_on, i, false);
                }
            }
        }
    }



    /* Step 3: Turn on the appropriate leds. */
    if (beat) {
        // Set the multiplexer position
        led = bar_pos / 12;
        SET_MULTI(MULT12, led);

        // Read the potmeter above it to determine the duration of this note & set it as the note's duration
        uint16_t raw_value = analogRead(MULT1_DATA);
        note_stat[LENGTH][led] = raw_value / 1023.0 * 96;
        note_duration[led] = note_stat[LENGTH][led];

        // Turn on the new led if there's enough time to go
        if (raw_value >= 15) {
            SET_BOOL(note_on, led, true);
        }
    }



    /* Step 4: At any given moment, write HIGH to all leds that are on. */
    for (int i = 0; i < 7; i++) {
        if (GET_BOOL(note_on, i)) {
            SET_MULTI(MULT12, i);
        }
    }
}
