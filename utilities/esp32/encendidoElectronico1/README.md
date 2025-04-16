# Control de Marcha basado en Sensor MAP para ESP32

## Descripción

Este proyecto implementa un sistema de control de marcha basado en la lectura de un sensor de presión absoluta en el múltiple de admisión (MAP). El sistema monitorea la presión y controla la marcha del motor para evitar daños por tiempos prolongados de activación o por presión inadecuada en el múltiple.

## Características

- Lectura precisa de sensor MAP mediante entrada analógica de 5V
- Control automático de marcha según presión del múltiple
- Control de válvula IAC usando transistor TIP120 para regular el flujo de aire
- Opción para conexión "Y" al cableado original del cuerpo de aceleración
- Control de marcha mediante transistor TIP120 y relevador
- Detención automática por tiempo máximo de operación
- Control remoto vía Bluetooth (apagado/encendido)
- Botón físico de encendido/apagado
- Indicaciones visuales mediante LED
- Advertencias de seguridad para vehículos manuales
- Circuito de alimentación integrado con regulador 7805

## Requisitos de Hardware

- Placa ESP32 DevKit o similar
- Sensor MAP (manómetro con salida analógica compatible con 5V)
- 2x Transistores TIP120 (para control de válvula IAC y relay de marcha)
- Válvula IAC o similar para control de aire
- Relevador para activación de marcha
- Resistencias pull-up/pull-down según especificaciones
- Pulsador para encendido/apagado físico
- Regulador de voltaje 7805
- Diodos 1N4007
- Capacitores (10μF y 100nF)
- Resistencias para polarización de transistores (1K)
- Fuente de alimentación de 12V (sistema eléctrico del vehículo)

## Diagrama de Conexión

```
ESP32 (GPIO34) <----- Sensor MAP (Salida analógica) [ENTRADA ANALÓGICA]

ESP32 (GPIO25) -----> Resistencia 1K ---> Base TIP120 #1 ---> Válvula IAC
                                           |
                                           | Emisor
                                           v
                                          GND

ESP32 (GPIO26) -----> Resistencia 1K ---> Conexión "Y" al cuerpo de aceleración
                                          (Alternativa al control directo de IAC)

ESP32 (GPIO27) -----> Resistencia 1K ---> Base TIP120 #2 ---> Relay de Marcha
                                           |
                                           | Emisor
                                           v
                                          GND

ESP32 (GPIO33) <----- Botón de Encendido (Con resistencia pull-up)

ESP32 (GPIO2)  -----> LED de Estado
```

## Circuito de Alimentación

Para alimentar el ESP32 desde el sistema eléctrico de 12V del vehículo, se utiliza el siguiente circuito:

```
12V (Batería) ---+--- Diodo 1N4007 ---> Regulador 7805 ---> 3.3V (ESP32)
                 |                        |     |
                 |                        |     |
         Capacitor 10μF                   |  Capacitor 100nF
                 |                        |     |
                 v                        v     v
                GND                      GND   GND
```

### Componentes del Circuito de Alimentación:
- **Diodo 1N4007**: Protección contra inversión de polaridad
- **Regulador 7805**: Convierte 12V a 5V estabilizados
- **Capacitor 10μF**: Filtro de entrada
- **Capacitor 100nF**: Filtro de salida
- **Resistencias**: Para limitar corriente en entradas/salidas

## Instalación

1. Clone este repositorio
2. Abra el proyecto en el IDE de ESP-IDF
3. Configure los pines según su hardware
4. Compile y cargue el programa en su ESP32

## Configuración de Pines

| Componente          | Pin ESP32 | Notas                                   |
|---------------------|-----------|----------------------------------------|
| Sensor MAP          | GPIO34    | Entrada ADC1 Canal 6                    |
| Válvula IAC (TIP120)| GPIO25    | Salida PWM para TIP120 de válvula IAC   |
| Cuerpo Aceleración  | GPIO26    | Salida PWM para conexión "Y" (opcional) |
| Relay de Marcha     | GPIO27    | Salida para TIP120 de control de marcha |
| Botón de Encendido  | GPIO33    | Entrada con pull-up                     |
| LED de Estado       | GPIO2     | LED integrado en placa                  |

## Modos de Operación

### Control de Válvula IAC

El sistema ofrece dos modos de control de flujo de aire:

1. **Modo Directo**: Control de válvula IAC mediante PWM a través de un transistor TIP120
   - Ideal para sustituir válvulas IAC defectuosas o sistemas sin control electrónico original

2. **Modo Conexión "Y"**: Envío de señales PWM al cuerpo de aceleración existente
   - Permite mantener el sistema original añadiendo control adicional basado en presión
   - Útil para vehículos más modernos donde no es recomendable modificar el control IAC original

Para seleccionar el modo, se puede usar la conexión Bluetooth y enviar los comandos:
- `IAC_MODE_1`: Activa el modo directo de control de IAC
- `IAC_MODE_2`: Activa el modo de conexión "Y" al cuerpo de aceleración

## Uso Básico

### Modo de Funcionamiento Normal

1. El sistema lee constantemente la presión del múltiple mediante el sensor MAP
2. Si la presión está fuera del rango seguro (20-105 kPa), el sistema no permite la activación de la marcha
3. Si la presión está por debajo del mínimo, ajusta automáticamente el flujo de aire mediante:
   - Control de la válvula IAC (Modo 1)
   - Envío de señales PWM al cuerpo de aceleración (Modo 2)
4. La marcha se detiene automáticamente después de 5 minutos para evitar sobrecalentamiento

### Control Manual

- **Botón físico**: Presione para encender/apagar la marcha (sujeto a verificación de condiciones)
- **Control Bluetooth**: Envíe comandos "START" o "STOP" mediante la conexión Bluetooth

## Parámetros Configurables

Los siguientes parámetros pueden ajustarse en el código fuente:

```c
#define MAP_MIN_PRESSURE     20      // Presión mínima en kPa
#define MAP_MAX_PRESSURE     105     // Presión máxima en kPa
#define MAX_IDLE_TIME        300000  // Tiempo máximo de marcha (ms)
#define IAC_CONTROL_MODE     1       // 1: Control directo, 2: Control con PWM en Y
```

## Precauciones de Seguridad

⚠️ **ADVERTENCIA**: Este sistema controla la marcha del vehículo. Siempre siga estas precauciones:

- En vehículos manuales, asegúrese de que el vehículo esté en NEUTRAL y con FRENO DE MANO para evitar accidentes
- No utilice el control remoto sin verificar visualmente la posición del vehículo
- Asegúrese de que el sensor MAP esté correctamente calibrado antes de usar el sistema
- No modifique las constantes de presión sin entender completamente su significado
- Asegure todas las conexiones eléctricas antes de la operación
- Utilice aislamiento adecuado para todas las conexiones en el vehículo
- El TIP120 puede calentarse; considere agregar un disipador térmico si se usa frecuentemente

## Montaje del Sistema en el Vehículo

### Consideraciones Importantes:

1. **Ubicación del Sensor MAP**:
   - Instale el sensor MAP en un punto del múltiple de admisión donde pueda medir la presión de manera precisa
   - Evite ubicaciones con calor excesivo o exposición a líquidos
   - Utilice mangueras de silicona de calidad para conectar el sensor al múltiple

2. **Conexión con la Válvula IAC**:
   - Identifique el conector de la válvula IAC original
   - Para el modo directo, desconecte la válvula original y conecte la controlada por el ESP32
   - Para el modo de conexión "Y", mantenga el control original y añada las conexiones adicionales

3. **Conexión con el Cuerpo de Aceleración**:
   - Para el modo de conexión "Y", identifique los cables de control del cuerpo de aceleración
   - Conecte la salida PWM del ESP32 en paralelo con el sistema original
   - Utilice diodos para evitar interferencias entre ambos sistemas

4. **Alimentación**:
   - Conecte el circuito de alimentación a una fuente de 12V que se active con el contacto del vehículo
   - Utilice fusibles adecuados para proteger el sistema
   - Asegure una buena conexión a masa

## Solución de Problemas

| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| No se activa la marcha | Presión MAP fuera de rango | Verificar sensor y calibración |
| | TIP120 defectuoso | Comprobar o reemplazar transistor |
| | Conexiones sueltas | Revisar cableado |
| | Relevador defectuoso | Comprobar funcionamiento del relevador |
| ESP32 no arranca | Problema de alimentación | Revisar circuito con 7805 |
| | Capacitores defectuosos | Comprobar filtrado |
| IAC no responde | TIP120 mal conectado | Verificar polaridad y resistencia de base |
| | Válvula obstruida | Limpiar válvula |

## Calibración Detallada con Instrumentos

### Calibración del Sensor MAP con Multímetro

Para determinar los rangos operativos exactos del sensor MAP y ajustar el código correctamente:

#### Materiales necesarios:
- Multímetro digital con medición de voltaje DC
- Bomba de vacío manual con manómetro (calibrada en kPa)
- Mangueras de conexión
- Cable jumper para pruebas

#### Procedimiento:
1. **Verificar el tipo de sensor MAP**
   - Los sensores MAP típicos tienen 3 pines: alimentación (5V), tierra (GND) y señal
   - Identificar los pines según el diagrama del fabricante

2. **Preparación del sensor**
   - Conectar el sensor MAP a una fuente de 5V estable
   - Conectar la manguera de la bomba de vacío al sensor

3. **Configuración del multímetro**
   - Colocar el multímetro en modo de voltaje DC (escala de 0-5V)
   - Conectar la sonda negativa a tierra (GND)
   - Conectar la sonda positiva al pin de señal del sensor

4. **Medición en puntos específicos**
   - **Punto 1**: Sin vacío aplicado (presión atmosférica, aprox. 101.3 kPa)
     - Anotar el voltaje (típicamente entre 4.5V y 4.9V)
   
   - **Punto 2**: Aplicar vacío medio (aprox. 50 kPa)
     - Usar la bomba de vacío para alcanzar 50 kPa en el manómetro
     - Anotar el voltaje (típicamente entre 2.0V y 3.0V)
   
   - **Punto 3**: Aplicar vacío máximo (lo más cercano a 0 kPa que permita la bomba)
     - Anotar el voltaje (típicamente entre 0.2V y 0.5V)

5. **Cálculo de la fórmula de conversión**
   - Con dos puntos podemos crear una ecuación lineal: `presion = m * voltaje + b`
   - Usar los puntos más extremos para mayor precisión
   - Ejemplo: Si a 100 kPa el voltaje es 4.8V, y a 20 kPa el voltaje es 0.5V
     - `m = (100 - 20) / (4.8 - 0.5) = 80 / 4.3 = 18.6`
     - `b = 100 - (m * 4.8) = 100 - 89.28 = 10.72`
     - Fórmula resultante: `presion_kpa = 18.6 * voltaje + 10.72`

6. **Actualizar el código del ESP32**
   - Localizar la sección de conversión en `ReadMAPSensor()`:
   ```c
   // Convertir voltaje a kPa según las mediciones realizadas
   // Reemplazar estos valores con los calculados en las pruebas
   uint32_t pressure_kpa = (m * voltage / 1000) + b;  // voltage está en mV
   ```

#### Tabla de registro de mediciones:

| Presión (kPa) | Voltaje (V) | Notas                      |
|---------------|-------------|----------------------------|
| 101.3         |             | Presión atmosférica normal |
| 80            |             |                            |
| 60            |             |                            |
| 40            |             |                            |
| 20            |             |                            |
| Mínimo        |             | Valor mínimo logrado       |

### Análisis de Señales de IAC y Cuerpo de Aceleración con Osciloscopio

Para visualizar y ajustar correctamente los pulsos PWM enviados a la válvula IAC o al cuerpo de aceleración:

#### Materiales necesarios:
- Osciloscopio digital (preferiblemente con al menos 50 MHz de ancho de banda)
- Sondas de osciloscopio con clips de conexión
- Herramienta de diagnóstico para monitorear datos en tiempo real (opcional)
- Un vehículo funcionando correctamente para capturar señales de referencia

#### Procedimiento para capturar señales de referencia:
1. **Preparación del vehículo**
   - Asegúrese que el motor esté a temperatura normal de funcionamiento
   - Coloque el vehículo en punto muerto con el freno de mano activado
   - Mantenga el motor en ralentí estable

2. **Conexión del osciloscopio a la válvula IAC original**
   - Identifique los cables de control de la válvula IAC
   - Para válvulas de paso a paso (stepper):
     - Conecte la sonda del osciloscopio a cada bobina por separado
     - Configure el osciloscopio en 5V/div vertical y 10ms/div horizontal
   - Para válvulas controladas por PWM:
     - Conecte la sonda al cable de señal
     - Configure el osciloscopio en 5V/div vertical y 20ms/div horizontal

3. **Captura de señales en diferentes condiciones**
   - Ralentí estable (800-900 RPM): Capturar la forma de onda durante 10 segundos
   - Aceleración súbita: Observe los cambios en el patrón de pulsos
   - Desaceleración: Observe cómo se ajusta la señal
   - Motor caliente vs. motor frío: Compare ambos patrones

4. **Análisis de la forma de onda PWM**
   - Medir la frecuencia de la señal (típicamente entre 50-300 Hz)
   - Medir el ciclo de trabajo (duty cycle) en diferentes condiciones:
     - Ralentí frío: Generalmente 30-60%
     - Ralentí caliente: Generalmente 10-30%
     - Durante compensación por carga: Hasta 80%

5. **Mediciones clave para PWM**
   - **Frecuencia**: Use la función de medición de tu osciloscopio
   - **Ciclo de trabajo**: Determinar el porcentaje que la señal permanece en estado alto
   - **Amplitud**: Verificar que sea consistente con los niveles lógicos esperados (generalmente 0-12V o 0-5V)
   - **Tiempo de subida/bajada**: Para evaluar la calidad de la señal

6. **Ajuste del código ESP32 según mediciones**
   - Modificar la configuración PWM en ConfigurePWM():
   ```c
   #define LEDC_FREQUENCY      XXX   // Reemplazar XXX con la frecuencia medida (Hz)
   ```

   - Ajustar los mapeos de apertura en ControlIACValve():
   ```c
   // Rangos de apertura ajustados según mediciones reales
   #define IAC_MIN_OPENING      XX   // Valor mínimo medido (0-255)
   #define IAC_MAX_OPENING      YY   // Valor máximo medido (0-255)
   ```

#### Interpretación de formas de onda PWM:

| Patrón observado | Interpretación | Ajuste recomendado |
|------------------|----------------|-------------------|
| Ciclo trabajo alto (>70%) | Apertura mayor para compensación | Aumentar IAC_MAX_OPENING |
| Ciclo trabajo bajo (<10%) | Condición mínima de ralentí | Ajustar IAC_MIN_OPENING |
| Frecuencia alta (>200Hz) | Control preciso para motores modernos | Aumentar LEDC_FREQUENCY |
| Pulsos irregulares | Posible compensación adaptativa | Implementar lógica de compensación |

### Determinación de Rangos de Presión Óptimos con Osciloscopio

Para determinar con precisión los valores operativos máximos y mínimos de presión en diferentes condiciones de funcionamiento:

#### Materiales necesarios:
- Osciloscopio digital con función de captura de datos
- Sensor MAP instalado en el vehículo o sensor MAP de prueba
- Interfaz para conectar la señal del sensor MAP al osciloscopio
- Manómetro de vacío para verificación cruzada (opcional)

#### Procedimiento para determinar valores operativos reales:

1. **Preparación para la medición**:
   - Conecte el osciloscopio al cable de señal del sensor MAP
   - Configure el osciloscopio para visualizar la señal de voltaje:
     - Escala vertical: 1V/div
     - Escala horizontal: 2-5 segundos/div
     - Modo de captura: Normal o Peak detect
   - Encienda el vehículo y deje que alcance la temperatura de operación normal

2. **Captura de señal en diferentes condiciones de marcha**:
   - **Condición de marcha mínima estable (ralentí)**:
     - Con el motor caliente a temperatura normal
     - Sin aceleración y sin cargas adicionales (A/C apagado, luces apagadas)
     - Registrar el voltaje promedio y convertirlo a presión (kPa)
   
   - **Condición de marcha con carga máxima**:
     - Con todas las cargas eléctricas activadas (A/C en máximo, luces encendidas)
     - Registrar el voltaje mínimo (presión máxima)
   
   - **Condición de desaceleración**:
     - Acelerar el motor a 2000 RPM y soltar el acelerador repentinamente
     - Registrar el voltaje máximo momentáneo (presión mínima)

3. **Análisis de fluctuaciones por factores ambientales**:
   - Repetir las mediciones en diferentes condiciones:
     - Motor frío (después de encender por primera vez en el día)
     - Motor a temperatura normal de operación
     - En condiciones de temperatura ambiente elevada (>30°C si es posible)
     - En condiciones de temperatura ambiente baja (<15°C si es posible)
     - En diferentes condiciones de humedad y altitud si es factible

4. **Documentación de los rangos observados**:

   | Condición | Voltaje Mín. (V) | Voltaje Máx. (V) | Presión Mín. (kPa) | Presión Máx. (kPa) |
   |-----------|-----------------|------------------|-------------------|-------------------|
   | Ralentí frío |              |                  |                   |                   |
   | Ralentí caliente |          |                  |                   |                   |
   | Carga máxima |              |                  |                   |                   |
   | Desaceleración |            |                  |                   |                   |
   | Altitud elevada* |          |                  |                   |                   |

   *Si aplica según ubicación geográfica

5. **Determinación de valores límite seguros**:
   - Establecer el valor de `MAP_MIN_PRESSURE` en un 5-10% por debajo del valor mínimo observado
   - Establecer el valor de `MAP_MAX_PRESSURE` en un 5-10% por encima del valor máximo observado
   - Estos márgenes de seguridad ayudan a compensar variaciones no previstas

### Factores Ambientales que Afectan la Calibración

Los siguientes factores pueden alterar significativamente las lecturas del sensor MAP y deben considerarse durante la calibración del sistema:

1. **Temperatura ambiente**:
   - Las bajas temperaturas aumentan la densidad del aire, lo que puede incrementar las lecturas de presión
   - Las altas temperaturas disminuyen la densidad del aire, reduciendo las lecturas de presión
   - Variación típica: ±3-5 kPa entre extremos de temperatura estacional

2. **Altitud geográfica**:
   - La presión atmosférica cambia aproximadamente 1.2 kPa por cada 100 metros de elevación
   - Sistemas calibrados a nivel del mar pueden requerir ajustes para operación en zonas montañosas
   - En elevaciones superiores a 1500m, considerar aumentar el valor de `MAP_MIN_PRESSURE` en 10-15 kPa

3. **Humedad relativa**:
   - Alta humedad puede reducir la densidad efectiva del aire
   - Efecto menor pero medible en condiciones extremas (±1-2 kPa)

4. **Cambios barométricos**:
   - Sistemas de baja presión (tormentas) pueden reducir temporalmente la presión atmosférica
   - Sistemas de alta presión pueden aumentarla
   - Variación típica: ±2-3 kPa

5. **Temperatura del motor**:
   - Un motor frío tiene menor eficiencia volumétrica, causando lecturas diferentes
   - La temperatura del colector de admisión afecta directamente las lecturas del MAP
   - Diferencia típica entre motor frío y caliente: 5-15 kPa

#### Estrategias de Compensación para Factores Ambientales:

1. **Implementación de una tabla de compensación por temperatura**:
   ```c
   // Ejemplo de compensación básica por temperatura
   float CompensateForTemperature(uint32_t rawPressure, float temperature) {
       // Factor simplificado basado en temperatura ambiente
       float compensationFactor = 1.0;
       
       if (temperature < 15.0) {
           // Compensación para temperaturas bajas
           compensationFactor = 1.0 + (15.0 - temperature) * 0.005; // +0.5% por cada °C debajo de 15°C
       } else if (temperature > 25.0) {
           // Compensación para temperaturas altas
           compensationFactor = 1.0 - (temperature - 25.0) * 0.003; // -0.3% por cada °C encima de 25°C
       }
       
       return rawPressure * compensationFactor;
   }
   ```

2. **Adaptación dinámica**:
   - Establecer un periodo de "aprendizaje" al encender el motor
   - Medir y promediar varios ciclos de presión antes de activar controles
   - Actualizar periódicamente los umbrales basándose en condiciones operativas

3. **Incorporación de sensor de temperatura externa**:
   - Añadir un sensor de temperatura ambiental permite ajustar automáticamente los valores
   - Recomendado para áreas con grandes variaciones climáticas

Para obtener la calibración más precisa, se recomienda realizar el procedimiento completo en diferentes condiciones meteorológicas y estacionales, documentando cada vez los resultados para identificar patrones de variación específicos del vehículo.

### Configuración del Sistema Basada en Mediciones

Una vez realizadas las mediciones con osciloscopio y multímetro, combine los resultados para ajustar el sistema:

1. **Actualización de constantes en el código**:
   ```c
   // Parámetros del sensor MAP según mediciones con multímetro
   #define MAP_VOLTAGE_MIN    XXX    // Voltaje mínimo medido (mV) 
   #define MAP_VOLTAGE_MAX    YYY    // Voltaje máximo medido (mV)
   #define MAP_PRESSURE_MIN   AA     // Presión mínima correspondiente (kPa)
   #define MAP_PRESSURE_MAX   BB     // Presión máxima correspondiente (kPa)
   
   // Parámetros PWM según mediciones con osciloscopio
   #define LEDC_FREQUENCY     FF     // Frecuencia medida (Hz)
   ```

2. **Modificación de la función de conversión**:
   ```c
   // En la función ReadMAPSensor()
   uint32_t pressure_kpa = MAP_PRESSURE_MIN + 
       (voltage - MAP_VOLTAGE_MIN) * (MAP_PRESSURE_MAX - MAP_PRESSURE_MIN) / 
       (MAP_VOLTAGE_MAX - MAP_VOLTAGE_MIN);
   ```

3. **Pruebas incrementales**:
   - Realice pruebas con cada ajuste por separado
   - Verifique que el comportamiento del sistema sea similar al original
   - Documente los cambios y sus efectos

### Consejos para la calibración precisa:

1. **Para el sensor MAP**:
   - Verificar que la lectura a presión atmosférica sea consistente (alrededor de 100-102 kPa)
   - Comprobar la linealidad realizando mediciones en múltiples puntos
   - Considerar efectos de temperatura en sensores antiguos

2. **Para el control PWM**:
   - Los vehículos más modernos suelen utilizar frecuencias PWM más altas (100-300 Hz)
   - Los vehículos más antiguos pueden usar frecuencias más bajas (50-100 Hz)
   - El rango útil del ciclo de trabajo suele estar entre 10-80%

3. **Solución de problemas comunes**:
   - Si hay oscilación del ralentí: reducir la sensibilidad de respuesta del sistema
   - Si el motor se apaga: verificar los límites mínimos de apertura IAC
   - Si hay ralentí inestable: revisar la precisión de las lecturas del sensor MAP

## Calibración del Sensor MAP

Para calibrar el sensor MAP, ajuste la siguiente sección del código:

```c
// Convertir voltaje a kPa (depende de la calibración del sensor específico)
// Asumiendo sensor típico MAP: 0.5V = 10 kPa, 4.5V = 105 kPa
uint32_t pressure_kpa = ((voltage - 500) * 95) / 4000 + 10;
```

## Contribuciones

Las contribuciones son bienvenidas. Por favor, abra un issue para discutir los cambios propuestos.

## Licencia

Este proyecto está licenciado bajo los términos especificados en el archivo LICENSE del repositorio principal.