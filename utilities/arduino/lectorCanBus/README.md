# Lector CAN Bus para Diagnóstico de Emergencia

> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

## Introducción

Esta herramienta es un lector CAN bus de emergencia diseñado para situaciones en las que los escáneres convencionales no logran establecer comunicación con el vehículo. Utilizando un Arduino y componentes accesibles, permite verificar la actividad básica en la red CAN, comprobar las resistencias terminadoras y monitorizar los mensajes del sistema.

## ¿Cuándo utilizar esta herramienta?

- Cuando el escáner de diagnóstico no logra comunicar con el vehículo
- Para verificar si hay actividad en la red CAN bus
- Para comprobar la presencia y el valor de las resistencias terminadoras
- Para identificar posibles problemas de comunicación entre módulos

## Componentes Necesarios

### Hardware

| Componente | Descripción | Cantidad |
|------------|-------------|----------|
| Arduino Uno/Nano/Mega | Placa de microcontrolador | 1 |
| Módulo CAN MCP2515 | Controlador CAN-SPI | 1 |
| Pantalla LCD 16x2 I2C | Para visualizar información (opcional) | 1 |
| LEDs | Para indicación visual: RX (verde), ERROR (rojo) | 2 |
| Pulsador | Para cambiar entre modos de operación | 1 |
| Interruptor | Para activar/desactivar resistencia terminadora | 1 |
| Resistencia 120Ω | Resistencia terminadora | 1 |
| Relay | Para conectar/desconectar la resistencia terminadora (opcional) | 1 |
| Caja protectora | Para montar el circuito | 1 |
| Cables Dupont | Para conexiones | Varios |
| Conector OBD-II | Para conectar al vehículo | 1 |

### Bibliotecas de Arduino Requeridas

- **SPI.h** - Incluida en Arduino IDE
- **mcp_can.h** - [Descargar desde GitHub](https://github.com/coryjfowler/MCP_CAN_lib)
- **LiquidCrystal_I2C.h** - Disponible en el Gestor de Bibliotecas Arduino IDE

## Diagrama de Conexiones

### Conexión Arduino - MCP2515

| Arduino | MCP2515 |
|---------|---------|
| 13 (SCK) | SCK |
| 12 (MISO) | SO |
| 11 (MOSI) | SI |
| 10 (SS) | CS |
| 2 | INT |
| 5V | VCC |
| GND | GND |

### Conexión Módulo CAN - OBD-II

| MCP2515 | Conector OBD-II |
|---------|----------------|
| CANH | Pin 6 (CAN-H) |
| CANL | Pin 14 (CAN-L) |
| GND | Pin 5 (GND) |

### Conexión del Circuito de la Resistencia Terminadora Conmutable

```
CANL o------ Interruptor ------ Resistencia 120Ω ------ o CANH
```

### Conexión Arduino - LCD I2C

| Arduino | LCD I2C |
|---------|---------|
| A4 (SDA) | SDA |
| A5 (SCL) | SCL |
| 5V | VCC |
| GND | GND |

## Montaje

1. Conecte el módulo MCP2515 al Arduino según el diagrama
2. Conecte la pantalla LCD al Arduino usando los pines I2C
3. Conecte los LEDs indicadores con resistencias de 220Ω a los pines definidos
4. Instale el botón para cambio de modo y el interruptor para la resistencia terminadora
5. Instale el circuito de resistencia terminadora conmutable
6. Prepare el cable con conector OBD-II para conectar al vehículo

## Instrucciones de Uso

### Instalación del Software

1. Instale el Arduino IDE desde [arduino.cc](https://www.arduino.cc/en/software)
2. Instale las bibliotecas requeridas mediante el Gestor de Bibliotecas o descárgalas manualmente
3. Descargue el código fuente `lectorCanBus.ino` de este repositorio
4. Abra el código en Arduino IDE y cargue el programa a su placa Arduino

### Operación de Diagnóstico

1. **Preparación**:
   - Con el vehículo apagado, conecte el dispositivo al puerto OBD-II
   - El dispositivo se encenderá con la alimentación del conector OBD-II
   - Si no enciende, verifique la alimentación en el conector del vehículo

2. **Verificación Inicial**:
   - La pantalla mostrará "Lector CAN Bus" e intentará inicializar el controlador CAN
   - Si aparece un error de inicialización, verifique las conexiones

3. **Modos de Operación** (cambie con el botón):
   - **Monitor Básico**: Muestra actividad básica del CAN bus
   - **Monitor Detallado**: Muestra información completa de mensajes
   - **Estadísticas**: Muestra distribución de tipos de mensajes
   - **Diagnóstico**: Intenta comunicarse usando el protocolo OBD2 estándar

4. **Verificación de Resistencia Terminadora**:
   - Desactive la resistencia terminadora del dispositivo usando el interruptor
   - Observe si hay cambios en la actividad del bus
   - Si no hay comunicación con resistencia desactivada, es posible que una o ambas resistencias terminadoras del vehículo estén defectuosas
   - Active la resistencia terminadora del dispositivo como prueba

5. **Interpretación de Resultados**:
   - **LED RX parpadeando**: Hay actividad en el bus CAN
   - **LED ERROR encendido**: Problemas en la inicialización o comunicación
   - **Mensajes por segundo** (Msgs/s): Indica el nivel de actividad del bus
   - **Distribución de IDs**: En modo estadísticas, muestra qué tipos de mensajes son más frecuentes

6. **Diagnóstico de Problemas Comunes**:
   - **No hay actividad (0 msgs/s)**: 
     - Verifique conexiones físicas
     - Asegúrese de que el vehículo está en contacto ("key on")
     - Pruebe diferentes velocidades de CAN si es necesario
   - **Mensajes pero sin respuesta diagnóstica**: 
     - Gateway puede estar bloqueando la comunicación
     - La ECU podría requerir un protocolo específico
   - **Actividad inconsistente o errores**: 
     - Posible ECU defectuosa saturando el bus
     - Problemas de cableado o interferencias

## Limitaciones

- Esta herramienta es para diagnóstico básico y no reemplaza un escáner profesional
- Solo soporta CAN estándar (no FlexRay, LIN, MOST, etc.)
- La velocidad predeterminada es de 500 kbps (común en sistemas OBD), pero algunos vehículos usan otras velocidades
- No puede realizar funciones avanzadas como programación o adaptaciones

## Personalización

El código puede ser modificado para:
- Soportar diferentes velocidades de CAN bus
- Añadir más filtros para mensajes específicos
- Interpretar datos específicos de ciertos fabricantes
- Añadir funciones de registro de datos

## Consejos Avanzados

- Para determinar la velocidad correcta del CAN bus de un vehículo, pruebe con 250 kbps, 500 kbps y 125 kbps, que son las más comunes
- Si ve mensajes pero no hay respuesta a solicitudes diagnósticas, puede que necesite emular un protocolo específico del fabricante
- En algunos vehículos, es necesario enviar un mensaje de "despertar" para activar la comunicación con ciertos módulos
- Para problemas de cableado CAN intermitentes, la función de estadística puede ayudar a identificar pérdidas momentáneas de comunicación

## Precaución

> **IMPORTANTE**: Esta herramienta es solo para diagnóstico y no permite modificar parámetros del vehículo. Su uso es bajo su propia responsabilidad. No desconecte ni conecte el dispositivo con el motor en marcha para evitar picos de voltaje.

## Véase También

- [Diagnóstico de problemas en sistema CAN Bus](../../../docs/diagnostico/diagnostico-canbus.md)
- [Sistemas de Comunicación](../../../docs/sensores-y-actuadores/sistemas-comunicacion.md)

[Volver al índice](../README.md)