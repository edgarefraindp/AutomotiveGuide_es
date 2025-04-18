# Banqueo ECU (Versión Arduino)

## Descripción General

Este proyecto implementa un sistema de "banqueo" para pruebas de ECU (Unidad de Control Electrónico) automotriz utilizando una placa Arduino. El sistema permite emular las señales de entrada que espera recibir una ECU y monitorear las señales que ésta genera como respuesta, facilitando el diagnóstico de fallas en estas unidades fuera del vehículo, en un banco de trabajo.

## ¿Qué hace?

El sistema Arduino de banqueo de ECU permite:

- Emular sensores críticos del motor (CKP, CMP, TPS, MAP, MAF, ECT, IAT)
- Monitorear las señales de salida de la ECU (inyectores, bobinas)
- Detectar anomalías en los tiempos de inyección y encendido
- Proporcionar una interfaz de control por puerto serie para modificar parámetros
- Analizar el comportamiento de la ECU bajo diferentes condiciones simuladas

## Comparación con la versión ESP32

Esta es una adaptación del proyecto original basado en ESP32, con las siguientes diferencias:

- **Arquitectura de programación**: Reemplazo del sistema multitarea (FreeRTOS) por programación no bloqueante basada en `millis()` y `micros()`
- **Capacidad de pines**: Limitación a los pines disponibles en Arduino (especialmente para interrupciones)
- **Resolución PWM**: Menor resolución de PWM (8 bits en Arduino vs. 13 bits en ESP32)
- **Velocidad de procesamiento**: Menor frecuencia de CPU, lo que puede afectar la precisión en señales de alta frecuencia
- **Memoria disponible**: Restricciones de memoria RAM y Flash que limitan la complejidad del código

## Requisitos de Hardware

- Placa Arduino (recomendado Arduino MEGA o DUE para mayor número de interrupciones)
- Cables y conectores para la ECU específica
- Fuente de alimentación de 12V para la ECU
- Resistencias y componentes electrónicos básicos para acondicionamiento de señal
- Opcional: Adaptadores de nivel lógico para convertir señales de 5V a 12V cuando sea necesario

## Conexiones de Arduino

| Función | Pin Arduino | Descripción |
|---------|-------------|-------------|
| Emulación CKP | 2 | Señal del sensor de posición del cigüeñal |
| Emulación CMP | 3 | Señal del sensor de posición del árbol de levas |
| Emulación MAP | 4 | Señal del sensor de presión absoluta del múltiple |
| Emulación TPS | 5 | Señal del sensor de posición del acelerador |
| Emulación MAF | 6 | Señal del sensor de flujo de masa de aire |
| Emulación ECT | 7 | Señal del sensor de temperatura del refrigerante |
| Emulación IAT | 8 | Señal del sensor de temperatura del aire |
| Monitor Inyector 1 | A0 | Monitoreo del pulso del inyector 1 |
| Monitor Inyector 2 | A1 | Monitoreo del pulso del inyector 2 |
| Monitor Inyector 3 | A2 | Monitoreo del pulso del inyector 3 |
| Monitor Inyector 4 | A3 | Monitoreo del pulso del inyector 4 |
| Monitor Bobina | A4 | Monitoreo del pulso de la bobina de ignición |
| Control Bomba | A5 | Control de la bomba de combustible |

## Cómo Funciona

El sistema utiliza salidas digitales y PWM para generar señales que emulan sensores vehiculares:

1. **Señales CKP/CMP**: Generación de pulsos digitales que simulan la rotación del motor con el patrón 60-2 común en muchos vehículos
2. **Señales analógicas**: Uso de PWM para emular sensores como TPS, MAP, MAF, ECT e IAT
3. **Monitoreo de actuadores**: Captura y medición de los pulsos generados por la ECU para activar inyectores y bobinas
4. **Detección de anomalías**: Análisis automático de patrones de pulsos y búsqueda de comportamientos inesperados

## En Palabras Sencillas

El Arduino actúa como un "simulador de motor" que engaña a la computadora del vehículo haciéndole creer que está conectada a un motor real. Le enviamos señales falsas que imitan a los sensores del vehículo y observamos cómo responde la computadora. Si la computadora responde incorrectamente (por ejemplo, no activa un inyector cuando debería), podemos determinar que hay un problema en esa parte de la ECU.

## Instalación y Uso

1. **Configuración inicial**:
   - Cargar el código en la placa Arduino
   - Conectar los pines de salida de Arduino a las entradas correspondientes de la ECU
   - Conectar los pines de monitoreo de Arduino a las salidas correspondientes de la ECU
   - Alimentar la ECU con 12V DC
   - Abrir el Monitor Serial a 115200 baudios

2. **Comandos disponibles**:
   - `rpm:VALOR` - Ajustar RPM simuladas (800-6000)
   - `tps:VALOR` - Ajustar posición del acelerador (0-100%)
   - `ect:VALOR` - Ajustar temperatura del refrigerante (-40 a 120°C)
   - `iat:VALOR` - Ajustar temperatura del aire (-40 a 120°C)
   - `start` - Iniciar la simulación del motor
   - `stop` - Detener la simulación del motor
   - `status` - Mostrar el estado actual del sistema
   - `help` - Mostrar lista de comandos disponibles

3. **Interpretación de resultados**:
   - El sistema mostrará los anchos de pulso de los inyectores en microsegundos
   - También indicará el tiempo de activación (dwell) de la bobina
   - Se mostrarán alertas cuando se detecten anomalías en las señales

## Consejo Práctico

Al utilizar este sistema, es recomendable comenzar siempre con valores de simulación estándar (motor en ralentí, temperatura de operación normal) antes de probar condiciones extremas. Esto permite establecer una línea base de comportamiento esperado y facilita la identificación de anomalías cuando se simulan condiciones más exigentes.

## Precauciones ⚠️

- **Niveles de voltaje**: Arduino opera con 5V, pero muchas ECUs utilizan señales de 12V. Use adaptadores de nivel lógico cuando sea necesario.
- **Protección de pines**: Coloque resistencias limitadoras de corriente (330Ω-1kΩ) en los pines de entrada para proteger el Arduino.
- **Alimentación**: La ECU requiere alimentación de 12V estable y con suficiente capacidad de corriente.
- **Conexión a tierra**: Asegúrese de que la ECU y el Arduino compartan la misma referencia de tierra.
- **Polaridad**: Verifique la polaridad correcta antes de conectar cualquier componente.

## Circuitos de Acondicionamiento de Señal

Para conectar correctamente el Arduino a una ECU automotriz, es necesario utilizar circuitos de acondicionamiento de señal que protejan el Arduino y adapten los niveles de voltaje entre ambos sistemas.

### Amplificación de Señales de Salida con TIP120

Las señales de salida del Arduino (5V) generalmente necesitan ser amplificadas a 12V para simular correctamente los sensores del vehículo. El transistor Darlington TIP120 es ideal para esta tarea:

```
Arduino (Pin Digital) ----[Resistencia 1kΩ]----+
                                              |
                                              +----|Base TIP120
                                              |
                                             [Resistencia 10kΩ]
                                              |
                                              +----GND
                                                  |
                                                  |
+12V ----[Resistencia Pull-up 4.7kΩ]----+---Colector TIP120
                                        |
                                        +---- A la ECU
                                              |
                                              |
GND ------------------------------+----Emisor TIP120
                                  |
                                  +---- GND ECU
```

Este circuito permite que la salida de 5V del Arduino controle una señal de 12V que la ECU puede leer correctamente. Para señales como el CKP o CMP, este circuito es esencial para generar pulsos con la amplitud correcta.

### Circuito de Alimentación y Protección con 7805

Para alimentar el circuito y proporcionar protección contra picos de voltaje del sistema eléctrico del vehículo, se recomienda implementar este circuito:

```
+12V Batería ---[Fusible 1A]---+---[Diodo 1N4007]---+---[7805]---+--- +5V Arduino
                               |                    |           |
                         [Diodo Zener 15V]     [Capacitor     [Capacitor
                               |               100μF 25V]     100nF]
                               |                    |           |
GND Batería ------------------+--------------------+-----+-----+--- GND Arduino
                                                         |
                                                         +--- GND Circuito
```

Este circuito proporciona:
- Protección contra inversión de polaridad (diodo 1N4007)
- Protección contra sobretensiones (diodo Zener)
- Filtrado de ruido (capacitores)
- Voltaje estable de 5V para el Arduino (regulador 7805)

### Lista de Componentes Recomendados

Para implementar los circuitos de acondicionamiento, necesitará:

| Componente | Cantidad | Descripción |
|------------|----------|-------------|
| TIP120 | 7 | Transistores Darlington NPN para amplificación de señales |
| 7805 | 1 | Regulador de voltaje de 5V |
| 1N4007 | 2 | Diodos rectificadores para protección |
| Diodo Zener 15V | 1 | Protección contra sobretensiones |
| Resistencias 1kΩ | 7 | Limitadoras de corriente para bases de transistores |
| Resistencias 10kΩ | 7 | Pull-down para estabilizar entradas |
| Resistencias 4.7kΩ | 7 | Pull-up para salidas |
| Capacitor 100μF 25V | 1 | Filtrado de baja frecuencia |
| Capacitor 100nF | 1 | Filtrado de alta frecuencia |
| Fusible 1A | 1 | Protección contra cortocircuitos |
| Portafusible | 1 | Soporte para fusible |
| Placa PCB perforada | 1 | Para montar el circuito |
| Borneras | 4-6 | Conexiones de alimentación y señales |

### Consideraciones para Sensores Específicos

- **CKP/CMP**: Utilice el circuito con TIP120 descrito anteriormente para generar pulsos de 0-12V.
- **TPS/MAP/MAF**: Para estos sensores analógicos, puede usar un divisor de voltaje o un circuito con amplificador operacional para convertir el PWM del Arduino en un voltaje analógico variable.
- **ECT/IAT**: Para emular sensores de temperatura NTC, es recomendable utilizar potenciómetros digitales (como el MCP4131) controlados por SPI desde el Arduino.

## Limitaciones de esta versión Arduino

- **Precisión temporal**: La generación de señales de alta frecuencia (como CKP a RPM altas) puede ser menos precisa que en la versión ESP32.
- **Interrupciones**: Un Arduino UNO/Nano solo tiene 2 pines con capacidad de interrupción (2 y 3), lo que limita la capacidad de monitoreo simultáneo.
- **Resolución PWM**: La resolución de PWM de 8 bits puede ser insuficiente para emular con precisión algunos sensores analógicos.

## Para Principiantes

Si es su primera experiencia con un sistema de banqueo, comience con una ECU sencilla (como sistemas de inyección monopunto) antes de intentar diagnosticar ECUs modernas más complejas. Las ECUs más antiguas tienen lógicas más simples y son más fáciles de diagnosticar con este método.

## Compatibilidad

Este código ha sido diseñado para ser compatible con la mayoría de placas Arduino, pero es recomendable utilizar modelos con mayor capacidad de procesamiento como Arduino MEGA, DUE o placas compatibles basadas en STM32 para obtener mejores resultados, especialmente en la emulación precisa de señales CKP/CMP a altas RPM.