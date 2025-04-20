/*
 * lectorCanBus.ino
 * 
 * Herramienta diagnóstica para verificar actividad en CAN bus cuando los escáneres convencionales fallan
 * 
 * Esta herramienta permite:
 * 1. Verificar si hay actividad en la red CAN
 * 2. Mostrar los mensajes CAN recibidos
 * 3. Comprobar las resistencias terminadoras
 * 4. Intentar comunicarse con módulos específicos
 * 
 * Conexiones hardware requeridas:
 * - Módulo MCP2515 conectado por SPI al Arduino
 * - Resistencia terminadora opcional de 120 ohms
 * - Pantalla LCD 16x2 con adaptador I2C (opcional)
 * 
 * Autor: Edgar Efraín Duarte Padrón
 * Fecha: 20/04/2025
 * 
 * Parte del proyecto AutomotiveGuide_es
 * https://github.com/edgarefraindp/AutomotiveGuide_es
 */

#include <SPI.h>
#include <mcp_can.h>
#include <LiquidCrystal_I2C.h>

// Definición de pines
const int PIN_CS_CAN = 10;      // Pin CS (Chip Select) del módulo MCP2515
const int PIN_INT_CAN = 2;      // Pin INT (Interrupción) del módulo MCP2515
const int PIN_LED_RX = 7;       // LED que parpadea al recibir mensajes CAN
const int PIN_LED_ERROR = 8;    // LED que indica error en el bus
const int PIN_BTN_MODE = 3;     // Botón para cambiar el modo de operación
const int PIN_SWITCH_TERM = 4;  // Switch para activar/desactivar resistencia terminadora

// Constantes
const unsigned long BAUDRATE_SERIAL = 115200;  // Velocidad del puerto serial
const unsigned long REFRESH_INTERVAL = 500;    // Intervalo de refresco de lecturas en ms
const byte CAN_SPEED = CAN_500KBPS;           // Velocidad del bus CAN (ajustar según vehículo)
const byte CRYSTAL_MHZ = MCP_16MHZ;           // Cristal del módulo MCP2515

// Variables globales
byte operationMode = 0;             // Modo de operación actual
unsigned long lastRefreshTime = 0;  // Control de tiempo para actualizaciones
unsigned int messagesReceived = 0;  // Contador de mensajes recibidos
bool canInitialized = false;        // Estado de inicialización del módulo CAN
bool termResistorEnabled = false;   // Estado de la resistencia terminadora

// Objetos
MCP_CAN CAN(PIN_CS_CAN);                      // Objeto para controlar el módulo CAN
LiquidCrystal_I2C lcd(0x27, 16, 2);          // Objeto para controlar la pantalla LCD

// Variables para estadísticas de mensajes
unsigned int msgStats[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // Contadores por ID
unsigned long lastMessageTime = 0;                    // Tiempo del último mensaje

void setup() {
  // Inicializar comunicación serial
  Serial.begin(BAUDRATE_SERIAL);
  Serial.println(F("Lector CAN bus - Herramienta Diagnóstica"));
  Serial.println(F("AutomotiveGuide_es"));
  Serial.println(F("--------------------------------------"));

  // Configurar pines
  pinMode(PIN_LED_RX, OUTPUT);
  pinMode(PIN_LED_ERROR, OUTPUT);
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_SWITCH_TERM, INPUT_PULLUP);
  pinMode(PIN_INT_CAN, INPUT);
  
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Lector CAN Bus");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando...");
  
  // Verificar el switch de resistencia terminadora
  UpdateTerminationResistor();
  
  // Inicializar el controlador CAN
  InitializeCANController();
  
  // Mostrar instrucciones
  PrintInstructions();
}

void loop() {
  // Verificar el switch de resistencia terminadora
  CheckTerminationSwitch();
  
  // Verificar botón de modo
  CheckModeButton();
  
  // Procesar mensajes CAN recibidos
  ProcessCANMessages();
  
  // Actualizar pantalla según intervalo
  unsigned long currentTime = millis();
  if (currentTime - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime = currentTime;
    UpdateDisplay();
    
    // Verificar timeout de comunicación (2 segundos sin mensajes = error)
    if (messagesReceived > 0 && (currentTime - lastMessageTime) > 2000) {
      SignalError();
    }
  }
}

void InitializeCANController() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando CAN...");
  
  // Intentar inicializar el controlador CAN
  byte retries = 0;
  canInitialized = false;
  
  while (retries < 3 && !canInitialized) {
    if (CAN.begin(CAN_SPEED, CRYSTAL_MHZ) == CAN_OK) {
      canInitialized = true;
      Serial.println(F("CAN inicializado correctamente"));
      
      lcd.setCursor(0, 1);
      lcd.print("OK! 500kbps");
      digitalWrite(PIN_LED_ERROR, LOW);
      delay(1000);
    } else {
      retries++;
      Serial.println(F("Error al inicializar CAN. Reintento..."));
      
      lcd.setCursor(0, 1);
      lcd.print("Error: Reintento");
      digitalWrite(PIN_LED_ERROR, HIGH);
      delay(1000);
    }
  }
  
  if (!canInitialized) {
    Serial.println(F("ERROR: No se pudo inicializar el controlador CAN"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR CAN!");
    lcd.setCursor(0, 1);
    lcd.print("Rev. conexiones");
    
    // Mantener LED de error encendido
    digitalWrite(PIN_LED_ERROR, HIGH);
  } else {
    // Configuración adicional del controlador CAN
    CAN.setMode(MCP_NORMAL);  // Set operation mode to normal
    
    // Filtros sencillos para evitar saturación
    // Esto permite todos los mensajes pero se puede modificar para filtrar
    CAN.init_Mask(0, 0, 0x00000000);
    CAN.init_Mask(1, 0, 0x00000000);
    
    // Limpiar pantalla para modo operativo
    lcd.clear();
  }
}

void ProcessCANMessages() {
  if (!canInitialized) return;
  
  byte len = 0;
  byte buf[8];
  unsigned long canId;
  
  // Verificar si hay mensajes disponibles
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    // Leer el mensaje
    if (CAN.readMsgBuf(&canId, &len, buf) == CAN_OK) {
      // Actualizar estadísticas
      messagesReceived++;
      lastMessageTime = millis();
      
      // Clasificar por ID para estadísticas (simplificado)
      byte idCategory = (canId & 0x700) >> 8;  // Usar bits significativos del ID
      if (idCategory < 8) {
        msgStats[idCategory]++;
      }
      
      // Parpadear LED de recepción
      digitalWrite(PIN_LED_RX, HIGH);
      
      // Mostrar datos según el modo
      switch (operationMode) {
        case 0:  // Modo monitor básico
          PrintBasicInfo(canId, len, buf);
          break;
        case 1:  // Modo monitor detallado
          PrintDetailedInfo(canId, len, buf);
          break;
        case 2:  // Modo estadísticas
          // Las estadísticas se muestran en UpdateDisplay()
          break;
        case 3:  // Modo diagnóstico específico
          ProcessDiagnosticRequest(canId, len, buf);
          break;
      }
      
      // Apagar LED después de un breve período
      delay(5);
      digitalWrite(PIN_LED_RX, LOW);
    }
  }
}

void PrintBasicInfo(unsigned long canId, byte len, byte *buf) {
  // Formato básico: ID - [Datos en hex]
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
  // Formato detallado con tipo de mensaje e interpretación
  Serial.print(F("ID: 0x"));
  Serial.print(canId, HEX);
  Serial.print(F(" Len:"));
  Serial.print(len);
  
  // Interpretar tipo de mensaje basado en ID
  Serial.print(F(" Tipo:"));
  if (canId < 0x100) {
    Serial.print(F("Motor"));
  } else if (canId < 0x200) {
    Serial.print(F("Trans"));
  } else if (canId < 0x400) {
    Serial.print(F("Chasis"));
  } else if (canId < 0x600) {
    Serial.print(F("Body"));
  } else if (canId < 0x7E0) {
    Serial.print(F("Info"));
  } else if (canId >= 0x7E0 && canId <= 0x7EF) {
    Serial.print(F("DIAG"));
  } else {
    Serial.print(F("Otro"));
  }
  
  // Mostrar datos
  Serial.print(F(" ["));
  for (int i = 0; i < len; i++) {
    if (buf[i] < 0x10) Serial.print(F("0"));
    Serial.print(buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.print(F("] ASCII:"));
  
  // Mostrar como ASCII si es posible
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
  // Procesar respuestas a solicitudes de diagnóstico
  if (canId >= 0x7E8 && canId <= 0x7EF) {  // Respuestas diagnósticas
    Serial.print(F("RESP DIAG ["));
    Serial.print(canId - 0x7E0, HEX);
    Serial.print(F("]: "));
    
    if (len > 2 && buf[0] > 0) {
      byte serviceId = buf[1];
      if (serviceId == 0x41) {  // Respuesta a modo 01 (datos actuales)
        Serial.print(F("Modo 01 PID "));
        Serial.print(buf[2], HEX);
        
        // Interpretar algunos PIDs comunes
        InterpretCommonPIDs(buf[2], &buf[3], len-3);
      } else if (serviceId == 0x42) {  // Respuesta a modo 02 (datos congelados)
        Serial.println(F("Datos congelados"));
      } else {
        Serial.print(F("Modo "));
        Serial.print(serviceId - 0x40, HEX);
      }
    }
    Serial.println();
  }
}

void InterpretCommonPIDs(byte pid, byte *data, byte len) {
  // Interpretación simplificada de algunos PIDs comunes
  switch (pid) {
    case 0x0C:  // RPM
      if (len >= 2) {
        int rpm = ((data[0] * 256) + data[1]) / 4;
        Serial.print(F(" = RPM: "));
        Serial.print(rpm);
      }
      break;
    case 0x0D:  // Velocidad
      if (len >= 1) {
        Serial.print(F(" = Velocidad: "));
        Serial.print(data[0]);
        Serial.print(F(" km/h"));
      }
      break;
    case 0x05:  // Temp. refrigerante
      if (len >= 1) {
        Serial.print(F(" = Temp: "));
        Serial.print(data[0] - 40);
        Serial.print(F("°C"));
      }
      break;
    default:
      // Para otros PIDs solo mostrar los datos en hex
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
  
  // Primera línea según modo
  lcd.setCursor(0, 0);
  switch (operationMode) {
    case 0:
      lcd.print("Monitor basico");
      break;
    case 1:
      lcd.print("Monitor detalle");
      break;
    case 2:
      lcd.print("Estadisticas");
      break;
    case 3:
      lcd.print("Diagnostico");
      break;
  }
  
  // Segunda línea: información relevante según el modo
  lcd.setCursor(0, 1);
  switch (operationMode) {
    case 0:
    case 1:
      // Mostrar mensajes por segundo
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
      
      // Mostrar si la resistencia terminadora está activa
      lcd.setCursor(13, 1);
      lcd.print(termResistorEnabled ? "TRM" : "   ");
      break;
    
    case 2:
      // En modo estadísticas mostrar distribución por tipo
      unsigned long total = 0;
      byte maxIndex = 0;
      
      // Encontrar el tipo de mensaje más frecuente
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
        lcd.print("Sin mensajes");
      }
      break;
      
    case 3:
      // En modo diagnóstico enviar periódicamente solicitudes
      static byte currentPID = 0;
      
      // Mostrar PID actual
      lcd.print("PID:");
      if (currentPID < 0x10) lcd.print("0");
      lcd.print(currentPID, HEX);
      
      // Enviar solicitud de diagnóstico para este PID
      SendDiagnosticRequest(0x01, currentPID);  // Modo 01, PID actual
      
      // Rotar al siguiente PID cada cierto tiempo
      currentPID = (currentPID + 1) % 32;  // Limitar a primeros 32 PIDs
      break;
  }
}

void SendDiagnosticRequest(byte mode, byte pid) {
  byte data[8] = {0x02, mode, pid, 0, 0, 0, 0, 0};
  byte result = CAN.sendMsgBuf(0x7DF, 0, 8, data);  // ID estándar de diagnóstico
  
  if (result != CAN_OK) {
    Serial.println(F("Error enviando solicitud diagnóstico"));
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
      // Cambiar al siguiente modo
      operationMode = (operationMode + 1) % 4;
      
      // Actualizar pantalla inmediatamente
      UpdateDisplay();
      
      // Indicación visual
      digitalWrite(PIN_LED_RX, HIGH);
      delay(100);
      digitalWrite(PIN_LED_RX, LOW);
      
      // Mostrar el nuevo modo en serial
      Serial.print(F("Modo cambiado a: "));
      
      switch (operationMode) {
        case 0:
          Serial.println(F("Monitor Básico"));
          break;
        case 1:
          Serial.println(F("Monitor Detallado"));
          break;
        case 2:
          Serial.println(F("Estadísticas"));
          break;
        case 3:
          Serial.println(F("Diagnóstico"));
          break;
      }
    }
  }
  
  lastButtonState = buttonState;
}

void CheckTerminationSwitch() {
  // Leer el estado del switch de resistencia terminadora
  bool currentState = (digitalRead(PIN_SWITCH_TERM) == LOW);
  
  // Si cambió, actualizar
  if (currentState != termResistorEnabled) {
    termResistorEnabled = currentState;
    UpdateTerminationResistor();
  }
}

void UpdateTerminationResistor() {
  // Aquí se activaría/desactivaría una resistencia terminadora física 
  // mediante un relay o transistor si el hardware lo soporta
  
  Serial.print(F("Resistencia terminadora: "));
  Serial.println(termResistorEnabled ? F("ACTIVADA") : F("DESACTIVADA"));
}

void SignalError() {
  digitalWrite(PIN_LED_ERROR, HIGH);
  delay(50);
  digitalWrite(PIN_LED_ERROR, LOW);
}

void PrintInstructions() {
  Serial.println();
  Serial.println(F("=== LECTOR CAN BUS - INSTRUCCIONES ==="));
  Serial.println(F("Botón 1: Cambiar modo (Monitor/Estadísticas/Diagnóstico)"));
  Serial.println(F("Switch 1: Activar/desactivar resistencia terminadora"));
  Serial.println();
  Serial.println(F("Modos disponibles:"));
  Serial.println(F("0 - Monitor Básico: Muestra ID y datos en formato simple"));
  Serial.println(F("1 - Monitor Detallado: Muestra información completa de mensajes"));
  Serial.println(F("2 - Estadísticas: Muestra distribución de tipos de mensajes"));
  Serial.println(F("3 - Diagnóstico: Intenta comunicarse con módulos usando protocolo OBD2"));
  Serial.println();
  Serial.println(F("LED RX: Parpadea al recibir mensajes"));
  Serial.println(F("LED ERROR: Indica problemas en la inicialización o comunicación"));
  Serial.println(F("======================================"));
}