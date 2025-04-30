/**
 * banqueoEcu1.ino - ECU Bench Testing System
 * 
 * This program emulates engine sensor signals and monitors
 * ECU (Engine Control Unit) responses for diagnostic purposes.
 * 
 * Date: 04/20/2025
 * 
 * Part of the AutomotiveGuide_es project
 * https://github.com/edgarefraindp/AutomotiveGuide_es
 */

#include <Arduino.h>

// Pin definitions for sensor emulation
#define PIN_EMU_CKP        2     // CKP sensor emulation (crankshaft)
#define PIN_EMU_CMP        3     // CMP sensor emulation (camshaft)
#define PIN_EMU_MAP        4     // MAP sensor emulation (manifold pressure)
#define PIN_EMU_TPS        5     // TPS sensor emulation (throttle position)
#define PIN_EMU_MAF        6     // MAF sensor emulation (air flow)
#define PIN_EMU_ECT        7     // ECT sensor emulation (coolant temperature)
#define PIN_EMU_IAT        8     // IAT sensor emulation (intake air temperature)

// Pin definitions for actuator monitoring
#define PIN_MON_INJ1       A0    // Injector 1 monitoring
#define PIN_MON_INJ2       A1    // Injector 2 monitoring
#define PIN_MON_INJ3       A2    // Injector 3 monitoring
#define PIN_MON_INJ4       A3    // Injector 4 monitoring
#define PIN_MON_COIL       A4    // Ignition coil monitoring
#define PIN_CTRL_PUMP      A5    // Fuel pump control

// PWM configuration constants
#define PWM_FREQUENCY      490   // Frequency in Hz (typical Arduino value)
#define TPS_MAX_DUTY       255   // Maximum PWM value (8 bits in Arduino)

// Constants for simulated engine control
#define RPM_MIN            800
#define RPM_MAX            6000
#define RPM_DEFAULT        1000
#define TPS_MIN            0
#define TPS_MAX            100
#define TPS_DEFAULT        15    // 15% TPS = idle
#define MAP_MIN            20    // 20 kPa (high vacuum)
#define MAP_MAX            101   // 101 kPa (atmospheric pressure)
#define MAF_MIN            2     // g/s at idle
#define MAF_MAX            150   // g/s at full load
#define ECT_MIN            -40   // Minimum temperature (°C)
#define ECT_MAX            120   // Maximum temperature (°C)
#define ECT_DEFAULT        85    // Normal operating temperature (°C)
#define IAT_DEFAULT        25    // Default ambient temperature (°C)

// Constants for anomaly detection
#define PULSE_MARGIN       0.15  // 15% acceptable error margin
#define MIN_VALID_PULSE_US 50    // Minimum valid pulse (μs)

// Variables for CKP teeth simulation (60-2 type)
#define TOTAL_TEETH        60
#define MISSING_TEETH      2
byte currentTooth = 0;

// Structure to store simulated engine parameters
typedef struct {
  uint16_t rpm;                // Engine RPM
  byte tps;                    // Throttle position (0-100%)
  byte map;                    // MAP pressure (kPa)
  byte maf;                    // MAF flow (g/s)
  int8_t ect;                  // Coolant temperature (°C)
  int8_t iat;                  // Intake air temperature (°C)
  bool engineRunning;          // Engine state (on/off)
} EngineParams;

// Structure to store output pulse parameters
typedef struct {
  uint32_t inj1PulseWidth;     // Injector 1 pulse width (μs)
  uint32_t inj2PulseWidth;     // Injector 2 pulse width (μs)
  uint32_t inj3PulseWidth;     // Injector 3 pulse width (μs)
  uint32_t inj4PulseWidth;     // Injector 4 pulse width (μs)
  uint32_t coilDwell;          // Coil dwell time (μs)
  uint32_t lastUpdateTime;     // Last update (ms)
  bool inj1Active;             // Injector 1 state
  bool inj2Active;             // Injector 2 state
  bool inj3Active;             // Injector 3 state
  bool inj4Active;             // Injector 4 state
  bool coilActive;             // Coil state
  bool pumpActive;             // Pump state
} OutputSignals;

// Enumeration for pulse event types
typedef enum {
  EVENT_INJ1_RISING,
  EVENT_INJ1_FALLING,
  EVENT_INJ2_RISING,
  EVENT_INJ2_FALLING,
  EVENT_INJ3_RISING,
  EVENT_INJ3_FALLING,
  EVENT_INJ4_RISING,
  EVENT_INJ4_FALLING,
  EVENT_COIL_RISING,
  EVENT_COIL_FALLING
} PulseEventType;

// Structure for pulse events
typedef struct {
  PulseEventType type;
  uint32_t timestamp;          // Timestamp in microseconds
} PulseEvent;

// Global variables
EngineParams engineParams = {
  RPM_DEFAULT,
  TPS_DEFAULT,
  MAP_MAX - 20,               // ~80 kPa at idle
  MAF_MIN + 3,                // 5 g/s at idle
  ECT_DEFAULT,
  IAT_DEFAULT,
  true
};

OutputSignals outputSignals = {0};
uint32_t lastCkpPulseTime = 0;
uint32_t lastCkpUpdateTime = 0;
uint32_t lastSensorUpdateTime = 0;
uint32_t lastCommandCheckTime = 0;
uint32_t lastAnomalyCheckTime = 0;

// Buffer for serial commands
char cmdBuffer[64];
byte cmdIndex = 0;

// Variables for handling interrupts and events
volatile uint32_t injStartTimes[4] = {0};
volatile uint32_t coilStartTime = 0;
volatile bool injectorState[4] = {false};
volatile bool coilState = false;

/**
 * Calculates the PWM duty value based on the sensor value and its range
 * 
 * @param value Current sensor value
 * @param minValue Minimum value of the sensor range
 * @param maxValue Maximum value of the sensor range
 * @return Duty cycle value (0-255)
 */
uint8_t CalculatePwmDuty(int value, int minValue, int maxValue) {
  // Limit value to range
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  
  // Calculate percentage (0.0 - 1.0)
  float percentage = (float)(value - minValue) / (float)(maxValue - minValue);
  
  // Calculate duty cycle (0-255)
  return (uint8_t)(percentage * TPS_MAX_DUTY);
}

/**
 * Updates the TPS sensor signal using PWM
 */
void UpdateTpsSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.tps, TPS_MIN, TPS_MAX);
  analogWrite(PIN_EMU_TPS, duty);
}

/**
 * Updates the MAP sensor signal (implemented with PWM)
 */
void UpdateMapSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.map, MAP_MIN, MAP_MAX);
  analogWrite(PIN_EMU_MAP, duty);
}

/**
 * Updates the MAF sensor emulation
 */
void UpdateMafSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.maf, MAF_MIN, MAF_MAX);
  analogWrite(PIN_EMU_MAF, duty);
}

/**
 * Emulates the resistance of the ECT sensor (NTC) using PWM
 * In a real system, a digital potentiometer would be used
 */
void UpdateEctSensor() {
  // For temperature sensors, we use a simplification with PWM
  float resistance = 0;
  
  // Simplified calculation of NTC resistance
  if (engineParams.ect > -40 && engineParams.ect < 150) {
    // Simplified formula: R = R0 * e^(B*(1/T - 1/T0))
    // Where R0 = 2.5kOhm at 25°C, B = 3380
    float t1 = 1.0f / (273.15f + (float)engineParams.ect);
    float t0 = 1.0f / (273.15f + 25.0f);  // Reference temperature (25°C)
    resistance = 2500.0f * exp(3380.0f * (t1 - t0));
    
    // Convert to duty cycle (inversely proportional)
    float maxResistance = 20000.0f; // Maximum expected resistance
    float dutyPercent = 1.0f - (resistance / maxResistance);
    if (dutyPercent < 0) dutyPercent = 0;
    if (dutyPercent > 1) dutyPercent = 1;
    
    analogWrite(PIN_EMU_ECT, (uint8_t)(dutyPercent * TPS_MAX_DUTY));
  }
  
  Serial.print("ECT: ");
  Serial.print(engineParams.ect);
  Serial.print("°C, Resistance: ");
  Serial.print(resistance);
  Serial.println(" Ohm");
}

/**
 * Emulates the resistance of the IAT sensor (NTC) using PWM
 */
void UpdateIatSensor() {
  // Implementation similar to ECT
  float resistance = 0;
  
  if (engineParams.iat > -40 && engineParams.iat < 150) {
    float t1 = 1.0f / (273.15f + (float)engineParams.iat);
    float t0 = 1.0f / (273.15f + 25.0f);
    resistance = 2500.0f * exp(3380.0f * (t1 - t0));
    
    // Convert to duty cycle (inversely proportional)
    float maxResistance = 20000.0f;
    float dutyPercent = 1.0f - (resistance / maxResistance);
    if (dutyPercent < 0) dutyPercent = 0;
    if (dutyPercent > 1) dutyPercent = 1;
    
    analogWrite(PIN_EMU_IAT, (uint8_t)(dutyPercent * TPS_MAX_DUTY));
  }
  
  Serial.print("IAT: ");
  Serial.print(engineParams.iat);
  Serial.print("°C, Resistance: ");
  Serial.print(resistance);
  Serial.println(" Ohm");
}

/**
 * Generates CKP and CMP signals based on the current RPM
 * This function should be called periodically in the main loop
 */
void UpdateCkpCmpSignals() {
  // Calculate interval between teeth based on RPM
  // RPM = 60 / (time per revolution in seconds)
  // For a 4-stroke engine, one revolution = 2 camshaft cycles
  // Time per tooth = (60 / RPM) / effective_number_of_teeth
  uint32_t currentTime = micros();
  uint32_t teethInterval = (uint32_t)((60000000UL / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH));
  
  // Check if it's time to generate the next tooth
  if (currentTime - lastCkpUpdateTime >= teethInterval) {
    lastCkpUpdateTime = currentTime;
    
    // Increment tooth position
    currentTooth = (currentTooth + 1) % TOTAL_TEETH;
    
    // Check if we are at the position of missing teeth (58-59 in a 60-2)
    bool generatePulse = true;
    if (currentTooth >= (TOTAL_TEETH - MISSING_TEETH)) {
      generatePulse = false;
    }
    
    // Generate CKP pulse
    if (engineParams.engineRunning) {
      digitalWrite(PIN_EMU_CKP, generatePulse ? HIGH : LOW);
      
      // At a certain tooth, also generate CMP pulse (e.g., at tooth 10)
      if (currentTooth == 10) {
        digitalWrite(PIN_EMU_CMP, HIGH);
      } else if (currentTooth == 11) {
        digitalWrite(PIN_EMU_CMP, LOW);
      }
    }
  }
}

/**
 * Calculates sensor values based on physical relationships
 * For example, higher TPS (throttle) results in lower MAP (suction pressure)
 */
void CalculateSensorRelationships() {
  // Correlate TPS with MAP (higher throttle opening, higher manifold pressure)
  if (engineParams.tps < 10) {
    // Idle - high suction
    engineParams.map = MAP_MIN + 15;
  } else if (engineParams.tps > 90) {
    // Throttle almost fully open - pressure close to atmospheric
    engineParams.map = MAP_MAX - 5;
  } else {
    // Linear interpolation between minimum and maximum
    engineParams.map = MAP_MIN + ((engineParams.tps * (MAP_MAX - MAP_MIN)) / 100);
  }
  
  // Correlate TPS with MAF (higher throttle opening, higher air flow)
  if (engineParams.tps < 10) {
    // Minimum flow at idle
    engineParams.maf = MAF_MIN + 3;
  } else {
    // Non-linear interpolation (flow increases exponentially with TPS)
    float tpsPercentage = (float)engineParams.tps / 100.0f;
    float mafRange = MAF_MAX - MAF_MIN;
    engineParams.maf = MAF_MIN + (uint8_t)(mafRange * tpsPercentage * tpsPercentage);
  }
}

/**
 * Updates all values of the emulated sensors
 */
void UpdateAllSensors() {
  UpdateTpsSensor();
  UpdateMapSensor();
  UpdateMafSensor();
  UpdateEctSensor();
  UpdateIatSensor();
  
  Serial.print("Sensors updated - RPM: ");
  Serial.print(engineParams.rpm);
  Serial.print(", TPS: ");
  Serial.print(engineParams.tps);
  Serial.print("%, MAP: ");
  Serial.print(engineParams.map);
  Serial.println("kPa");
}

/**
 * Processes interrupt events for monitoring pins
 * 
 * @param pin Pin number that triggered the interrupt
 * @param state Current state of the pin (HIGH/LOW)
 */
void ProcessPinInterrupt(uint8_t pin, bool state) {
  uint32_t timestamp = micros();
  uint32_t pulseWidth = 0;
  
  switch (pin) {
    case PIN_MON_INJ1:
      if (state) { // Rising edge
        injStartTimes[0] = timestamp;
        outputSignals.inj1Active = true;
      } else {    // Falling edge
        if (injStartTimes[0] > 0) {
          pulseWidth = timestamp - injStartTimes[0];
          if (pulseWidth > MIN_VALID_PULSE_US) {
            outputSignals.inj1PulseWidth = pulseWidth;
            Serial.print("Injector 1: ");
            Serial.print(pulseWidth);
            Serial.println(" μs");
          }
        }
        outputSignals.inj1Active = false;
      }
      break;
      
    case PIN_MON_INJ2:
      if (state) { // Rising edge
        injStartTimes[1] = timestamp;
        outputSignals.inj2Active = true;
      } else {    // Falling edge
        if (injStartTimes[1] > 0) {
          pulseWidth = timestamp - injStartTimes[1];
          if (pulseWidth > MIN_VALID_PULSE_US) {
            outputSignals.inj2PulseWidth = pulseWidth;
            Serial.print("Injector 2: ");
            Serial.print(pulseWidth);
            Serial.println(" μs");
          }
        }
        outputSignals.inj2Active = false;
      }
      break;

    case PIN_MON_INJ3:
      if (state) { // Rising edge
        injStartTimes[2] = timestamp;
        outputSignals.inj3Active = true;
      } else {    // Falling edge
        if (injStartTimes[2] > 0) {
          pulseWidth = timestamp - injStartTimes[2];
          if (pulseWidth > MIN_VALID_PULSE_US) {
            outputSignals.inj3PulseWidth = pulseWidth;
            Serial.print("Injector 3: ");
            Serial.print(pulseWidth);
            Serial.println(" μs");
          }
        }
        outputSignals.inj3Active = false;
      }
      break;
      
    case PIN_MON_INJ4:
      if (state) { // Rising edge
        injStartTimes[3] = timestamp;
        outputSignals.inj4Active = true;
      } else {    // Falling edge
        if (injStartTimes[3] > 0) {
          pulseWidth = timestamp - injStartTimes[3];
          if (pulseWidth > MIN_VALID_PULSE_US) {
            outputSignals.inj4PulseWidth = pulseWidth;
            Serial.print("Injector 4: ");
            Serial.print(pulseWidth);
            Serial.println(" μs");
          }
        }
        outputSignals.inj4Active = false;
      }
      break;
      
    case PIN_MON_COIL:
      if (state) { // Rising edge
        coilStartTime = timestamp;
        outputSignals.coilActive = true;
      } else {    // Falling edge
        if (coilStartTime > 0) {
          pulseWidth = timestamp - coilStartTime;
          if (pulseWidth > MIN_VALID_PULSE_US) {
            outputSignals.coilDwell = pulseWidth;
            Serial.print("Coil Dwell: ");
            Serial.print(pulseWidth);
            Serial.println(" μs");
          }
        }
        outputSignals.coilActive = false;
      }
      break;
  }
  
  outputSignals.lastUpdateTime = millis();
}

/**
 * ISRs (Interrupt Service Routines) for monitoring pins
 * Arduino UNO/NANO only supports interrupts on specific pins
 * Arduino MEGA/DUE supports more pins with interrupts
 */

// ISR for injector 1 pin
void Inj1ISR() {
  bool state = digitalRead(PIN_MON_INJ1);
  injectorState[0] = state;
}

// ISR for injector 2 pin
void Inj2ISR() {
  bool state = digitalRead(PIN_MON_INJ2);
  injectorState[1] = state;
}

// ISR for injector 3 pin
void Inj3ISR() {
  bool state = digitalRead(PIN_MON_INJ3);
  injectorState[2] = state;
}

// ISR for injector 4 pin
void Inj4ISR() {
  bool state = digitalRead(PIN_MON_INJ4);
  injectorState[3] = state;
}

// ISR for coil pin
void CoilISR() {
  bool state = digitalRead(PIN_MON_COIL);
  coilState = state;
}

/**
 * Analyzes ECU outputs and detects anomalies
 */
void DetectAnomalies() {
  uint32_t currentTime = millis();
  
  // Only check every second to avoid console saturation
  if (currentTime - lastAnomalyCheckTime < 1000) {
    return;
  }
  
  lastAnomalyCheckTime = currentTime;
  
  // 1. Check for missing pulses in injectors
  if (engineParams.engineRunning && engineParams.rpm > 0) {
    if (outputSignals.inj1PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALY: No pulses in Injector 1");
    }
    
    if (outputSignals.inj2PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALY: No pulses in Injector 2");
    }
    
    if (outputSignals.inj3PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALY: No pulses in Injector 3");
    }
    
    if (outputSignals.inj4PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALY: No pulses in Injector 4");
    }
  }
  
  // 2. Check balance between injectors (should not have more than 15% difference)
  if (outputSignals.inj1PulseWidth > 0 && outputSignals.inj2PulseWidth > 0) {
    float ratio = (float)outputSignals.inj1PulseWidth / (float)outputSignals.inj2PulseWidth;
    if (ratio < (1.0 - PULSE_MARGIN) || ratio > (1.0 + PULSE_MARGIN)) {
      Serial.print("ANOMALY: Imbalance between injector 1 (");
      Serial.print(outputSignals.inj1PulseWidth);
      Serial.print(" μs) and injector 2 (");
      Serial.print(outputSignals.inj2PulseWidth);
      Serial.println(" μs)");
    }
  }
  
  // 3. Check that injection times increase with high TPS
  if (engineParams.tps > 50 && outputSignals.inj1PulseWidth < 3000) {
    Serial.print("ANOMALY: High TPS (");
    Serial.print(engineParams.tps);
    Serial.print("%) but low injection time (");
    Serial.print(outputSignals.inj1PulseWidth);
    Serial.println(" μs)");
  }
  
  // 4. Check correlation between RPM and coil dwell
  // Higher RPM, shorter dwell time
  if (engineParams.rpm > 3000 && outputSignals.coilDwell > 4000) {
    Serial.print("ANOMALY: High RPM (");
    Serial.print(engineParams.rpm);
    Serial.print(") but very long coil dwell (");
    Serial.print(outputSignals.coilDwell);
    Serial.println(" μs)");
  }
}

/**
 * Processes commands received via the serial port
 * 
 * @param cmdBuffer Buffer containing the received command
 * @param length Command length
 * @return true if the command was recognized and processed
 */
bool ProcessCommand(const char* cmdBuffer, size_t length) {
  if (strncmp(cmdBuffer, "rpm:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= RPM_MIN && value <= RPM_MAX) {
      engineParams.rpm = value;
      Serial.print("RPM adjusted to: ");
      Serial.println(engineParams.rpm);
      return true;
    }
  } else if (strncmp(cmdBuffer, "tps:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= TPS_MIN && value <= TPS_MAX) {
      engineParams.tps = value;
      Serial.print("TPS adjusted to: ");
      Serial.print(engineParams.tps);
      Serial.println("%");
      return true;
    }
  } else if (strncmp(cmdBuffer, "ect:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= ECT_MIN && value <= ECT_MAX) {
      engineParams.ect = value;
      Serial.print("ECT adjusted to: ");
      Serial.print(engineParams.ect);
      Serial.println("°C");
      return true;
    }
  } else if (strncmp(cmdBuffer, "iat:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= ECT_MIN && value <= ECT_MAX) { // Same range as ECT
      engineParams.iat = value;
      Serial.print("IAT adjusted to: ");
      Serial.print(engineParams.iat);
      Serial.println("°C");
      return true;
    }
  } else if (strcmp(cmdBuffer, "status") == 0) {
    Serial.println("=== System Status ===");
    Serial.print("RPM: "); Serial.println(engineParams.rpm);
    Serial.print("TPS: "); Serial.print(engineParams.tps); Serial.println("%");
    Serial.print("MAP: "); Serial.print(engineParams.map); Serial.println(" kPa");
    Serial.print("MAF: "); Serial.print(engineParams.maf); Serial.println(" g/s");
    Serial.print("ECT: "); Serial.print(engineParams.ect); Serial.println("°C");
    Serial.print("IAT: "); Serial.print(engineParams.iat); Serial.println("°C");
    Serial.print("Engine: "); Serial.println(engineParams.engineRunning ? "Running" : "Stopped");
    Serial.print("Injector 1: "); Serial.print(outputSignals.inj1PulseWidth); Serial.println(" μs");
    Serial.print("Injector 2: "); Serial.print(outputSignals.inj2PulseWidth); Serial.println(" μs");
    Serial.print("Injector 3: "); Serial.print(outputSignals.inj3PulseWidth); Serial.println(" μs");
    Serial.print("Injector 4: "); Serial.print(outputSignals.inj4PulseWidth); Serial.println(" μs");
    Serial.print("Coil Dwell: "); Serial.print(outputSignals.coilDwell); Serial.println(" μs");
    return true;
  } else if (strcmp(cmdBuffer, "start") == 0) {
    engineParams.engineRunning = true;
    Serial.println("Engine started");
    return true;
  } else if (strcmp(cmdBuffer, "stop") == 0) {
    engineParams.engineRunning = false;
    Serial.println("Engine stopped");
    return true;
  } else if (strcmp(cmdBuffer, "help") == 0) {
    Serial.println("Available commands:");
    Serial.println("  rpm:VALUE     - Adjust RPM (800-6000)");
    Serial.println("  tps:VALUE     - Adjust throttle position (0-100%)");
    Serial.println("  ect:VALUE     - Adjust coolant temperature (-40 to 120°C)");
    Serial.println("  iat:VALUE     - Adjust air temperature (-40 to 120°C)");
    Serial.println("  start         - Start engine (begin signals)");
    Serial.println("  stop          - Stop engine (halt signals)");
    Serial.println("  status        - Show current status");
    Serial.println("  help          - Show this help");
    return true;
  }
  
  return false;
}

/**
 * Checks if there are commands available on the serial port
 */
void CheckSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    // Local echo of the character
    Serial.write(c);
    
    if (c == '\n' || c == '\r') {
      // End of command
      Serial.println();
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        if (!ProcessCommand(cmdBuffer, cmdIndex)) {
          Serial.print("Unrecognized command: ");
          Serial.println(cmdBuffer);
        }
        cmdIndex = 0;
      }
    } else if (cmdIndex < sizeof(cmdBuffer) - 1) {
      // Add character to buffer
      cmdBuffer[cmdIndex++] = c;
    }
  }
}

/**
 * Checks and processes the states of monitoring pins
 * This is done in the main loop to handle interrupts
 */
void ProcessPinStates() {
  static bool lastInjState[4] = {false};
  static bool lastCoilState = false;
  
  // Check changes in injectors
  for (uint8_t i = 0; i < 4; i++) {
    if (injectorState[i] != lastInjState[i]) {
      lastInjState[i] = injectorState[i];
      uint8_t pin = 0;
      switch (i) {
        case 0: pin = PIN_MON_INJ1; break;
        case 1: pin = PIN_MON_INJ2; break;
        case 2: pin = PIN_MON_INJ3; break;
        case 3: pin = PIN_MON_INJ4; break;
      }
      ProcessPinInterrupt(pin, injectorState[i]);
    }
  }
  
  // Check changes in coil
  if (coilState != lastCoilState) {
    lastCoilState = coilState;
    ProcessPinInterrupt(PIN_MON_COIL, coilState);
  }
}

void setup() {
  // Initialize the serial port
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for the serial port to connect (only needed for boards with native USB)
  }
  
  Serial.println("\n=== ECU Bench Testing - Control Interface ===");
  Serial.println("Type 'help' to see available commands");
  
  // Configure pins for sensor emulation
  pinMode(PIN_EMU_CKP, OUTPUT);
  pinMode(PIN_EMU_CMP, OUTPUT);
  pinMode(PIN_EMU_MAP, OUTPUT);
  pinMode(PIN_EMU_TPS, OUTPUT);
  pinMode(PIN_EMU_MAF, OUTPUT);
  pinMode(PIN_EMU_ECT, OUTPUT);
  pinMode(PIN_EMU_IAT, OUTPUT);
  
  // Configure pins for monitoring with interrupts
  pinMode(PIN_MON_INJ1, INPUT_PULLUP);
  pinMode(PIN_MON_INJ2, INPUT_PULLUP);
  pinMode(PIN_MON_INJ3, INPUT_PULLUP);
  pinMode(PIN_MON_INJ4, INPUT_PULLUP);
  pinMode(PIN_MON_COIL, INPUT_PULLUP);
  pinMode(PIN_CTRL_PUMP, INPUT_PULLUP);
  
  // Attach interrupts (if available on the Arduino board)
  // Note: on Arduino UNO/Nano only pins 2 and 3 support interrupts
  // In a real implementation, adjust according to the board used
  attachInterrupt(digitalPinToInterrupt(PIN_MON_INJ1), Inj1ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_MON_INJ2), Inj2ISR, CHANGE);
  
  // Initialize system state
  digitalWrite(PIN_EMU_CKP, LOW);
  digitalWrite(PIN_EMU_CMP, LOW);
  
  // Initialize timestamps
  lastCkpUpdateTime = micros();
  lastSensorUpdateTime = millis();
  lastCommandCheckTime = millis();
  lastAnomalyCheckTime = millis();
  
  // Update initial sensors
  UpdateAllSensors();
}

void loop() {
  uint32_t currentMillis = millis();
  
  // Update CKP/CMP signals in each cycle
  UpdateCkpCmpSignals();
  
  // Process states of monitoring pins
  ProcessPinStates();
  
  // Check serial commands every 20ms
  if (currentMillis - lastCommandCheckTime >= 20) {
    lastCommandCheckTime = currentMillis;
    CheckSerialCommands();
  }
  
  // Update sensors every 500ms
  if (currentMillis - lastSensorUpdateTime >= 500) {
    lastSensorUpdateTime = currentMillis;
    
    // Calculate physical relationships between sensors
    CalculateSensorRelationships();
    
    // Update all sensors
    UpdateAllSensors();
    
    // Check anomalies
    DetectAnomalies();
  }
}