/* SEQUENCER
 *   by Lut99
 * 
 * An eight-step sequencer with the ARDUINO.
 * 
 * TODO:
 *   - Send MIDI timesync when in 1-3 => Should be working, but doesn't (seem to) :(
 *   
 * IDEAS:
 *   - ???
 * 
 * NOTE: This is the old, 3.0 version of the sequencer, and only kept her as reference.
 */


/***** INCLUDES *****/
// Nothing yet...



/***** CONSTANTS *****/

/* Comment to disable MIDI-mode. */
#define MIDI_MODE

/* The beats per minute of the slowest tempo of the button. */
#define TEMPO_SLOW 40
/* The default tempo. Must be in between TEMPO_SLOW and TEMPO_FAST (but can take either values). */
#define TEMPO_DEFAULT 120
/* The beats per minute of the fastest tempo of the button. */
#define TEMPO_FAST 360
/* The distance between the memory value and the actual value before the button is activated. */
#define TRIGGER_THRESHOLD 5
/* Amount of times a button or switch has to consistently tell us the same value for it to count. */
#define DEBOUNCE_SEQUENCE 10
/* Number of milliseconds that a LED is on (and the same time off) when in channel mode and channel >= 8. */
#define CHANNEL_BLINK_DELAY 250

/* Active sensing delay (in ms) */
#define MIDI_ACTIVE_SENSE_DELAY 200

/* The channel of the sequencer. */
#define DEFAULT_CHANNEL 0
/* The note length that is by default. */
#define DEFAULT_NOTE_LENGTH 10
/* The pitch at the left of the knob */
#define LOW_PITCH 42
/* The pitch in the center of the knob */
#define DEFAULT_PITCH 60
/* The pitch at the right of the knob */
#define HIGH_PITCH 78
/* The velocity that is by default. */
#define DEFAULT_VELOCITY 100

/* The CC that is by default. */
#define DEFAULT_CC_VALUE 0
/* The CC value of the first set of notes (DEFAULT: cutoff). */
#define CC_1 74
/* The CC value of the second set of notes (DEFAULT: resonance). */
#define CC_2 71



/***** PIN LAYOUT *****/

/* LED pins */
#define LED_START 2

/* TEMPO pin */
#define TEMPO_PIN A2

/* Multiplexer pins */
#define MULTI_IN A1
#define MULTI_1 11
#define MULTI_2 12
#define MULTI_3 13

/* Big button */
#define CHOICE_PIN A0

/* Half time / normal time / double time switch. */
#define XTIME_PIN A4

/* The pin of the rundmode switch */
#define RUNMODE_PIN A5

/* The pin determining if we're in CHANNEL mode or not. */
#define CHANNELMODE_PIN 10

/* The small turning knob. */
#define SMALLKNOB_PIN A3



enum class RunMode {
  forward,
  random,
  backward
};



/***** GLOBALS *****/

int count;
int i;
int beat_part;
int bar_part;
long bar_count;
int choice;
int prev_choice;
float tempo;
float timeout;

byte volume;
byte prev_volume;
unsigned long long last_volume_ping;

short* values;
byte* enabled;
byte* prev_enabled;

short note_length_lifetime[8];
short note_length[8];
byte note_length_enabled;

short note_pitch[8];
byte note_pitch_enabled;

short note_velocity[8];
byte note_velocity_enabled;

short note_cc[8];
byte note_cc_last[2];
byte note_cc_enabled;

bool midichannel_enabled;
bool tempo_enabled;

byte note_state[24];

unsigned long long last_beat_ping;
unsigned long long last_choice_ping;
unsigned long long last_active_sense_ping;
unsigned long long last_channel_blink_ping;

bool play;
bool midi_play;

bool onoff_override;
bool onoff_value;
int onoff_dbnc_count;

RunMode runmode;
byte runmode_dbnc_count;

bool channelmode;
byte channelmode_dbnc_count;

float xtime;
byte xtime_dbnc_count;

byte midi_channel;
byte prev_midi_channel;


/***** HEADERS *****/
extern void noteOn(byte led, short pitch, short velocity);
extern void midiOn(byte pitch, byte velocity);
extern void noteOff(byte led);
extern void midiOff(byte pitch);
extern void noteCCOn(byte led, byte group, short value);
extern void noteCCOff(byte led);

extern inline int writeBool(byte data, byte pin, bool value);
extern inline bool readBool(byte data, byte pin);

extern void play_reset();
extern void play_start();
extern void play_continue();
extern void play_stop();

extern int readMulti(byte pin);
extern int readChoice();
extern RunMode readRunmode();
extern float readXtime();



void setup() {
  /***** GLOBAL INIT *****/
  count = 0;
  i = 0;
  beat_part = 0;
  bar_part = 0;
  bar_count = 0;
  choice = 1;
  prev_choice = 1;
  // Give the last_on and last_off at least a 60 second delay before they start playing (on button read will override this)
  last_beat_ping = 60000;
  last_choice_ping = 0;
  last_active_sense_ping = 0;
  last_channel_blink_ping = 0;
  for (byte i = 0; i < 8; i++) {
    note_length[i] = DEFAULT_NOTE_LENGTH;
    note_pitch[i] = ((float) DEFAULT_PITCH - (float) LOW_PITCH) / ((float) HIGH_PITCH - (float) LOW_PITCH) * 1023.0 + 0.5;
    note_velocity[i] = DEFAULT_VELOCITY * 8;
    note_cc[i] = DEFAULT_CC_VALUE;
    if (i == 0 || i == 1) {
      note_cc_last[i] = DEFAULT_CC_VALUE;
    }
  }
  note_length_enabled = 0x0;
  note_pitch_enabled = 0x0;
  note_velocity_enabled = 0x0;
  note_cc_enabled = 0x0;
  values = NULL;
  enabled = NULL;
  prev_enabled = NULL;

  tempo = 1.0 - ((float) TEMPO_DEFAULT - (float) TEMPO_SLOW) / ((float) TEMPO_FAST - (float) TEMPO_SLOW);

  midi_channel = DEFAULT_CHANNEL;
  prev_midi_channel = midi_channel;

  play = false;
  midi_play = false;

  volume = -1;
  
  onoff_override = false;
  onoff_value = false;
  onoff_dbnc_count = 0;

  runmode = RunMode::forward;
  runmode_dbnc_count = 0;

  channelmode = false;
  channelmode_dbnc_count = 0;

  tempo_enabled = false;
  midichannel_enabled = false;

  xtime = 1;
  xtime_dbnc_count = 0;
  
  /***** PIN SETUP *****/
  pinMode(MULTI_1, OUTPUT);
  pinMode(MULTI_2, OUTPUT);
  pinMode(MULTI_3, OUTPUT);
  pinMode(MULTI_IN, INPUT);

  // Setup the LEDs
  for (byte i = 0; i < 8; i++) {
    pinMode(LED_START + i, OUTPUT);
  }

  pinMode(TEMPO_PIN, INPUT);
  pinMode(CHOICE_PIN, INPUT);
  pinMode(XTIME_PIN, INPUT);
  pinMode(RUNMODE_PIN, INPUT);
  pinMode(CHANNELMODE_PIN, INPUT_PULLUP);
  pinMode(SMALLKNOB_PIN, INPUT);

  /***** SERIAL SETUP *****/
  #ifdef MIDI_MODE
  Serial.begin(31250);
  #else
  Serial.begin(9600);
  #endif
}



void loop() {
  bool advance = false;
  
  // Read the on / off switch
  if ((analogRead(RUNMODE_PIN) < 500) != onoff_value) {
    // Choose the action based on the count
    if (onoff_dbnc_count < DEBOUNCE_SEQUENCE) {
      ++onoff_dbnc_count;
    } else {
      onoff_value = !onoff_value;
      if (onoff_value) {
        play_start();
      } else {
        for (byte i = 0; i < 8; i++) {
          digitalWrite(LED_START + i, LOW);
        }
        play_stop();
      }
    }
  } else {
    // Set the count to zero again
    onoff_dbnc_count = 0;
  }



  // Read if we're going into channel mode
  bool channelmode_new = digitalRead(CHANNELMODE_PIN);
  if (channelmode_new != channelmode) {
    if (channelmode_dbnc_count < DEBOUNCE_SEQUENCE) {
      ++channelmode_dbnc_count;
    } else {
      channelmode = channelmode_new;
      channelmode_dbnc_count = 0;

      // Turn all LEDS off whenever we change
      for (int i = 0; i < 8; ++i) { digitalWrite(LED_START + i, LOW); }

      // Disable both locks
      tempo_enabled = false;
      midichannel_enabled = false;

      // Set the prev_midichannel on something wack so we always trigger it
      prev_midi_channel = 17;
    }
  }



  // Read the volume, only occassionally, so let's lift on the choice timer
  if (volume == -1) {
    // But the first time, make sure to not send
    volume = 127 - analogRead(SMALLKNOB_PIN) / 8;
    prev_volume = volume;
    last_volume_ping = millis();
  }
  if (millis() - last_volume_ping >= 10) {
    volume = 127 - analogRead(SMALLKNOB_PIN) / 8;
    if (volume != prev_volume) {
      #ifdef MIDI_MODE
      // Send a CC message
      Serial.write(0xB0 | (midi_channel & 0xF));
      Serial.write(0x07);
      Serial.write(volume & 0x7F);
      #endif

      // Update the previous volume
      prev_volume = volume;
    }
    last_volume_ping = millis();
  }
  


  // Read the Runmode independt of the beat, as we debounce
  RunMode runmode_now = readRunmode();
  RunMode new_runmode = runmode;
  if (runmode_now != runmode) {
    if (runmode_dbnc_count < DEBOUNCE_SEQUENCE) {
      ++runmode_dbnc_count;
    } else {
      new_runmode = runmode_now;
      runmode_dbnc_count = 0;
    }
  }

  // Read the Runmode & Xtime pins, but only when on the start of a whole bar
  if ((runmode == RunMode::random || count == 0) && bar_part == 0 && beat_part == 0) {
    // Xtime
    xtime = readXtime();
    
    // Update the runmode only now
    runmode = new_runmode;

    // If we switched to non-random, let's reset to the 0
    if (runmode != RunMode::random) {
      count = 0;
    }
  }

  

  // Read the tempo pin and use that to possible enabled advance
  if (choice < 4 || choice > 6) {
    // Only read if we're allowed to
    if (!channelmode) {
      if (tempo_enabled) {
        tempo = analogRead(TEMPO_PIN) / 1023.0;
      } else if (abs(tempo * 1023 - analogRead(TEMPO_PIN)) < TRIGGER_THRESHOLD) {
        // Enable the lock since we're close enough to the read value
        tempo_enabled = true;
      }
    }
    
    // Compute the timeout (2500 = 60000 / 24)
    timeout = 2500 / (TEMPO_SLOW + (1 - tempo) * (TEMPO_FAST - TEMPO_SLOW));

    if (millis() - last_beat_ping >= timeout) {
      advance = true;

      // If we're in 1-3, send a midi timesync code
      if (choice >= 1 && choice <= 3) {
        #ifdef MIDI_MODE
        Serial.write(0xF8);
        #endif
      }
      
      last_beat_ping += timeout + 0.5;
    }
  }

  // Read from MIDI
  #ifdef MIDI_MODE
  if (Serial.available()) {
    byte midi_byte = (byte) Serial.read();

    // Always send it to Serial out again!
    Serial.write(midi_byte);

    if (choice >= 4 && choice <= 6) {
      if (midi_byte == 0xFA) {
        // START command
        midi_play = true;
        play_start();
      } else if (midi_byte == 0xFB) {
        // CONTINUE command
        midi_play = true;
        play_continue();
      } else if (midi_byte == 0xFC) {
        // STOP command
        midi_play = false;
        play_stop();
      } else if (midi_byte == 0xF8) {
        // TIME SYNC command
        advance = true;
      } else if (midi_byte == 0xF2) {
        // SONG POSITION command (always reset to zero)
        play_reset();
      }
    }
  }
  #endif



  // If we're on channel mode, set the leds to reflect which channel
  if (channelmode) {
    // Read the channel if we're allowed to
    if (midichannel_enabled) {
      midi_channel = 15 - analogRead(TEMPO_PIN) / 64;
    } else if (midi_channel == 15 - analogRead(TEMPO_PIN) / 64) {
      // Enable the channel
      midichannel_enabled = true;
    }
    
    // Loop through all the LEDS to blink 'em
    if (midi_channel != prev_midi_channel) {
      // Turn the old one off
      digitalWrite(LED_START + prev_midi_channel % 8, LOW);
      // Turn the new one on
      digitalWrite(LED_START + midi_channel % 8, HIGH);
    } else if (midi_channel >= 8 && millis() - last_channel_blink_ping >= CHANNEL_BLINK_DELAY) {
      // Blink it
      digitalWrite(LED_START + midi_channel - 8, millis() % (CHANNEL_BLINK_DELAY * 2) < CHANNEL_BLINK_DELAY);
      last_channel_blink_ping = millis();
    }

    prev_midi_channel = midi_channel;
  }

  

  // Read the choice pin
  if (values == NULL || millis() - last_choice_ping >= 100) {
    choice = readChoice();

    // If the choice changed, switch everything briefly off
    if (prev_choice != choice) {
      if ((prev_choice - 1) / 3 != (choice - 1) / 3) {
        play_stop();
        if (onoff_value && (choice < 4 || choice > 6 || midi_play)) {
          play_start();
        } else {
          play_reset();
        }
      }
      
      prev_choice = choice;
    }

    // Update the written-to lists
    if (choice == 1 || choice == 4) {
      // Choice 1 & 4 : Length
      values = note_length;
      enabled = &note_length_enabled;
    } else if (choice == 2 || choice == 5) {
      // Choice 2 & 5 : Pitch
      values = note_pitch;
      enabled = &note_pitch_enabled;
    } else if (choice == 3 || choice == 6) {
      // Choice 3 & 6 : Velocity
      values = note_velocity;
      enabled = &note_velocity_enabled;
    } else if (choice == 7) {
      // Choice 7     : Control Change
      values = note_cc;
      enabled = &note_cc_enabled;
    }
    if (prev_enabled != enabled) {
      if (prev_enabled != NULL) {
        *prev_enabled = 0x0;
      }
    }
    prev_enabled = enabled;
    
    last_choice_ping = millis();
  }

  // Read the values into this array 'n' stuff
  if (readBool(*enabled, i)) {
    values[i] = readMulti(i);
  } else if (abs(values[i] - readMulti(i)) < TRIGGER_THRESHOLD) {
    *enabled = writeBool(*enabled, i, true);
  }

  // Move to the next beat
  if (advance && isCorrectBeat()) {
    // Only update and turn on if off
    if (note_length_lifetime[count] <= 0) {
      // Turn the node on if the length is in an enable range
      short value = note_length[count];
      if (play && value >= 10) {
        if (choice >= 0 && choice <= 6) {
          // Turn this node on
          note_length_lifetime[count] = value / 1023.0 * 16 * 24;
          noteOn(count, note_pitch[count] / 1023.0 * (HIGH_PITCH - LOW_PITCH) + LOW_PITCH, note_velocity[count] / 8);
        } else if (choice == 7) {
          // Send a CC with the value of the note
          note_length_lifetime[count] = DEFAULT_NOTE_LENGTH;
          noteCCOn(count, count % 2, note_cc[count] / 8);
        }
      }
    }

    // Increment the counter
    if (play) {
      if (runmode == RunMode::forward) {
        if (++count > 7) { count = 0; }
      } else if (runmode == RunMode::random) {
        count = random(8);
      } else if (runmode == RunMode::backward) {
        if (--count < 0) { count = 7; }
      }
    }
  }

  // Turning off has 24x more precision
  if (play && advance) {
    // Decrease the duration of all notes and turn them off if needed
    for (byte j = 0; j < 8; j++) {
      if (note_length_lifetime[j] > 0) {
        --note_length_lifetime[j];
      } else if (note_length_lifetime[j] == 0) {
        --note_length_lifetime[j];
        if (choice >= 0 && choice <= 6) {
          noteOff(j);
        } else if (choice == 7) {
          noteCCOff(j);
        }
      }
    }
  }

  if (play) {
    if (++i > 7) { i = 0; }
    if (advance) {
      if (++beat_part > 23) {
        beat_part = 0;
        if (++bar_part > 3) {
          bar_part = 0;
          ++bar_count;
        }
      }
    }
  }

  // Send an active sense message each every time
  #ifdef MIDI_MODE
  if (millis() - last_active_sense_ping >= MIDI_ACTIVE_SENSE_DELAY) {
    Serial.write(0xFE);

    last_active_sense_ping = millis();
  }
  #endif
}



void noteOn(byte led, short pitch, short velocity) {
  if (!channelmode) { digitalWrite(LED_START + led, HIGH); }

  // Also send a note
  midiOn(pitch, velocity, midi_channel);
  // Note the pitch and data we set them on with
  note_state[(led * 3)] = pitch;
  note_state[(led * 3) + 1] = velocity;
  note_state[(led * 3) + 2] = midi_channel;
}

void midiOn(byte pitch, byte velocity, byte channel) {
  #ifdef MIDI_MODE
  // Also send a note
  Serial.write(0x90 | (channel & 0xF));
  Serial.write(pitch & 0x7F);
  Serial.write(velocity & 0x7F);
  #endif
}

void noteOff(byte led) {
  if (!channelmode) { digitalWrite(LED_START + led, LOW); }

  midiOff(note_state[(led * 3)], note_state[(led * 3) + 2]);
}

void midiOff(byte pitch, byte channel) {
  #ifdef MIDI_MODE
  // Also send a note
  Serial.write(0x80 | (channel & 0xF));
  Serial.write(pitch & 0x7F);
  Serial.write(0x00);
  #endif
}

void noteCCOn(byte led, byte group, short value) {
  if (!channelmode) { digitalWrite(LED_START + led, HIGH); }

  // Update the value of the group if and only if it changed
  if (note_cc_last[group] != value) {
    note_cc_last[group] = value;

    #ifdef MIDI_MODE
    // Send a CC message
    Serial.write(0xB0 | (midi_channel & 0xF));
    Serial.write((group ? CC_1 : CC_2) & 0x7F);
    Serial.write(value & 0x7F);
    #endif
  }
}

void noteCCOff(byte led) {
  if (!channelmode) { digitalWrite(LED_START + led, LOW); }
}



inline int writeBool(byte data, byte pin, bool value) {
  return value ? data ^ (0x1 << pin) : data & (~(0x1 << pin));
}

inline bool readBool(byte data, byte pin) {
  return (data >> pin) & 0x1;
}



void play_reset() {
  // Reset the position of the sequencer
  count = 0;
  i = 0;
  beat_part = 0;
  bar_part = 0;

  // Reset the timings, but make sure they do not underflow
  unsigned long now = millis();
  if (now < timeout) {
    last_beat_ping = 0;
  } else {
    last_beat_ping = now - timeout;
  }

  // Reset the lifetimes of each note & turn them off
  for (byte i = 0; i < 8; i++)  {
    note_length_lifetime[i] = -1;
    if(!channelmode) { digitalWrite(LED_START + i, LOW); }
  }
}

void play_start() {
  // Reset the relevant data
  play_reset();
  
  // Do not play if the main switch is not on and the midi does not allow it
  if (!onoff_value || (choice >= 4 && choice <= 6 && !midi_play)) { return; }

  // Resume the program
  play = true;
}

void play_continue() {
  // Do nothing if the main switch is not on and the midi does not allow it
  if (play || !onoff_value || (choice >= 4 && choice <= 6 && !midi_play)) { return; }
  
  // Set the notes on again that were on
  for (byte i = 0; i < 8; i++) {
    if (note_length_lifetime[i] > 0) {
      midiOn(note_state[(i * 3)], note_state[(i * 3) + 1], note_state[(i * 3) + 2]);
    }
  }
  
  // Only resume the play if the master value is true & midi allows it / no midi
  play = true;
}

void play_stop() {
  // Do not start if we're not playing
  if (!play) { return; }

  // Send a stop for all notes that were on
  for (byte i = 0; i < 8; i++) {
    if (note_length_lifetime[i] > 0) {
      midiOff(note_state[(i * 3)], note_state[(i * 3) + 2]);
    }
  }

  // Halt the playback and reset the midi-override by form of reset
  play = false;
}



bool isCorrectBeat() {
  return (xtime == 2 && (beat_part == 0 || beat_part == 5 || beat_part == 11 || beat_part == 17)) ||
         (xtime == 1 && (beat_part == 0 || beat_part == 11)) ||
         (xtime == 0.5 && beat_part == 0);
}



int readMulti(byte pin) {
  // Read the bits from the number
  byte r0 = pin & 0x1;
  byte r1 = (pin >> 1) & 0x1;
  byte r2 = (pin >> 2) & 0x1;

  // Write to the pins
  digitalWrite(MULTI_1, r0);
  digitalWrite(MULTI_2, r1);
  digitalWrite(MULTI_3, r2);

  // Return the value
  return analogRead(MULTI_IN);
}

int readChoice() {
  int value = analogRead(CHOICE_PIN);
  if (value >= 0 && value < 200) { return 7; }
  else if (value >= 200 && value < 300) { return 6; }
  else if (value >= 300 && value < 360) { return 5; }
  else if (value >= 360 && value < 500) { return 4; }
  else if (value >= 500 && value < 600) { return 3; }
  else if (value >= 600 && value < 1000) { return 2; }
  else { return 1; }
}

RunMode readRunmode() {
  int value = analogRead(RUNMODE_PIN);
  if (value >= 100 && value < 500) { return RunMode::random; }
  else if (value >= 50 && value < 100) { return RunMode::backward; }
  else { return RunMode::forward; } // value < 50
}

float readXtime() {
  int value = analogRead(XTIME_PIN);
  if (value > 500) {
    return 2;
  } else if (value < 50) {
    return 1;
  } else {
    return 0.5;
  }
}
