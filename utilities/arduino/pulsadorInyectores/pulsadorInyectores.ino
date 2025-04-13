/*
 * INJECTOR PULSER
 * 
 * This program simulates pulse signals for 4 fuel injectors
 * using different firing orders common in 4-cylinder engines.
 * 
 * The pulse speed is controlled by a potentiometer connected
 * to an analog input, simulating the throttle position.
 * 
 * Repository: https://github.com/edgarefraindp/AutomotiveGuide_es
 * For support or to donate: Visit the GitHub repository page
 * 
 * Proyecto creado para AutomotiveGuide_es
 * 
 */

// PIN DEFINITIONS
#define PIN_INJECTOR_1    9     // Pin for Injector #1
#define PIN_INJECTOR_2    10    // Pin for Injector #2
#define PIN_INJECTOR_3    11    // Pin for Injector #3
#define PIN_INJECTOR_4    12    // Pin for Injector #4
#define PIN_THROTTLE      A0    // Analog input for potentiometer (throttle)

// OPTIONAL BUTTONS AND LEDS
#define PIN_ORDER_BUTTON  2     // Button to change firing order
#define PIN_STATUS_LED    13    // Status indicator LED (built into Arduino)

// SYSTEM PARAMETERS
#define MIN_PULSE_TIME    2     // Minimum pulse time in ms (throttle at minimum)
#define MAX_PULSE_TIME    15    // Maximum pulse time in ms (throttle at maximum)
#define MIN_CYCLE_TIME    20    // Minimum time between cycles in ms (maximum RPM)
#define MAX_CYCLE_TIME    200   // Maximum time between cycles in ms (minimum RPM)

// FIRING ORDERS FOR 4-CYLINDER ENGINES
// Each order is an array with the sequence of injectors to activate
const byte ORDER_1342[] = {0, 2, 3, 1}; // Order 1-3-4-2 (common in many inline 4-cylinder engines)
const byte ORDER_1243[] = {0, 1, 3, 2}; // Order 1-2-4-3 (some European engines)
const byte ORDER_1324[] = {0, 2, 1, 3}; // Order 1-3-2-4 (some Japanese engines)

// Global variables
byte currentOrder = 0;          // 0=1342, 1=1243, 2=1324
byte currentPosition = 0;       // Current position in the sequence (0-3)
unsigned long previousTime = 0; // For time control
int pulseWidth = 5;             // Initial pulse width in ms
int cycleTime = 100;            // Initial time between cycles in ms
boolean orderChanged = false;   // Flag to detect order change

void setup() {
  // Configure output pins (injectors)
  pinMode(PIN_INJECTOR_1, OUTPUT);
  pinMode(PIN_INJECTOR_2, OUTPUT);
  pinMode(PIN_INJECTOR_3, OUTPUT);
  pinMode(PIN_INJECTOR_4, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);
  
  // Configure input pins
  pinMode(PIN_ORDER_BUTTON, INPUT_PULLUP);
  
  // Initialize all injectors to off
  digitalWrite(PIN_INJECTOR_1, LOW);
  digitalWrite(PIN_INJECTOR_2, LOW);
  digitalWrite(PIN_INJECTOR_3, LOW);
  digitalWrite(PIN_INJECTOR_4, LOW);
  
  // Start serial communication for debugging
  Serial.begin(9600);
  Serial.println("Injector Pulse Simulator");
  Serial.println("------------------------");
  PrintCurrentOrder();
}

void loop() {
  // Read potentiometer and adjust cycle time and pulse width
  int throttleValue = analogRead(PIN_THROTTLE);
  
  // Map throttle value to cycle time and pulse
  // Note: When throttle increases, cycle time decreases (RPM increases)
  cycleTime = map(throttleValue, 0, 1023, MAX_CYCLE_TIME, MIN_CYCLE_TIME);
  
  // Pulse width increases with throttle (more fuel)
  pulseWidth = map(throttleValue, 0, 1023, MIN_PULSE_TIME, MAX_PULSE_TIME);
  
  // Check if button was pressed to change order
  if (digitalRead(PIN_ORDER_BUTTON) == LOW && !orderChanged) {
    orderChanged = true;
    currentOrder = (currentOrder + 1) % 3; // Rotate among the 3 orders
    PrintCurrentOrder();
  }
  
  // Reset flag when button is released
  if (digitalRead(PIN_ORDER_BUTTON) == HIGH) {
    orderChanged = false;
  }
  
  // Time control for injection sequence
  unsigned long currentTime = millis();
  if (currentTime - previousTime >= cycleTime) {
    previousTime = currentTime;
    ActivateNextInjector();
  }
}

// Activates the next injector in the sequence according to selected order
void ActivateNextInjector() {
  // Turn off all injectors
  digitalWrite(PIN_INJECTOR_1, LOW);
  digitalWrite(PIN_INJECTOR_2, LOW);
  digitalWrite(PIN_INJECTOR_3, LOW);
  digitalWrite(PIN_INJECTOR_4, LOW);
  
  // Determine which injector to activate based on current order
  byte injector = 0;
  
  switch(currentOrder) {
    case 0: // Order 1-3-4-2
      injector = ORDER_1342[currentPosition];
      break;
    case 1: // Order 1-2-4-3
      injector = ORDER_1243[currentPosition];
      break;
    case 2: // Order 1-3-2-4
      injector = ORDER_1324[currentPosition];
      break;
  }
  
  // Activate the corresponding injector
  ActivateInjector(injector);
  
  // Blink status LED for visualization
  digitalWrite(PIN_STATUS_LED, HIGH);
  
  // Advance to the next position in the sequence
  currentPosition = (currentPosition + 1) % 4;
  
  // Show information on serial monitor
  Serial.print("Injector: ");
  Serial.print(injector + 1); // +1 to show 1-4 instead of 0-3
  Serial.print(" | Cycle: ");
  Serial.print(cycleTime);
  Serial.print("ms | Pulse: ");
  Serial.print(pulseWidth);
  Serial.println("ms");
  
  // Turn off LED after pulse time
  delay(pulseWidth);
  digitalWrite(PIN_STATUS_LED, LOW);
}

// Activates a specific injector
void ActivateInjector(byte injectorNum) {
  switch(injectorNum) {
    case 0:
      digitalWrite(PIN_INJECTOR_1, HIGH);
      break;
    case 1:
      digitalWrite(PIN_INJECTOR_2, HIGH);
      break;
    case 2:
      digitalWrite(PIN_INJECTOR_3, HIGH);
      break;
    case 3:
      digitalWrite(PIN_INJECTOR_4, HIGH);
      break;
  }
}

// Shows the current firing order in the serial monitor
void PrintCurrentOrder() {
  Serial.print("Firing order: ");
  
  switch(currentOrder) {
    case 0:
      Serial.println("1-3-4-2");
      break;
    case 1:
      Serial.println("1-2-4-3");
      break;
    case 2:
      Serial.println("1-3-2-4");
      break;
  }
}