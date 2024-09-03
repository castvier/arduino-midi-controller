#include <Encoder.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// Define the encoder pins
Encoder myEnc(2, 3);

// Define the potentiometer pin
const int potPin = A0;

// Define the button pins
const int buttonPins[] = {8, 9, 10, 11};
const int numButtons = sizeof(buttonPins) / sizeof(int);

int buttonStates[numButtons] = {0};
int lastButtonStates[numButtons] = {0};
unsigned long lastDebounceTime[numButtons] = {0};
unsigned long debounceDelay = 50;

// Last known encoder position.
long oldPosition = -999;

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastButtonStates[i] = digitalRead(buttonPins[i]);
  }
}

void loop() {
  // Check the rotary encoder
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    // Convert the encoder position to a MIDI value (0 to 127)
    int midiValue = constrain((int)newPosition, 0, 127);
    // Send MIDI CC message on channel 1
    MIDI.sendControlChange(74, midiValue, 1);  // 74 is the CC number for cutoff frequency on many synths
  }

  // Read the potentiometer
  int potValue = analogRead(potPin);
  // Map the potentiometer value to a MIDI range (0 to 127)
  int mappedValue = map(potValue, 0, 1023, 0, 127);
  // Send MIDI CC message on channel 2
  MIDI.sendControlChange(75, mappedValue, 2);  // 75 is the CC number for another parameter

  // Check the buttons
  for (int i = 0; i < numButtons; i++) {
    int reading = digitalRead(buttonPins[i]);

    if (reading != lastButtonStates[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        if (buttonStates[i] == HIGH) {
          MIDI.sendControlChange(40 + i, 127, 1);
          Serial.print("Button ");
          Serial.print(i + 1);
          Serial.println(" Pressed");
        } else if (buttonStates[i] == LOW) {
          MIDI.sendControlChange(40 + i, 0, 1);
          Serial.print("Button ");
          Serial.print(i + 1);
          Serial.println(" Released");
        }
      }
    }

    lastButtonStates[i] = reading;
  }

  delay(10);
}
