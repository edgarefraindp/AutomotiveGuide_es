/*
 * lectorCanBus.ino - CAN Bus Reader
 * 
 * Diagnostic tool to verify activity on CAN bus when conventional scanners fail
 * 
 * This tool allows:
 * 1. Verify if there is activity on the CAN network
 * 2. Display received CAN messages
 * 3. Check termination resistors
 * 4. Attempt to communicate with specific modules
 * 
 * Required hardware connections:
 * - MCP2515 module connected via SPI to Arduino
 * - Optional 120 ohm termination resistor
 * - 16x2 LCD display with I2C adapter (optional)
 * 
 * Date: 04/20/2025
 * 
 * Part of the AutomotiveGuide_es project
 * https://github.com/edgarefraindp/AutomotiveGuide_es
 */

#include <SPI.h>
#include <mcp_can.h>
#include <LiquidCrystal_I2C.h>

// Pin definitions
const int PIN_CS_CAN = 10;      // CS (Chip Select) pin for MCP2515 module
const int PIN_INT_CAN = 2;      // INT (Interrupt) pin for MCP2515 module
const int PIN_LED_RX = 7;       // LED that blinks when receiving CAN messages
const int PIN_LED_ERROR = 8;    // LED that indicates bus error
const int PIN_BTN_MODE = 3;     // Button to change operation mode
const int PIN_SWITCH_TERM = 4;  // Switch to enable/disable termination resistor

// Constants
const unsigned long BAUDRATE_SERIAL = 115200;  // Serial port speed
const unsigned long REFRESH_INTERVAL = 500;    // Reading refresh interval in ms
const byte CAN_SPEED = CAN_500KBPS;           // CAN bus speed (adjust according to vehicle)
const byte CRYSTAL_MHZ = MCP_16MHZ;           // Crystal frequency of MCP2515 module

// Global variables
byte operationMode = 0;             // Current operation mode
unsigned long lastRefreshTime = 0;  // Time control for updates
unsigned int messagesReceived = 0;  // Counter for received messages
bool canInitialized = false;        // Initialization state of CAN module
bool termResistorEnabled = false;   // State of termination resistor

// Objects
MCP_CAN CAN(PIN_CS_CAN);                      // Object to control CAN module
LiquidCrystal_I2C lcd(0x27, 16, 2);          // Object to control LCD display

// Variables for message statistics
unsigned int msgStats[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // Counters by ID
unsigned long lastMessageTime = 0;                    // Time of last message

void setup() {
  // Initialize serial communication
  Serial.begin(BAUDRATE_SERIAL);
  Serial.println(F("CAN Bus Reader - Diagnostic Tool"));
  Serial.println(F("AutomotiveGuide_es"));
  Serial.println(F("--------------------------------------"));

  // Configure pins
  pinMode(PIN_LED_RX, OUTPUT);
  pinMode(PIN_LED_ERROR, OUTPUT);
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_SWITCH_TERM, INPUT_PULLUP);
  pinMode(PIN_INT_CAN, INPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CAN Bus Reader");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  // Check termination resistor switch
  UpdateTerminationResistor();
  
  // Initialize CAN controller
  InitializeCANController();
  
  // Display instructions
  PrintInstructions();
}

void loop() {
  // Check termination resistor switch
  CheckTerminationSwitch();
  
  // Check mode button
  CheckModeButton();
  
  // Process received CAN messages
  ProcessCANMessages();
  
  // Update display based on interval
  unsigned long currentTime = millis();
  if (currentTime - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime = currentTime;
    UpdateDisplay();
    
    // Check communication timeout (2 seconds without messages = error)
    if (messagesReceived > 0 && (currentTime - lastMessageTime) > 2000) {
      SignalError();
    }
  }
}

void InitializeCANController() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting CAN...");
  
  // Attempt to initialize CAN controller
  byte retries = 0;
  canInitialized = false;
  
  while (retries < 3 && !canInitialized) {
    if (CAN.begin(CAN_SPEED, CRYSTAL_MHZ) == CAN_OK) {
      canInitialized = true;
      Serial.println(F("CAN initialized successfully"));
      
      lcd.setCursor(0, 1);
      lcd.print("OK! 500kbps");
      digitalWrite(PIN_LED_ERROR, LOW);
      delay(1000);
    } else {
      retries++;
      Serial.println(F("Error initializing CAN. Retrying..."));
      
      lcd.setCursor(0, 1);
      lcd.print("Error: Retry");
      digitalWrite(PIN_LED_ERROR, HIGH);
      delay(1000);
    }
  }
  
  if (!canInitialized) {
    Serial.println(F("ERROR: Failed to initialize CAN controller"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CAN ERROR!");
    lcd.setCursor(0, 1);
    lcd.print("Check connections");
    
    // Keep error LED on
    digitalWrite(PIN_LED_ERROR, HIGH);
  } else {
    // Additional CAN controller configuration
    CAN.setMode(MCP_NORMAL);  // Set operation mode to normal
    
    // Simple filters to avoid saturation
    // This allows all messages but can be modified to filter
    CAN.init_Mask(0, 0, 0x00000000);
    CAN.init_Mask(1, 0, 0x00000000);
    
    // Clear display for operational mode
    lcd.clear();
  }
}

void ProcessCANMessages() {
  if (!canInitialized) return;
  
  byte len = 0;
  byte buf[8];
  unsigned long canId;
  
  // Check if messages are available
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    // Read the message
    if (CAN.readMsgBuf(&canId, &len, buf) == CAN_OK) {
      // Update statistics
      messagesReceived++;
      lastMessageTime = millis();
      
      // Classify by ID for statistics (simplified)
      byte idCategory = (canId & 0x700) >> 8;  // Use significant bits of ID
      if (idCategory < 8) {
        msgStats[idCategory]++;
      }
      
      // Blink reception LED
      digitalWrite(PIN_LED_RX, HIGH);
      
      // Display data based on mode
      switch (operationMode) {
        case 0:  // Basic monitor mode
          PrintBasicInfo(canId, len, buf);
          break;
        case 1:  // Detailed monitor mode
          PrintDetailedInfo(canId, len, buf);
          break;
        case 2:  // Statistics mode
          // Statistics are displayed in UpdateDisplay()
          break;
        case 3:  // Specific diagnostic mode
          ProcessDiagnosticRequest(canId, len, buf);
          break;
      }
      
      // Turn off LED after a short period
      delay(5);
      digitalWrite(PIN_LED_RX, LOW);
    }
  }
}

void PrintBasicInfo(unsigned long canId, byte len, byte *buf) {
  // Basic format: ID - [Data in hex]
  Serial.print(F("ID: 0x"));
  Serial.print(canId, HEX);
  
  Serial.print(F(" ["));
  for (int i = 0; i < len; i++) {
    Serial.print(buf[i], HEX);
    if (i < len - 1) {
      Serial.print(F(" "));
    }
  }
  Serial.println(F("]"));
}

void PrintDetailedInfo(unsigned long canId, byte len, byte *buf) {
  // Detailed format with message type and interpretation
  Serial.print(F("ID: 0x"));
  Serial.print(canId, HEX);
  Serial.print(F(" Len:"));
  Serial.print(len);
  
  // Interpret message type based on ID
  Serial.print(F(" Type:"));
  if (canId < 0x100) {
    Serial.print(F("Engine"));
  } else if (canId < 0x200) {
    Serial.print(F("Trans"));
  } else if (canId < 0x400) {
    Serial.print(F("Chassis"));
  } else if (canId < 0x600) {
    Serial.print(F("Body"));
  } else if (canId < 0x7E0) {
    Serial.print(F("Info"));
  } else if (canId >= 0x7E0 && canId <= 0x7EF) {
    Serial.print(F("DIAG"));
  } else {
    Serial.print(F("Other"));
  }
  
  // Display data
  Serial.print(F(" ["));
  for (int i = 0; i < len; i++) {
    if (buf[i] < 0x10) Serial.print(F("0"));
    Serial.print(buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.print(F("] ASCII:"));
  
  // Display as ASCII if possible
  for (int i = 0; i < len; i++) {
    if (buf[i] >= 32 && buf[i] <= 126) {
      Serial.write(buf[i]);
    } else {
      Serial.print(F("."));
    }
  }
  Serial.println();
}

void ProcessDiagnosticRequest(unsigned long canId, byte len, byte *buf) {
  // Process responses to diagnostic requests
  if (canId >= 0x7E8 && canId <= 0x7EF) {  // Diagnostic responses
    Serial.print(F("DIAG RESP ["));
    Serial.print(canId - 0x7E0, HEX);
    Serial.print(F("]: "));
    
    if (len > 2 && buf[0] > 0) {
      byte serviceId = buf[1];
      if (serviceId == 0x41) {  // Response to mode 01 (current data)
        Serial.print(F("Mode 01 PID "));
        Serial.print(buf[2], HEX);
        
        // Interpret some common PIDs
        InterpretCommonPIDs(buf[2], &buf[3], len-3);
      } else if (serviceId == 0x42) {  // Response to mode 02 (frozen data)
        Serial.println(F("Frozen data"));
      } else {
        Serial.print(F("Mode "));
        Serial.print(serviceId - 0x40, HEX);
      }
    }
    Serial.println();
  }
}

void InterpretCommonPIDs(byte pid, byte *data, byte len) {
  // Simplified interpretation of some common PIDs
  switch (pid) {
    case 0x0C:  // RPM
      if (len >= 2) {
        int rpm = ((data[0] * 256) + data[1]) / 4;
        Serial.print(F(" = RPM: "));
        Serial.print(rpm);
      }
      break;
    case 0x0D:  // Speed
      if (len >= 1) {
        Serial.print(F(" = Speed: "));
        Serial.print(data[0]);
        Serial.print(F(" km/h"));
      }
      break;
    case 0x05:  // Coolant temperature
      if (len >= 1) {
        Serial.print(F(" = Temp: "));
        Serial.print(data[0] - 40);
        Serial.print(F("°C"));
      }
      break;
    default:
      // For other PIDs just display the data in hex
      Serial.print(F(" = "));
      for (int i = 0; i < len; i++) {
        if (data[i] < 0x10) Serial.print(F("0"));
        Serial.print(data[i], HEX);
        Serial.print(F(" "));
      }
  }
}

void UpdateDisplay() {
  if (!canInitialized) return;
  
  lcd.clear();
  
  // First line based on mode
  lcd.setCursor(0, 0);
  switch (operationMode) {
    case 0:
      lcd.print("Basic monitor");
      break;
    case 1:
      lcd.print("Detailed monitor");
      break;
    case 2:
      lcd.print("Statistics");
      break;
    case 3:
      lcd.print("Diagnostic");
      break;
  }
  
  // Second line: relevant information based on mode
  lcd.setCursor(0, 1);
  switch (operationMode) {
    case 0:
    case 1:
      // Display messages per second
      unsigned long currentTime = millis();
      static unsigned int lastCount = 0;
      static unsigned long lastCalcTime = 0;
      static int msgsPerSecond = 0;
      
      if (currentTime - lastCalcTime >= 1000) {
        msgsPerSecond = messagesReceived - lastCount;
        lastCount = messagesReceived;
        lastCalcTime = currentTime;
      }
      
      lcd.print("Msgs: ");
      lcd.print(msgsPerSecond);
      lcd.print("/s");
      
      // Display if termination resistor is active
      lcd.setCursor(13, 1);
      lcd.print(termResistorEnabled ? "TRM" : "   ");
      break;
    
    case 2:
      // In statistics mode display distribution by type
      unsigned long total = 0;
      byte maxIndex = 0;
      
      // Find the most frequent message type
      for (int i = 0; i < 8; i++) {
        total += msgStats[i];
        if (msgStats[i] > msgStats[maxIndex]) {
          maxIndex = i;
        }
      }
      
      if (total > 0) {
        lcd.print("ID:");
        lcd.print(maxIndex);
        lcd.print(" ");
        lcd.print(map(msgStats[maxIndex]*100, 0, total, 0, 100));
        lcd.print("% ");
        lcd.print(total);
      } else {
        lcd.print("No messages");
      }
      break;
      
    case 3:
      // In diagnostic mode send periodic requests
      static byte currentPID = 0;
      
      // Display current PID
      lcd.print("PID:");
      if (currentPID < 0x10) lcd.print("0");
      lcd.print(currentPID, HEX);
      
      // Send diagnostic request for this PID
      SendDiagnosticRequest(0x01, currentPID);  // Mode 01, current PID
      
      // Rotate to next PID every certain time
      currentPID = (currentPID + 1) % 32;  // Limit to first 32 PIDs
      break;
  }
}

void SendDiagnosticRequest(byte mode, byte pid) {
  byte data[8] = {0x02, mode, pid, 0, 0, 0, 0, 0};
  byte result = CAN.sendMsgBuf(0x7DF, 0, 8, data);  // Standard diagnostic ID
  
  if (result != CAN_OK) {
    Serial.println(F("Error sending diagnostic request"));
  }
}

void CheckModeButton() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  
  bool buttonState = digitalRead(PIN_BTN_MODE);
  
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Switch to next mode
      operationMode = (operationMode + 1) % 4;
      
      // Update display immediately
      UpdateDisplay();
      
      // Visual indication
      digitalWrite(PIN_LED_RX, HIGH);
      delay(100);
      digitalWrite(PIN_LED_RX, LOW);
      
      // Display new mode on serial
      Serial.print(F("Mode changed to: "));
      
      switch (operationMode) {
        case 0:
          Serial.println(F("Basic Monitor"));
          break;
        case 1:
          Serial.println(F("Detailed Monitor"));
          break;
        case 2:
          Serial.println(F("Statistics"));
          break;
        case 3:
          Serial.println(F("Diagnostic"));
          break;
      }
    }
  }
  
  lastButtonState = buttonState;
}

void CheckTerminationSwitch() {
  // Read termination resistor switch state
  bool currentState = (digitalRead(PIN_SWITCH_TERM) == LOW);
  
  // If changed, update
  if (currentState != termResistorEnabled) {
    termResistorEnabled = currentState;
    UpdateTerminationResistor();
  }
}

void UpdateTerminationResistor() {
  // Here a physical termination resistor would be enabled/disabled 
  // via a relay or transistor if the hardware supports it
  
  Serial.print(F("Termination resistor: "));
  Serial.println(termResistorEnabled ? F("ENABLED") : F("DISABLED"));
}

void SignalError() {
  digitalWrite(PIN_LED_ERROR, HIGH);
  delay(50);
  digitalWrite(PIN_LED_ERROR, LOW);
}

void PrintInstructions() {
  Serial.println();
  Serial.println(F("=== CAN BUS READER - INSTRUCTIONS ==="));
  Serial.println(F("Button 1: Change mode (Monitor/Statistics/Diagnostic)"));
  Serial.println(F("Switch 1: Enable/disable termination resistor"));
  Serial.println();
  Serial.println(F("Available modes:"));
  Serial.println(F("0 - Basic Monitor: Displays ID and data in simple format"));
  Serial.println(F("1 - Detailed Monitor: Displays complete message information"));
  Serial.println(F("2 - Statistics: Displays message type distribution"));
  Serial.println(F("3 - Diagnostic: Attempts to communicate with modules using OBD2 protocol"));
  Serial.println();
  Serial.println(F("RX LED: Blinks when receiving messages"));
  Serial.println(F("ERROR LED: Indicates initialization or communication issues"));
  Serial.println(F("======================================"));
}