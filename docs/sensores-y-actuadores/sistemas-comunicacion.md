# Sistemas de Comunicación

> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

## Introducción a los Sistemas de Comunicación

# SISTEMAS DE COMUNICACIÓN

Los vehículos modernos incorporan múltiples redes de comunicación que permiten el intercambio de información entre las distintas unidades de control electrónico (ECUs). Estas redes son fundamentales para el funcionamiento integrado de todos los sistemas del vehículo.

## 1. REDES DE COMUNICACIÓN VEHICULAR

### Tipos de Redes en Vehículos Modernos

- **CAN (Controller Area Network)**
  - CAN de alta velocidad (500 kbps - 1 Mbps): Motor, transmisión, ABS, ESP
  - CAN de baja velocidad (125 kbps): Carrocería, confort, iluminación
  - Características: 2 hilos trenzados, comunicación por diferencia de voltaje

- **LIN (Local Interconnect Network)**
  - Velocidad: 20 kbps
  - Aplicaciones: Sistemas sencillos (elevalunas, asientos, espejos)
  - Características: 1 solo hilo, maestro-esclavo, bajo coste

- **FlexRay**
  - Velocidad: hasta 10 Mbps
  - Aplicaciones: Sistemas críticos, por tiempo real (dirección, suspensión activa)
  - Características: Alta fiabilidad, determinista, redundante

- **MOST (Media Oriented Systems Transport)**
  - Velocidad: 25-150 Mbps
  - Aplicaciones: Sistemas multimedia, audio/video de alta definición
  - Características: Fibra óptica o par trenzado, anillo lógico

- **Ethernet Automotriz**
  - Velocidad: 100 Mbps - 1 Gbps
  - Aplicaciones: Sistemas ADAS, cámaras, diagnóstico, actualizaciones OTA
  - Características: Compatible con TCP/IP, alto ancho de banda

### 1.1 Módulo de Control de Carrocería (BCM)

- **Función**: Coordinar y controlar las funciones de carrocería y confort
- **Ubicación**: Generalmente bajo el tablero, cerca de la columna de dirección
- **Sistemas controlados**:
  - Iluminación interior y exterior
  - Limpiaparabrisas y lavaparabrisas
  - Cierre centralizado
  - Elevalunas eléctricos
  - Alarma e inmovilizador
  - Gestión de energía
- **Interfaces de comunicación**:
  - CAN Confort/Carrocería
  - LIN para subsistemas
  - Conexión directa a sensores y actuadores
- **Síntomas de fallo**:
  - Funciones intermitentes de iluminación o cierre
  - Mensajes de error en cuadro de instrumentos
  - Consumo excesivo de batería en reposo
  - Múltiples códigos de fallo en distintos sistemas
- **Diagnóstico**:
  - Lectura de códigos con escáner multimarca o específico
  - Verificación de comunicación en redes
  - Comprobación de alimentación y masa
  - Análisis de señales CAN/LIN con osciloscopio

### 1.2 Gateway de Comunicaciones

- **Función**: Interconectar y traducir mensajes entre diferentes redes del vehículo
- **Ubicación**: 
  - Integrado en BCM
  - Como unidad independiente cerca del conector OBD
  - En algunos casos integrado en el cuadro de instrumentos
- **Características**:
  - Filtrado de mensajes entre redes
  - Priorización de comunicaciones
  - Seguridad y control de acceso
  - Interfaz para diagnóstico externo
- **Redes típicamente interconectadas**:
  - CAN Powertrain (motor/transmisión)
  - CAN Chasis (frenos, suspensión, dirección)
  - CAN Confort/Carrocería
  - CAN Diagnóstico
  - LIN, FlexRay o MOST según equipamiento
  - Ethernet Automotriz
- **Síntomas de fallo**:
  - Comunicación intermitente entre sistemas
  - Imposibilidad de comunicación con ciertas ECUs
  - Mensajes de error de comunicación múltiples
  - Funcionamiento degradado de varios sistemas
- **Diagnóstico**:
  - Análisis de actividad en buses de comunicación
  - Verificación de resistencias terminadoras en CAN
  - Comprobación de integridad de cableado
  - Reinicialización de sistemas (desconexión de batería)

## Procedimientos de Diagnóstico en Redes de Comunicación

### Verificación de Red CAN

1. **Inspección visual**:
   - Comprobar conectores y cableado
   - Verificar ausencia de óxido o corrosión

2. **Medición de resistencia**:
   - Entre pines CAN H y CAN L: 60 ohms (dos resistencias de 120 ohms en paralelo)
   - CAN H a positivo y CAN L a masa: Infinito (sin cortocircuitos)

3. **Análisis con osciloscopio**:
   - CAN H: Nivel de reposo 2.5V, nivel dominante 3.5V
   - CAN L: Nivel de reposo 2.5V, nivel dominante 1.5V
   - Diferencia entre señales: 0V en reposo, 2V en dominante

### Verificación de Red LIN

1. **Inspección visual de cableado y conectores**

2. **Mediciones**:
   - Verificar alimentación (8-18V)
   - Comprobar nivel de señal LIN: 0-1V (dominante), 8-12V (recesivo)
   - Verificar pull-up de 1K-10K ohms a positivo

3. **Análisis de trama LIN con osciloscopio**:
   - Bit de inicio (dominante)
   - Campo de sincronización (0x55)
   - Identificador, datos y checksum

## Actualización de Software

Una característica importante de los vehículos modernos es la capacidad de actualizar el software de las ECUs:

- **Tipos de actualización**:
  - En taller con equipo específico (método tradicional)
  - FOTA (Firmware Over The Air) mediante conexión celular/WiFi
  - Semi-OTA (descarga a dispositivo y transferencia por conexión local)

- **Consideraciones importantes**:
  - Verificar compatibilidad de versiones
  - Asegurar alimentación estable durante el proceso
  - Seguir procedimiento específico del fabricante
  - Recalibración de sistemas tras actualización

[Volver al índice](../sensores-y-actuadores.md)