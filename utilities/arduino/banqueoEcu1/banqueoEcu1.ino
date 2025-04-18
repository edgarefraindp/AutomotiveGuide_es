/**
 * banqueoEcu1.ino - Sistema de Banqueo para ECU Automotriz
 * 
 * Este programa emula las señales de sensores de motor y monitorea las
 * respuestas de la ECU (unidad de control electrónico) para diagnóstico.
 * 
 * Adaptado de la versión ESP32 para plataforma Arduino.
 */

#include <Arduino.h>

// Definiciones de pines para emulación de sensores
#define PIN_EMU_CKP        2     // Emulación sensor CKP (cigüeñal)
#define PIN_EMU_CMP        3     // Emulación sensor CMP (árbol de levas)
#define PIN_EMU_MAP        4     // Emulación sensor MAP (presión múltiple)
#define PIN_EMU_TPS        5     // Emulación sensor TPS (posición acelerador)
#define PIN_EMU_MAF        6     // Emulación sensor MAF (flujo aire)
#define PIN_EMU_ECT        7     // Emulación sensor ECT (temp. refrigerante)
#define PIN_EMU_IAT        8     // Emulación sensor IAT (temp. aire)

// Definiciones de pines para monitoreo de actuadores
#define PIN_MON_INJ1       A0    // Monitoreo inyector 1
#define PIN_MON_INJ2       A1    // Monitoreo inyector 2
#define PIN_MON_INJ3       A2    // Monitoreo inyector 3
#define PIN_MON_INJ4       A3    // Monitoreo inyector 4
#define PIN_MON_COIL       A4    // Monitoreo bobina
#define PIN_CTRL_PUMP      A5    // Control bomba combustible

// Constantes para configuración de PWM
#define PWM_FREQUENCY      490   // Frecuencia en Hz (valor típico Arduino)
#define TPS_MAX_DUTY       255   // Valor máximo de PWM (8 bits en Arduino)

// Constantes para control de motor simulado
#define RPM_MIN            800
#define RPM_MAX            6000
#define RPM_DEFAULT        1000
#define TPS_MIN            0
#define TPS_MAX            100
#define TPS_DEFAULT        15    // 15% TPS = ralentí
#define MAP_MIN            20    // 20 kPa (alta succión)
#define MAP_MAX            101   // 101 kPa (presión atmosférica)
#define MAF_MIN            2     // g/s a ralentí
#define MAF_MAX            150   // g/s a plena carga
#define ECT_MIN            -40   // Temperatura mínima (°C)
#define ECT_MAX            120   // Temperatura máxima (°C)
#define ECT_DEFAULT        85    // Temperatura operación normal (°C)
#define IAT_DEFAULT        25    // Temperatura ambiente predeterminada (°C)

// Constantes para detección de anomalías
#define PULSE_MARGIN       0.15  // 15% margen de error aceptable
#define MIN_VALID_PULSE_US 50    // Pulso mínimo válido (μs)

// Variables para emulación de dientes de CKP (tipo 60-2)
#define TOTAL_TEETH        60
#define MISSING_TEETH      2
byte currentTooth = 0;

// Estructura para almacenar parámetros de motor simulado
typedef struct {
  uint16_t rpm;                // RPM del motor
  byte tps;                    // Posición del acelerador (0-100%)
  byte map;                    // Presión MAP (kPa)
  byte maf;                    // Flujo MAF (g/s)
  int8_t ect;                  // Temperatura refrigerante (°C)
  int8_t iat;                  // Temperatura aire (°C)
  bool engineRunning;          // Estado del motor (encendido/apagado)
} EngineParams;

// Estructura para almacenar parámetros de pulsos de salida
typedef struct {
  uint32_t inj1PulseWidth;     // Ancho de pulso inyector 1 (μs)
  uint32_t inj2PulseWidth;     // Ancho de pulso inyector 2 (μs)
  uint32_t inj3PulseWidth;     // Ancho de pulso inyector 3 (μs)
  uint32_t inj4PulseWidth;     // Ancho de pulso inyector 4 (μs)
  uint32_t coilDwell;          // Tiempo dwell de bobina (μs)
  uint32_t lastUpdateTime;     // Última actualización (ms)
  bool inj1Active;             // Estado del inyector 1
  bool inj2Active;             // Estado del inyector 2
  bool inj3Active;             // Estado del inyector 3
  bool inj4Active;             // Estado del inyector 4
  bool coilActive;             // Estado de la bobina
  bool pumpActive;             // Estado de la bomba
} OutputSignals;

// Enumeración para tipos de eventos de pulso
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

// Estructura para eventos de pulso
typedef struct {
  PulseEventType type;
  uint32_t timestamp;          // Timestamp en microsegundos
} PulseEvent;

// Variables globales
EngineParams engineParams = {
  RPM_DEFAULT,
  TPS_DEFAULT,
  MAP_MAX - 20,               // ~80 kPa en ralentí
  MAF_MIN + 3,                // 5 g/s en ralentí
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

// Buffer para comandos serie
char cmdBuffer[64];
byte cmdIndex = 0;

// Variables para manejar interrupciones y eventos
volatile uint32_t injStartTimes[4] = {0};
volatile uint32_t coilStartTime = 0;
volatile bool injectorState[4] = {false};
volatile bool coilState = false;

/**
 * Calcula el valor PWM de acuerdo al valor del sensor y su rango
 * 
 * @param value Valor actual del sensor
 * @param minValue Valor mínimo del rango del sensor
 * @param maxValue Valor máximo del rango del sensor
 * @return Valor del duty cycle (0-255)
 */
uint8_t CalculatePwmDuty(int value, int minValue, int maxValue) {
  // Limitar el valor al rango
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  
  // Calcular porcentaje (0.0 - 1.0)
  float percentage = (float)(value - minValue) / (float)(maxValue - minValue);
  
  // Calcular duty cycle (0-255)
  return (uint8_t)(percentage * TPS_MAX_DUTY);
}

/**
 * Actualiza la señal del sensor TPS utilizando PWM
 */
void UpdateTpsSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.tps, TPS_MIN, TPS_MAX);
  analogWrite(PIN_EMU_TPS, duty);
}

/**
 * Actualiza la señal del sensor MAP (implementada con PWM)
 */
void UpdateMapSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.map, MAP_MIN, MAP_MAX);
  analogWrite(PIN_EMU_MAP, duty);
}

/**
 * Actualiza la emulación del sensor MAF
 */
void UpdateMafSensor() {
  uint8_t duty = CalculatePwmDuty(engineParams.maf, MAF_MIN, MAF_MAX);
  analogWrite(PIN_EMU_MAF, duty);
}

/**
 * Emula la resistencia del sensor ECT (NTC) usando PWM
 * En un sistema real se usaría un potenciómetro digital
 */
void UpdateEctSensor() {
  // Para sensores de temperatura usamos una simplificación con PWM
  float resistance = 0;
  
  // Cálculo simplificado de resistencia NTC
  if (engineParams.ect > -40 && engineParams.ect < 150) {
    // Fórmula simplificada: R = R0 * e^(B*(1/T - 1/T0))
    // Donde R0 = 2.5kOhm a 25°C, B = 3380
    float t1 = 1.0f / (273.15f + (float)engineParams.ect);
    float t0 = 1.0f / (273.15f + 25.0f);  // Temperatura de referencia (25°C)
    resistance = 2500.0f * exp(3380.0f * (t1 - t0));
    
    // Convertir a duty cycle (inversamente proporcional)
    float maxResistance = 20000.0f; // Resistencia máxima esperada
    float dutyPercent = 1.0f - (resistance / maxResistance);
    if (dutyPercent < 0) dutyPercent = 0;
    if (dutyPercent > 1) dutyPercent = 1;
    
    analogWrite(PIN_EMU_ECT, (uint8_t)(dutyPercent * TPS_MAX_DUTY));
  }
  
  Serial.print("ECT: ");
  Serial.print(engineParams.ect);
  Serial.print("°C, Resistencia: ");
  Serial.print(resistance);
  Serial.println(" Ohm");
}

/**
 * Emula la resistencia del sensor IAT (NTC) usando PWM
 */
void UpdateIatSensor() {
  // Implementación similar a ECT
  float resistance = 0;
  
  if (engineParams.iat > -40 && engineParams.iat < 150) {
    float t1 = 1.0f / (273.15f + (float)engineParams.iat);
    float t0 = 1.0f / (273.15f + 25.0f);
    resistance = 2500.0f * exp(3380.0f * (t1 - t0));
    
    // Convertir a duty cycle (inversamente proporcional)
    float maxResistance = 20000.0f;
    float dutyPercent = 1.0f - (resistance / maxResistance);
    if (dutyPercent < 0) dutyPercent = 0;
    if (dutyPercent > 1) dutyPercent = 1;
    
    analogWrite(PIN_EMU_IAT, (uint8_t)(dutyPercent * TPS_MAX_DUTY));
  }
  
  Serial.print("IAT: ");
  Serial.print(engineParams.iat);
  Serial.print("°C, Resistencia: ");
  Serial.print(resistance);
  Serial.println(" Ohm");
}

/**
 * Genera las señales CKP y CMP basadas en las RPM actuales
 * Esta función debe llamarse periódicamente en el loop principal
 */
void UpdateCkpCmpSignals() {
  // Calcular intervalo entre dientes basado en RPM
  // RPM = 60 / (tiempo por revolución en segundos)
  // Para motor de 4 tiempos, una revolución = 2 ciclos de leva
  // Tiempo por diente = (60 / RPM) / número_de_dientes_efectivos
  uint32_t currentTime = micros();
  uint32_t teethInterval = (uint32_t)((60000000UL / engineParams.rpm) / (TOTAL_TEETH - MISSING_TEETH));
  
  // Verificar si es tiempo de generar el siguiente diente
  if (currentTime - lastCkpUpdateTime >= teethInterval) {
    lastCkpUpdateTime = currentTime;
    
    // Incrementar posición de diente
    currentTooth = (currentTooth + 1) % TOTAL_TEETH;
    
    // Verificar si estamos en la posición de dientes faltantes (58-59 en un 60-2)
    bool generatePulse = true;
    if (currentTooth >= (TOTAL_TEETH - MISSING_TEETH)) {
      generatePulse = false;
    }
    
    // Generar pulso CKP
    if (engineParams.engineRunning) {
      digitalWrite(PIN_EMU_CKP, generatePulse ? HIGH : LOW);
      
      // En determinado diente, generar también pulso CMP (por ejemplo, en el diente 10)
      if (currentTooth == 10) {
        digitalWrite(PIN_EMU_CMP, HIGH);
      } else if (currentTooth == 11) {
        digitalWrite(PIN_EMU_CMP, LOW);
      }
    }
  }
}

/**
 * Calcula los valores de sensores en base a relaciones físicas
 * Por ejemplo, a mayor TPS (acelerador), menor MAP (presión de succión)
 */
void CalculateSensorRelationships() {
  // Correlacionar TPS con MAP (a mayor apertura de acelerador, mayor presión en múltiple)
  if (engineParams.tps < 10) {
    // Ralentí - alta succión
    engineParams.map = MAP_MIN + 15;
  } else if (engineParams.tps > 90) {
    // Acelerador casi a fondo - presión cercana a la atmosférica
    engineParams.map = MAP_MAX - 5;
  } else {
    // Interpolación lineal entre mínimo y máximo
    engineParams.map = MAP_MIN + ((engineParams.tps * (MAP_MAX - MAP_MIN)) / 100);
  }
  
  // Correlacionar TPS con MAF (a mayor apertura, mayor flujo de aire)
  if (engineParams.tps < 10) {
    // Flujo mínimo a ralentí
    engineParams.maf = MAF_MIN + 3;
  } else {
    // Interpolación no lineal (el flujo crece exponencialmente con TPS)
    float tpsPercentage = (float)engineParams.tps / 100.0f;
    float mafRange = MAF_MAX - MAF_MIN;
    engineParams.maf = MAF_MIN + (uint8_t)(mafRange * tpsPercentage * tpsPercentage);
  }
}

/**
 * Actualiza todos los valores de los sensores emulados
 */
void UpdateAllSensors() {
  UpdateTpsSensor();
  UpdateMapSensor();
  UpdateMafSensor();
  UpdateEctSensor();
  UpdateIatSensor();
  
  Serial.print("Sensores actualizados - RPM: ");
  Serial.print(engineParams.rpm);
  Serial.print(", TPS: ");
  Serial.print(engineParams.tps);
  Serial.print("%, MAP: ");
  Serial.print(engineParams.map);
  Serial.println("kPa");
}

/**
 * Procesa los eventos de interrupción para los pines de monitoreo
 * 
 * @param pin Número de pin que generó la interrupción
 * @param state Estado actual del pin (HIGH/LOW)
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
            Serial.print("Inyector 1: ");
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
            Serial.print("Inyector 2: ");
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
            Serial.print("Inyector 3: ");
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
            Serial.print("Inyector 4: ");
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
            Serial.print("Dwell Bobina: ");
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
 * ISRs (Interrupt Service Routines) para los pines de monitoreo
 * Arduino UNO/NANO solo soporta interrupciones en pines específicos
 * Arduino MEGA/DUE soporta más pines con interrupciones
 */

// ISR para el pin de inyector 1
void Inj1ISR() {
  bool state = digitalRead(PIN_MON_INJ1);
  injectorState[0] = state;
}

// ISR para el pin de inyector 2
void Inj2ISR() {
  bool state = digitalRead(PIN_MON_INJ2);
  injectorState[1] = state;
}

// ISR para el pin de inyector 3
void Inj3ISR() {
  bool state = digitalRead(PIN_MON_INJ3);
  injectorState[2] = state;
}

// ISR para el pin de inyector 4
void Inj4ISR() {
  bool state = digitalRead(PIN_MON_INJ4);
  injectorState[3] = state;
}

// ISR para el pin de bobina
void CoilISR() {
  bool state = digitalRead(PIN_MON_COIL);
  coilState = state;
}

/**
 * Analiza las salidas de la ECU y detecta anomalías
 */
void DetectAnomalies() {
  uint32_t currentTime = millis();
  
  // Solo verificar cada segundo para no saturar la consola
  if (currentTime - lastAnomalyCheckTime < 1000) {
    return;
  }
  
  lastAnomalyCheckTime = currentTime;
  
  // 1. Verificar si hay pulsos faltantes en los inyectores
  if (engineParams.engineRunning && engineParams.rpm > 0) {
    if (outputSignals.inj1PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALÍA: No hay pulsos en Inyector 1");
    }
    
    if (outputSignals.inj2PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALÍA: No hay pulsos en Inyector 2");
    }
    
    if (outputSignals.inj3PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALÍA: No hay pulsos en Inyector 3");
    }
    
    if (outputSignals.inj4PulseWidth == 0 && currentTime > 2000) {
      Serial.println("ANOMALÍA: No hay pulsos en Inyector 4");
    }
  }
  
  // 2. Verificar equilibrio entre inyectores (no deberían tener más de 15% de diferencia)
  if (outputSignals.inj1PulseWidth > 0 && outputSignals.inj2PulseWidth > 0) {
    float ratio = (float)outputSignals.inj1PulseWidth / (float)outputSignals.inj2PulseWidth;
    if (ratio < (1.0 - PULSE_MARGIN) || ratio > (1.0 + PULSE_MARGIN)) {
      Serial.print("ANOMALÍA: Desequilibrio entre inyector 1 (");
      Serial.print(outputSignals.inj1PulseWidth);
      Serial.print(" μs) e inyector 2 (");
      Serial.print(outputSignals.inj2PulseWidth);
      Serial.println(" μs)");
    }
  }
  
  // 3. Verificar que los tiempos de inyección aumenten con TPS alto
  if (engineParams.tps > 50 && outputSignals.inj1PulseWidth < 3000) {
    Serial.print("ANOMALÍA: TPS alto (");
    Serial.print(engineParams.tps);
    Serial.print("%) pero tiempo de inyección bajo (");
    Serial.print(outputSignals.inj1PulseWidth);
    Serial.println(" μs)");
  }
  
  // 4. Verificar la correlación entre RPM y dwell de bobina
  // A mayor RPM, menor tiempo de dwell
  if (engineParams.rpm > 3000 && outputSignals.coilDwell > 4000) {
    Serial.print("ANOMALÍA: RPM alto (");
    Serial.print(engineParams.rpm);
    Serial.print(") pero dwell de bobina muy largo (");
    Serial.print(outputSignals.coilDwell);
    Serial.println(" μs)");
  }
}

/**
 * Procesa comandos recibidos por el puerto serie
 * 
 * @param cmdBuffer Buffer que contiene el comando recibido
 * @param length Longitud del comando
 * @return true si el comando fue reconocido y procesado
 */
bool ProcessCommand(const char* cmdBuffer, size_t length) {
  if (strncmp(cmdBuffer, "rpm:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= RPM_MIN && value <= RPM_MAX) {
      engineParams.rpm = value;
      Serial.print("RPM ajustado a: ");
      Serial.println(engineParams.rpm);
      return true;
    }
  } else if (strncmp(cmdBuffer, "tps:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= TPS_MIN && value <= TPS_MAX) {
      engineParams.tps = value;
      Serial.print("TPS ajustado a: ");
      Serial.print(engineParams.tps);
      Serial.println("%");
      return true;
    }
  } else if (strncmp(cmdBuffer, "ect:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= ECT_MIN && value <= ECT_MAX) {
      engineParams.ect = value;
      Serial.print("ECT ajustado a: ");
      Serial.print(engineParams.ect);
      Serial.println("°C");
      return true;
    }
  } else if (strncmp(cmdBuffer, "iat:", 4) == 0 && length > 4) {
    int value = atoi(cmdBuffer + 4);
    if (value >= ECT_MIN && value <= ECT_MAX) { // Mismo rango que ECT
      engineParams.iat = value;
      Serial.print("IAT ajustado a: ");
      Serial.print(engineParams.iat);
      Serial.println("°C");
      return true;
    }
  } else if (strcmp(cmdBuffer, "status") == 0) {
    Serial.println("=== Estado del Sistema ===");
    Serial.print("RPM: "); Serial.println(engineParams.rpm);
    Serial.print("TPS: "); Serial.print(engineParams.tps); Serial.println("%");
    Serial.print("MAP: "); Serial.print(engineParams.map); Serial.println(" kPa");
    Serial.print("MAF: "); Serial.print(engineParams.maf); Serial.println(" g/s");
    Serial.print("ECT: "); Serial.print(engineParams.ect); Serial.println("°C");
    Serial.print("IAT: "); Serial.print(engineParams.iat); Serial.println("°C");
    Serial.print("Motor: "); Serial.println(engineParams.engineRunning ? "Encendido" : "Apagado");
    Serial.print("Inyector 1: "); Serial.print(outputSignals.inj1PulseWidth); Serial.println(" μs");
    Serial.print("Inyector 2: "); Serial.print(outputSignals.inj2PulseWidth); Serial.println(" μs");
    Serial.print("Inyector 3: "); Serial.print(outputSignals.inj3PulseWidth); Serial.println(" μs");
    Serial.print("Inyector 4: "); Serial.print(outputSignals.inj4PulseWidth); Serial.println(" μs");
    Serial.print("Dwell Bobina: "); Serial.print(outputSignals.coilDwell); Serial.println(" μs");
    return true;
  } else if (strcmp(cmdBuffer, "start") == 0) {
    engineParams.engineRunning = true;
    Serial.println("Motor arrancado");
    return true;
  } else if (strcmp(cmdBuffer, "stop") == 0) {
    engineParams.engineRunning = false;
    Serial.println("Motor detenido");
    return true;
  } else if (strcmp(cmdBuffer, "help") == 0) {
    Serial.println("Comandos disponibles:");
    Serial.println("  rpm:VALOR     - Ajustar RPM (800-6000)");
    Serial.println("  tps:VALOR     - Ajustar posición acelerador (0-100%)");
    Serial.println("  ect:VALOR     - Ajustar temperatura refrigerante (-40 a 120°C)");
    Serial.println("  iat:VALOR     - Ajustar temperatura aire (-40 a 120°C)");
    Serial.println("  start         - Arrancar motor (iniciar señales)");
    Serial.println("  stop          - Detener motor (parar señales)");
    Serial.println("  status        - Mostrar estado actual");
    Serial.println("  help          - Mostrar esta ayuda");
    return true;
  }
  
  return false;
}

/**
 * Verifica si hay comandos disponibles en el puerto serie
 */
void CheckSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    // Eco local del carácter
    Serial.write(c);
    
    if (c == '\n' || c == '\r') {
      // Fin de comando
      Serial.println();
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        if (!ProcessCommand(cmdBuffer, cmdIndex)) {
          Serial.print("Comando no reconocido: ");
          Serial.println(cmdBuffer);
        }
        cmdIndex = 0;
      }
    } else if (cmdIndex < sizeof(cmdBuffer) - 1) {
      // Agregar carácter al buffer
      cmdBuffer[cmdIndex++] = c;
    }
  }
}

/**
 * Verifica y procesa los estados de los pines de monitoreo
 * Esto se hace en el loop principal para manejar las interrupciones
 */
void ProcessPinStates() {
  static bool lastInjState[4] = {false};
  static bool lastCoilState = false;
  
  // Verificar cambios en inyectores
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
  
  // Verificar cambios en bobina
  if (coilState != lastCoilState) {
    lastCoilState = coilState;
    ProcessPinInterrupt(PIN_MON_COIL, coilState);
  }
}

void setup() {
  // Inicializar el puerto serie
  Serial.begin(115200);
  while (!Serial) {
    ; // Esperar a que se conecte el puerto serie (solo necesario para placas con USB nativo)
  }
  
  Serial.println("\n=== Banqueo ECU - Interfaz de Control ===");
  Serial.println("Escriba 'help' para ver los comandos disponibles");
  
  // Configurar pines para emulación de sensores
  pinMode(PIN_EMU_CKP, OUTPUT);
  pinMode(PIN_EMU_CMP, OUTPUT);
  pinMode(PIN_EMU_MAP, OUTPUT);
  pinMode(PIN_EMU_TPS, OUTPUT);
  pinMode(PIN_EMU_MAF, OUTPUT);
  pinMode(PIN_EMU_ECT, OUTPUT);
  pinMode(PIN_EMU_IAT, OUTPUT);
  
  // Configurar pines para monitoreo con interrupciones
  pinMode(PIN_MON_INJ1, INPUT_PULLUP);
  pinMode(PIN_MON_INJ2, INPUT_PULLUP);
  pinMode(PIN_MON_INJ3, INPUT_PULLUP);
  pinMode(PIN_MON_INJ4, INPUT_PULLUP);
  pinMode(PIN_MON_COIL, INPUT_PULLUP);
  pinMode(PIN_CTRL_PUMP, INPUT_PULLUP);
  
  // Adjuntar interrupciones (si están disponibles en la placa Arduino)
  // Nota: en Arduino UNO/Nano solo los pines 2 y 3 soportan interrupciones
  // En una implementación real, ajustar según la placa utilizada
  attachInterrupt(digitalPinToInterrupt(PIN_MON_INJ1), Inj1ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_MON_INJ2), Inj2ISR, CHANGE);
  
  // Inicializar el estado del sistema
  digitalWrite(PIN_EMU_CKP, LOW);
  digitalWrite(PIN_EMU_CMP, LOW);
  
  // Inicializar timestamps
  lastCkpUpdateTime = micros();
  lastSensorUpdateTime = millis();
  lastCommandCheckTime = millis();
  lastAnomalyCheckTime = millis();
  
  // Actualizar sensores iniciales
  UpdateAllSensors();
}

void loop() {
  uint32_t currentMillis = millis();
  
  // Actualizar señales CKP/CMP en cada ciclo
  UpdateCkpCmpSignals();
  
  // Procesar estados de pines de monitoreo
  ProcessPinStates();
  
  // Verificar comandos serie cada 20ms
  if (currentMillis - lastCommandCheckTime >= 20) {
    lastCommandCheckTime = currentMillis;
    CheckSerialCommands();
  }
  
  // Actualizar sensores cada 500ms
  if (currentMillis - lastSensorUpdateTime >= 500) {
    lastSensorUpdateTime = currentMillis;
    
    // Calcular relaciones físicas entre sensores
    CalculateSensorRelationships();
    
    // Actualizar todos los sensores
    UpdateAllSensors();
    
    // Verificar anomalías
    DetectAnomalies();
  }
}