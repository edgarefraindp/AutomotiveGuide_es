/**
 * controlLuces1.ino - Sequential Light Control System for Vehicles
 * 
 * This sketch simulates the behavior of modern vehicle lights
 * with sequential ignition patterns and movement effects.
 * 
 * Created: 04/28/2025
 * 
 * Date: 04/20/2025
 * 
 * Part of the AutomotiveGuide_es project
 * https://github.com/edgarefraindp/AutomotiveGuide_es 
 */
#include <Arduino.h>

// ----- PIN DEFINITIONS -----

// Front light pins
const int PIN_LOW_BEAM_LEFT = 2;    // Left low beam
const int PIN_LOW_BEAM_RIGHT = 3;   // Right low beam
const int PIN_HIGH_BEAM_LEFT = 4;   // Left high beam
const int PIN_HIGH_BEAM_RIGHT = 5;  // Right high beam

// Front position light pins
const int PIN_FRONT_POSITION_LEFT = 6;    // Front left position light
const int PIN_FRONT_POSITION_RIGHT = 7;   // Front right position light

// Multiplexer control pins for turn signals (4 control bits)
// These pins will control 16 LEDs for each turn signal through a 4-to-16 multiplexer
const int PIN_MUX_BIT0 = 8;   // Multiplexer bit 0 (LSB)
const int PIN_MUX_BIT1 = 9;   // Multiplexer bit 1
const int PIN_MUX_BIT2 = 10;  // Multiplexer bit 2
const int PIN_MUX_BIT3 = 11;  // Multiplexer bit 3 (MSB)

// Enable pins for each turn signal group
const int PIN_ENABLE_FRONT_LEFT_TURN = 12;   // Enable front left turn signal
const int PIN_ENABLE_FRONT_RIGHT_TURN = 13;  // Enable front right turn signal
const int PIN_ENABLE_REAR_LEFT_TURN = 22;    // Enable rear left turn signal
const int PIN_ENABLE_REAR_RIGHT_TURN = 23;   // Enable rear right turn signal

// Rear light pins
const int PIN_REAR_POSITION_LEFT = 24;   // Rear left position light
const int PIN_REAR_POSITION_RIGHT = 25;  // Rear right position light

// Brake light pins
const int PIN_BRAKE_LEFT = 26;   // Left brake light
const int PIN_BRAKE_RIGHT = 27;  // Right brake light

// ----- INPUT PINS -----

const int PIN_IGNITION = 32;          // Vehicle ignition signal
const int PIN_BRAKE_PEDAL = 33;       // Brake pedal signal
const int PIN_LIGHT_SWITCH = 34;      // Light switch (0=off, 1=position, 2=low beam)
const int PIN_HIGH_BEAM_SWITCH = 35;  // High beam switch
const int PIN_TURN_LEFT_SWITCH = 36;  // Left turn signal switch
const int PIN_TURN_RIGHT_SWITCH = 37; // Right turn signal switch
const int PIN_HAZARD_SWITCH = 38;     // Hazard lights button

// ----- VARIABLES AND CONSTANTS FOR TURN SIGNAL CONTROL -----

// Predefined patterns for turn signal effects (16 LEDs per signal)
const uint16_t PATTERN_ALL_OFF = 0x0000;         // All LEDs off
const uint16_t PATTERN_ALL_ON = 0xFFFF;          // All LEDs on
const uint16_t PATTERN_LEFT_TO_RIGHT = 0x8000;   // Initial pattern for left-to-right sweep
const uint16_t PATTERN_RIGHT_TO_LEFT = 0x0001;   // Initial pattern for right-to-left sweep

// ----- GLOBAL VARIABLES -----

unsigned long previousMillis = 0;
unsigned long turnSignalInterval = 500;  // Interval for turn signal blinking (ms)
bool turnSignalState = false;            // Current state of turn signal blinking
bool hazardLightsOn = false;             // State of hazard lights
bool vehicleOn = false;                  // State of vehicle ignition
int lightSwitchPosition = 0;             // Position of light switch
bool highBeamActive = false;             // State of high beam lights
bool leftTurnActive = false;             // State of left turn signal
bool rightTurnActive = false;            // State of right turn signal
bool brakeActive = false;                // State of brake pedal
bool startupSequenceComplete = false;    // Indicates if startup sequence is complete
int currentTurnStep = 0;                 // Current step in turn signal sequence
uint16_t currentPattern = PATTERN_ALL_OFF; // Current pattern of turn signal LEDs

// ----- CONFIGURATION FUNCTIONS -----

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Initializing light control system...");
  
  // Configure output pins (lights)
  ConfigureOutputPins();
  
  // Configure input pins (switches)
  ConfigureInputPins();
  
  // Perform initial test of all lights
  TestAllLights();
  
  Serial.println("System initialized successfully.");
}

void ConfigureOutputPins() {
  // Configure front light pins
  pinMode(PIN_LOW_BEAM_LEFT, OUTPUT);
  pinMode(PIN_LOW_BEAM_RIGHT, OUTPUT);
  pinMode(PIN_HIGH_BEAM_LEFT, OUTPUT);
  pinMode(PIN_HIGH_BEAM_RIGHT, OUTPUT);
  pinMode(PIN_FRONT_POSITION_LEFT, OUTPUT);
  pinMode(PIN_FRONT_POSITION_RIGHT, OUTPUT);
  
  // Configure multiplexer pins
  pinMode(PIN_MUX_BIT0, OUTPUT);
  pinMode(PIN_MUX_BIT1, OUTPUT);
  pinMode(PIN_MUX_BIT2, OUTPUT);
  pinMode(PIN_MUX_BIT3, OUTPUT);
  
  // Configure enable pins for turn signals
  pinMode(PIN_ENABLE_FRONT_LEFT_TURN, OUTPUT);
  pinMode(PIN_ENABLE_FRONT_RIGHT_TURN, OUTPUT);
  pinMode(PIN_ENABLE_REAR_LEFT_TURN, OUTPUT);
  pinMode(PIN_ENABLE_REAR_RIGHT_TURN, OUTPUT);
  
  // Configure rear light pins
  pinMode(PIN_REAR_POSITION_LEFT, OUTPUT);
  pinMode(PIN_REAR_POSITION_RIGHT, OUTPUT);
  pinMode(PIN_BRAKE_LEFT, OUTPUT);
  pinMode(PIN_BRAKE_RIGHT, OUTPUT);
  
  // Initialize all pins to low state
  digitalWrite(PIN_MUX_BIT0, LOW);
  digitalWrite(PIN_MUX_BIT1, LOW);
  digitalWrite(PIN_MUX_BIT2, LOW);
  digitalWrite(PIN_MUX_BIT3, LOW);
  
  digitalWrite(PIN_ENABLE_FRONT_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_FRONT_RIGHT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_RIGHT_TURN, LOW);
}

void ConfigureInputPins() {
  // Configure input pins with internal pull-up resistors
  pinMode(PIN_IGNITION, INPUT_PULLUP);
  pinMode(PIN_BRAKE_PEDAL, INPUT_PULLUP);
  pinMode(PIN_LIGHT_SWITCH, INPUT_PULLUP);
  pinMode(PIN_HIGH_BEAM_SWITCH, INPUT_PULLUP);
  pinMode(PIN_TURN_LEFT_SWITCH, INPUT_PULLUP);
  pinMode(PIN_TURN_RIGHT_SWITCH, INPUT_PULLUP);
  pinMode(PIN_HAZARD_SWITCH, INPUT_PULLUP);
}

// ----- MULTIPLEXER CONTROL FUNCTIONS -----

/**
 * Sets the multiplexer pins according to the specified binary value (0-15)
 * @param value 4-bit binary value (0-15) to set on the multiplexer pins
 */
void SetMuxOutput(uint8_t value) {
  // Set each bit of the multiplexer
  digitalWrite(PIN_MUX_BIT0, (value & 0x01));      // Bit 0 (LSB)
  digitalWrite(PIN_MUX_BIT1, (value & 0x02) >> 1); // Bit 1
  digitalWrite(PIN_MUX_BIT2, (value & 0x04) >> 2); // Bit 2
  digitalWrite(PIN_MUX_BIT3, (value & 0x08) >> 3); // Bit 3 (MSB)
}

/**
 * Activates a specific LED in one or more turn signal groups
 * @param ledIndex Index of the LED to activate (0-15)
 * @param frontLeft Activate in front left group
 * @param frontRight Activate in front right group
 * @param rearLeft Activate in rear left group
 * @param rearRight Activate in rear right group
 */
void ActivateSingleLed(uint8_t ledIndex, bool frontLeft, bool frontRight, bool rearLeft, bool rearRight) {
  // Set the index on the multiplexer
  SetMuxOutput(ledIndex);
  
  // Activate the selected turn signal groups
  digitalWrite(PIN_ENABLE_FRONT_LEFT_TURN, frontLeft ? HIGH : LOW);
  digitalWrite(PIN_ENABLE_FRONT_RIGHT_TURN, frontRight ? HIGH : LOW);
  digitalWrite(PIN_ENABLE_REAR_LEFT_TURN, rearLeft ? HIGH : LOW);
  digitalWrite(PIN_ENABLE_REAR_RIGHT_TURN, rearRight ? HIGH : LOW);
  
  // Small delay to make the LED visible
  delayMicroseconds(100);
}

/**
 * Displays a 16-bit pattern on the turn signal LED groups
 * @param pattern 16-bit pattern where each bit represents an LED
 * @param frontLeft Display on front left group
 * @param frontRight Display on front right group
 * @param rearLeft Display on rear left group
 * @param rearRight Display on rear right group
 * @param durationMs Duration in milliseconds to display the full pattern
 */
void DisplayPattern(uint16_t pattern, bool frontLeft, bool frontRight, bool rearLeft, bool rearRight, int durationMs) {
  unsigned long startTime = millis();
  
  // Continue displaying the pattern for the specified time
  while (millis() - startTime < durationMs) {
    // Iterate through the 16 LEDs and activate those corresponding to the pattern
    for (int i = 0; i < 16; i++) {
      // Check if the corresponding bit is on in the pattern
      bool ledOn = (pattern & (1 << (15 - i))) != 0;
      
      if (ledOn) {
        // Activate the LED in the selected groups
        ActivateSingleLed(i, frontLeft, frontRight, rearLeft, rearRight);
      }
    }
  }
  
  // Turn off all groups at the end
  digitalWrite(PIN_ENABLE_FRONT_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_FRONT_RIGHT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_RIGHT_TURN, LOW);
}

/**
 * Generates a sweep pattern that moves from left to right
 * @param numLedsActive Number of LEDs active simultaneously in the sweep
 * @param stepIndex Current step of the animation (0-15)
 * @return 16-bit pattern with active LEDs
 */
uint16_t GenerateLeftToRightSweepPattern(int numLedsActive, int stepIndex) {
  uint16_t pattern = 0;
  
  // Ensure stepIndex is within the correct range
  stepIndex = stepIndex % 16;
  
  // Activate the specified number of consecutive LEDs
  for (int i = 0; i < numLedsActive; i++) {
    int ledPos = (stepIndex + i) % 16;
    pattern |= (1 << (15 - ledPos));
  }
  
  return pattern;
}

/**
 * Generates a sweep pattern that moves from right to left
 * @param numLedsActive Number of LEDs active simultaneously in the sweep
 * @param stepIndex Current step of the animation (0-15)
 * @return 16-bit pattern with active LEDs
 */
uint16_t GenerateRightToLeftSweepPattern(int numLedsActive, int stepIndex) {
  uint16_t pattern = 0;
  
  // Ensure stepIndex is within the correct range
  stepIndex = stepIndex % 16;
  
  // Activate the specified number of consecutive LEDs
  for (int i = 0; i < numLedsActive; i++) {
    int ledPos = 15 - ((stepIndex + i) % 16);
    pattern |= (1 << ledPos);
  }
  
  return pattern;
}

/**
 * Executes a complete left-to-right sweep animation
 * @param frontLeft Display on front left group
 * @param frontRight Display on front right group
 * @param rearLeft Display on rear left group
 * @param rearRight Display on rear right group
 * @param totalDurationMs Total duration of the complete animation
 */
void AnimateLeftToRightSweep(bool frontLeft, bool frontRight, bool rearLeft, bool rearRight, int totalDurationMs) {
  int numSteps = 16; // One step per LED
  int stepDuration = totalDurationMs / numSteps;
  int numActiveLedsInSweep = 4; // Number of LEDs active simultaneously
  
  for (int step = 0; step < numSteps; step++) {
    uint16_t pattern = GenerateLeftToRightSweepPattern(numActiveLedsInSweep, step);
    DisplayPattern(pattern, frontLeft, frontRight, rearLeft, rearRight, stepDuration);
  }
}

/**
 * Executes a complete right-to-left sweep animation
 * @param frontLeft Display on front left group
 * @param frontRight Display on front right group
 * @param rearLeft Display on rear left group
 * @param rearRight Display on rear right group
 * @param totalDurationMs Total duration of the complete animation
 */
void AnimateRightToLeftSweep(bool frontLeft, bool frontRight, bool rearLeft, bool rearRight, int totalDurationMs) {
  int numSteps = 16; // One step per LED
  int stepDuration = totalDurationMs / numSteps;
  int numActiveLedsInSweep = 4; // Number of LEDs active simultaneously
  
  for (int step = 0; step < numSteps; step++) {
    uint16_t pattern = GenerateRightToLeftSweepPattern(numActiveLedsInSweep, step);
    DisplayPattern(pattern, frontLeft, frontRight, rearLeft, rearRight, stepDuration);
  }
}

// ----- TEST FUNCTIONS -----

void TestAllLights() {
  // Test sequence for all lights at startup
  Serial.println("Running test of all lights...");
  
  // Turn off all lights first
  TurnOffAllLights();
  delay(500);
  
  // Test position lights
  Serial.println("Testing position lights...");
  digitalWrite(PIN_FRONT_POSITION_LEFT, HIGH);
  digitalWrite(PIN_FRONT_POSITION_RIGHT, HIGH);
  digitalWrite(PIN_REAR_POSITION_LEFT, HIGH);
  digitalWrite(PIN_REAR_POSITION_RIGHT, HIGH);
  delay(1000);
  
  // Test low beam lights
  Serial.println("Testing low beam lights...");
  digitalWrite(PIN_LOW_BEAM_LEFT, HIGH);
  digitalWrite(PIN_LOW_BEAM_RIGHT, HIGH);
  delay(1000);
  
  // Test high beam lights
  Serial.println("Testing high beam lights...");
  digitalWrite(PIN_HIGH_BEAM_LEFT, HIGH);
  digitalWrite(PIN_HIGH_BEAM_RIGHT, HIGH);
  delay(1000);
  digitalWrite(PIN_HIGH_BEAM_LEFT, LOW);
  digitalWrite(PIN_HIGH_BEAM_RIGHT, LOW);
  
  // Test brake lights
  Serial.println("Testing brake lights...");
  digitalWrite(PIN_BRAKE_LEFT, HIGH);
  digitalWrite(PIN_BRAKE_RIGHT, HIGH);
  delay(1000);
  digitalWrite(PIN_BRAKE_LEFT, LOW);
  digitalWrite(PIN_BRAKE_RIGHT, LOW);
  
  // Test left turn signals with sequential effect
  Serial.println("Testing left turn signals...");
  for (int i = 0; i < 3; i++) {
    AnimateLeftToRightSweep(true, false, true, false, 500);
    delay(200);
  }
  
  // Test right turn signals with sequential effect
  Serial.println("Testing right turn signals...");
  for (int i = 0; i < 3; i++) {
    AnimateRightToLeftSweep(false, true, false, true, 500);
    delay(200);
  }
  
  // Test hazard lights (both turn signals)
  Serial.println("Testing hazard lights...");
  for (int i = 0; i < 3; i++) {
    // Display full pattern on all groups
    DisplayPattern(PATTERN_ALL_ON, true, true, true, true, 300);
    delay(200);
    DisplayPattern(PATTERN_ALL_OFF, true, true, true, true, 300);
    delay(200);
  }
  
  // Turn off everything at the end
  TurnOffAllLights();
  delay(500);
  
  Serial.println("Light test completed.");
}

void TurnOffAllLights() {
  // Turn off all lights
  digitalWrite(PIN_LOW_BEAM_LEFT, LOW);
  digitalWrite(PIN_LOW_BEAM_RIGHT, LOW);
  digitalWrite(PIN_HIGH_BEAM_LEFT, LOW);
  digitalWrite(PIN_HIGH_BEAM_RIGHT, LOW);
  digitalWrite(PIN_FRONT_POSITION_LEFT, LOW);
  digitalWrite(PIN_FRONT_POSITION_RIGHT, LOW);
  digitalWrite(PIN_REAR_POSITION_LEFT, LOW);
  digitalWrite(PIN_REAR_POSITION_RIGHT, LOW);
  digitalWrite(PIN_BRAKE_LEFT, LOW);
  digitalWrite(PIN_BRAKE_RIGHT, LOW);
  
  // Turn off all turn signals
  TurnOffAllTurnSignals();
}

void TurnOffAllTurnSignals() {
  // Turn off all turn signal groups
  digitalWrite(PIN_ENABLE_FRONT_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_FRONT_RIGHT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_LEFT_TURN, LOW);
  digitalWrite(PIN_ENABLE_REAR_RIGHT_TURN, LOW);
  
  // Reset the current pattern
  currentPattern = PATTERN_ALL_OFF;
  currentTurnStep = 0;
}

// ----- LIGHT EFFECT FUNCTIONS -----

void SequentialTurnLeft(bool includeRear) {
  // Generate the pattern for left turn signal
  uint16_t pattern = GenerateLeftToRightSweepPattern(4, currentTurnStep);
  
  // Display the pattern on the corresponding groups
  DisplayPattern(pattern, true, false, includeRear, false, 10);
  
  // Increment the step for the next update
  currentTurnStep = (currentTurnStep + 1) % 16;
}

void SequentialTurnRight(bool includeRear) {
  // Generate the pattern for right turn signal
  uint16_t pattern = GenerateRightToLeftSweepPattern(4, currentTurnStep);
  
  // Display the pattern on the corresponding groups
  DisplayPattern(pattern, false, true, false, includeRear, 10);
  
  // Increment the step for the next update
  currentTurnStep = (currentTurnStep + 1) % 16;
}

void PerformStartupSequence() {
  // Welcome sequence when the vehicle is turned on, Audi style
  Serial.println("Executing welcome sequence...");
  
  // Sweep sequence on front lights
  digitalWrite(PIN_FRONT_POSITION_LEFT, HIGH);
  digitalWrite(PIN_FRONT_POSITION_RIGHT, HIGH);
  delay(200);
  
  // Low beam sequence
  digitalWrite(PIN_LOW_BEAM_LEFT, HIGH);
  digitalWrite(PIN_LOW_BEAM_RIGHT, HIGH);
  delay(200);
  
  // Sequence for turn signals (complete sweep effect)
  // First left-to-right on the left side
  AnimateLeftToRightSweep(true, false, true, false, 400);
  
  // Then right-to-left on the right side
  AnimateRightToLeftSweep(false, true, false, true, 400);
  
  // Activate rear lights
  digitalWrite(PIN_REAR_POSITION_LEFT, HIGH);
  digitalWrite(PIN_REAR_POSITION_RIGHT, HIGH);
  
  // Brake light flash
  digitalWrite(PIN_BRAKE_LEFT, HIGH);
  digitalWrite(PIN_BRAKE_RIGHT, HIGH);
  delay(300);
  digitalWrite(PIN_BRAKE_LEFT, LOW);
  digitalWrite(PIN_BRAKE_RIGHT, LOW);
  delay(200);
  digitalWrite(PIN_BRAKE_LEFT, HIGH);
  digitalWrite(PIN_BRAKE_RIGHT, HIGH);
  delay(300);
  digitalWrite(PIN_BRAKE_LEFT, LOW);
  digitalWrite(PIN_BRAKE_RIGHT, LOW);
  
  // Finish with a pattern on all turn signals
  DisplayPattern(PATTERN_ALL_ON, true, true, true, true, 300);
  DisplayPattern(PATTERN_ALL_OFF, true, true, true, true, 100);
  DisplayPattern(PATTERN_ALL_ON, true, true, true, true, 300);
  DisplayPattern(PATTERN_ALL_OFF, true, true, true, true, 100);
  
  Serial.println("Welcome sequence completed.");
  startupSequenceComplete = true;
  
  // Restore light state according to switches
  UpdateAllLights();
}

// ----- LIGHT CONTROL FUNCTIONS -----

void UpdatePositionLights(bool state) {
  // Update state of position lights
  digitalWrite(PIN_FRONT_POSITION_LEFT, state);
  digitalWrite(PIN_FRONT_POSITION_RIGHT, state);
  digitalWrite(PIN_REAR_POSITION_LEFT, state);
  digitalWrite(PIN_REAR_POSITION_RIGHT, state);
}

void UpdateLowBeamLights(bool state) {
  // Update state of low beam lights
  digitalWrite(PIN_LOW_BEAM_LEFT, state);
  digitalWrite(PIN_LOW_BEAM_RIGHT, state);
}

void UpdateHighBeamLights(bool state) {
  // Update state of high beam lights
  digitalWrite(PIN_HIGH_BEAM_LEFT, state);
  digitalWrite(PIN_HIGH_BEAM_RIGHT, state);
}

void UpdateBrakeLights(bool state) {
  // Update state of brake lights
  digitalWrite(PIN_BRAKE_LEFT, state);
  digitalWrite(PIN_BRAKE_RIGHT, state);
}

void UpdateTurnSignals() {
  // Update state of turn signals based on time
  unsigned long currentMillis = millis();
  
  // Check if it's time to change the state of the blinkers
  if (currentMillis - previousMillis >= turnSignalInterval) {
    previousMillis = currentMillis;
    turnSignalState = !turnSignalState;
    
    // Turn off all turn signals first
    TurnOffAllTurnSignals();
    
    // If the lights are off in this cycle, do nothing else
    if (!turnSignalState) {
      return;
    }
    
    // If hazard lights are activated
    if (hazardLightsOn) {
      // In the on state, display the current pattern on all groups
      uint16_t hazardPattern = PATTERN_ALL_ON;
      DisplayPattern(hazardPattern, true, true, true, true, 10);
    }
    // Otherwise, check individual turn signals
    else {
      if (leftTurnActive) {
        SequentialTurnLeft(true);
      }
      if (rightTurnActive) {
        SequentialTurnRight(true);
      }
    }
  }
}

void UpdateAllLights() {
  // Update all lights based on the current state
  
  // If the vehicle is off, all lights should be off
  if (!vehicleOn) {
    TurnOffAllLights();
    return;
  }
  
  // Update lights based on the position of the switch
  switch (lightSwitchPosition) {
    case 0: // Lights off
      UpdatePositionLights(false);
      UpdateLowBeamLights(false);
      break;
    case 1: // Position lights
      UpdatePositionLights(true);
      UpdateLowBeamLights(false);
      break;
    case 2: // Low beam lights
      UpdatePositionLights(true);
      UpdateLowBeamLights(true);
      break;
  }
  
  // Update high beam lights
  UpdateHighBeamLights(highBeamActive);
  
  // Update brake lights
  UpdateBrakeLights(brakeActive);
  
  // Update turn signals (handled in UpdateTurnSignals)
}

void ReadInputs() {
  // Read state of input pins
  
  // Read ignition state (active low due to INPUT_PULLUP)
  bool newVehicleOn = !digitalRead(PIN_IGNITION);
  
  // If the vehicle has just been turned on, start the welcome sequence
  if (newVehicleOn && !vehicleOn) {
    vehicleOn = true;
    startupSequenceComplete = false;
    PerformStartupSequence();
  } else if (!newVehicleOn && vehicleOn) {
    // If the vehicle has been turned off
    vehicleOn = false;
    startupSequenceComplete = false;
    TurnOffAllLights();
  } else {
    vehicleOn = newVehicleOn;
  }
  
  // Read position of the light switch (simulate a 3-position switch)
  // Note: In a real implementation, we could use two digital pins to determine the position
  int switchPin1 = digitalRead(PIN_LIGHT_SWITCH);
  int switchPin2 = digitalRead(PIN_LIGHT_SWITCH + 1); // Use the next pin to simulate
  
  if (switchPin1 == HIGH && switchPin2 == HIGH) {
    lightSwitchPosition = 0; // Lights off
  } else if (switchPin1 == LOW && switchPin2 == HIGH) {
    lightSwitchPosition = 1; // Position lights
  } else if (switchPin1 == HIGH && switchPin2 == LOW) {
    lightSwitchPosition = 2; // Low beam lights
  }
  
  // Read state of high beam lights (active low due to INPUT_PULLUP)
  highBeamActive = !digitalRead(PIN_HIGH_BEAM_SWITCH);
  
  // Read state of brake pedal (active low due to INPUT_PULLUP)
  brakeActive = !digitalRead(PIN_BRAKE_PEDAL);
  
  // Read state of turn signals (active low due to INPUT_PULLUP)
  leftTurnActive = !digitalRead(PIN_TURN_LEFT_SWITCH);
  rightTurnActive = !digitalRead(PIN_TURN_RIGHT_SWITCH);
  
  // Read state of hazard lights (active low due to INPUT_PULLUP)
  // When active, both turn signals blink together
  hazardLightsOn = !digitalRead(PIN_HAZARD_SWITCH);
  
  // If hazard lights are activated, deactivate individual turn signals
  if (hazardLightsOn) {
    leftTurnActive = false;
    rightTurnActive = false;
  }
}

// ----- MAIN FUNCTION -----

void loop() {
  // Read state of input pins
  ReadInputs();
  
  // If the vehicle is on and the startup sequence is complete
  if (vehicleOn && startupSequenceComplete) {
    // Update all lights based on the current state
    UpdateAllLights();
    
    // Handle turn signals with sequential effect
    UpdateTurnSignals();
  }
  
  // Small delay for stability
  delay(10);
}