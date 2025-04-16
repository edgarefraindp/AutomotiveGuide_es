# Manual de Usuario: Pulsador de Inyectores para ESP32

## Descripción
Este proyecto utiliza el microcontrolador ESP32 para simular señales de pulso para 4 inyectores de combustible, imitando diferentes órdenes de encendido comunes en motores de 4 cilindros. Es una herramienta útil para mecánicos que necesitan probar el funcionamiento de inyectores de combustible sin necesidad de instalarlos en un motor real. Esta versión aprovecha las capacidades avanzadas del ESP32, como su ADC de mayor resolución.

## Requisitos
- Placa ESP32 (DevKit, NodeMCU-ESP32, etc.)
- 4 LEDs o inyectores reales (con sus respectivos drivers si se usan inyectores reales)
- Transistores TIP120 o MOSFET IRF540N (para conectar inyectores reales)
- Diodos 1N4007 (para protección contra corriente inversa)
- 1 potenciómetro de 10kΩ
- 1 pulsador
- Resistencias de 220Ω para los LEDs (si se utilizan LEDs)
- Resistencias de 1kΩ para las bases de los transistores
- Cables de conexión
- Fuente de alimentación adecuada (12V para inyectores reales)

## Conexiones
1. **Inyectores (o LEDs):**
   - Inyector #1 → Pin 16 (a través de transistor si se usan inyectores reales)
   - Inyector #2 → Pin 17 (a través de transistor si se usan inyectores reales)
   - Inyector #3 → Pin 18 (a través de transistor si se usan inyectores reales)
   - Inyector #4 → Pin 19 (a través de transistor si se usan inyectores reales)

2. **Potenciómetro (simula el acelerador):**
   - Terminal central → Pin 34 (entrada analógica)
   - Terminales laterales → 3.3V y GND

3. **Pulsador para cambiar el orden de encendido:**
   - Un terminal → Pin 5
   - Otro terminal → GND

4. **LED de estado:**
   - Conectado al Pin 2 (LED integrado en muchas placas ESP32)

## Diagrama de conexión
```
                           +-----+
                           |     |
      +------------------o |     |
      |                    +-----+
      |                     ESP32
      |
LED/Inyector 1 ------ Pin 16
LED/Inyector 2 ------ Pin 17
LED/Inyector 3 ------ Pin 18
LED/Inyector 4 ------ Pin 19
      |
      +------ Resistencia 220Ω -------- GND
      
Potenciómetro:
      Terminal Central ---- Pin 34
      Terminal Lateral 1 -- 3.3V
      Terminal Lateral 2 -- GND
      
Pulsador:
      Terminal 1 ---------- Pin 5
      Terminal 2 ---------- GND
```

### Conexión de inyectores reales con transistor o MOSFET
Si desea conectar inyectores reales, debe utilizar un transistor TIP120 (Darlington NPN) o un MOSFET IRF540N para cada inyector:

#### Con transistor TIP120:
```
ESP32 Pin (16,17,18,19) ---- Resistencia 1kΩ ---- Base del TIP120
                                                     |
                                                  Colector del TIP120 ---- Inyector ---- +12V
                                                     |                       |
                                                     |                     Diodo 1N4007 (cátodo hacia +12V)
                                                     |
                                                  Emisor del TIP120 ---- GND
```

#### Con MOSFET IRF540N (recomendado para ESP32):
```
ESP32 Pin (16,17,18,19) ---- Resistencia 1kΩ ---- Gate del MOSFET
                                                     |
                                                  Drain del MOSFET ---- Inyector ---- +12V
                                                     |                    |
                                                     |                  Diodo 1N4007 (cátodo hacia +12V)
                                                     |
                                                  Source del MOSFET ---- GND
```

## Funcionamiento
1. **Carga del programa:**
   - Se debe conectar el ESP32 a una computadora mediante USB
   - Es necesario abrir el archivo `pulsadorInyectores_ESP32.ino` en el IDE de Arduino
   - Debe instalar la biblioteca ESP32 para Arduino IDE si no lo ha hecho (en Gestor de Placas)
   - Se debe seleccionar el tipo de placa ESP32 correcto y el puerto USB
   - Se debe cargar el programa al ESP32

2. **Uso del dispositivo:**
   - El potenciómetro controla la velocidad de pulsación (simula la posición del acelerador)
     - Girando a la izquierda: Pulsos más lentos (menor RPM)
     - Girando a la derecha: Pulsos más rápidos (mayor RPM)
   
   - El pulsador cambia entre diferentes órdenes de encendido:
     - Orden 1-3-4-2 (común en motores de 4 cilindros en línea)
     - Orden 1-2-4-3 (algunos motores europeos)
     - Orden 1-3-2-4 (algunos motores japoneses)

   - El LED de estado (Pin 2) parpadea para indicar el funcionamiento
   - La información de operación puede visualizarse en el Monitor Serial a 115200 hertz

## Órdenes de encendido
El programa simula tres órdenes de encendido comunes:
1. **1-3-4-2:** Utilizado en muchos motores de 4 cilindros en línea
2. **1-2-4-3:** Común en algunos motores europeos
3. **1-3-2-4:** Utilizado en algunos motores japoneses

Se debe presionar el botón conectado al Pin 5 para cambiar entre estos órdenes.

## Ventajas de esta versión con ESP32
1. **Mayor resolución del ADC:** El ESP32 tiene un ADC de 12 bits (0-4095) en comparación con los 10 bits (0-1023) de Arduino, lo que permite un control más preciso del acelerador.
2. **Mayor velocidad de procesamiento:** El ESP32 funciona a 240MHz, mucho más rápido que Arduino, permitiendo tiempos de respuesta más precisos.
3. **Implementación mejorada del debounce:** Esta versión incluye protección mejorada contra rebotes en el botón para cambio de orden.
4. **Posibilidad de expansión:** El ESP32 tiene capacidades WiFi y Bluetooth que podrían utilizarse para controlar el pulsador remotamente o registrar datos.

## Solución de problemas comunes
1. **Los LEDs/inyectores no pulsan:**
   - Se recomienda verificar las conexiones de cada LED/inyector
   - Es necesario asegurarse de que el ESP32 esté correctamente alimentado
   - Se debe comprobar que el programa se haya cargado correctamente
   - Verifique que esté utilizando pines GPIO que no tengan funciones especiales en el arranque

2. **No cambia el orden de encendido:**
   - Se debe verificar la conexión del pulsador al pin 5
   - Es necesario asegurarse de que el pulsador esté conectado a GND correctamente
   - Verifique que el sistema de debounce no esté causando problemas (ajuste DEBOUNCE_DELAY si es necesario)

3. **La velocidad no cambia:**
   - Se recomienda comprobar la conexión del potenciómetro al pin 34
   - Es importante verificar que el potenciómetro esté conectado correctamente a 3.3V y GND (no use 5V con ESP32)
   - Verifique en el monitor serial los valores leídos del ADC

4. **Problemas de compilación:**
   - Asegúrese de haber instalado el soporte para ESP32 en el IDE de Arduino
   - Verifique que ha seleccionado la placa ESP32 correcta en el menú Herramientas > Placa

## Conceptos básicos
- **Orden de encendido:** Es la secuencia en la que los cilindros de un motor reciben la chispa para la combustión. En este proyecto se simulan diferentes órdenes usando LEDs o inyectores.
- **Duración del pulso:** Representa el tiempo que el inyector permanece abierto. En un motor real, esto determina la cantidad de combustible inyectado.
- **Tiempo de ciclo:** Intervalo entre ciclos de inyección, relacionado con las RPM del motor.
- **ADC (Convertidor Analógico-Digital):** Componente que convierte señales analógicas (como la posición del potenciómetro) en valores digitales que el microcontrolador puede procesar.

## Modificaciones posibles
- Se pueden ajustar las constantes `MIN_PULSE_TIME` y `MAX_PULSE_TIME` para cambiar el rango de duración de los pulsos
- Es posible modificar `MIN_CYCLE_TIME` y `MAX_CYCLE_TIME` para alterar la velocidad de los ciclos
- Puede implementar funciones WiFi/Bluetooth para control remoto o monitoreo
- Puede agregar una pantalla OLED/LCD para mostrar información sin necesidad de computadora

## Precauciones de seguridad
- Al conectar inyectores reales, es necesario utilizar transistores o MOSFETs como drivers y una fuente de alimentación de 12V adecuada
- Los transistores pueden calentarse durante el uso prolongado; se recomienda considerar añadir disipadores de calor
- Siempre se deben incluir diodos 1N4007 en paralelo con los inyectores para proteger el circuito contra corrientes inversas
- No se deben conectar los inyectores directamente a los pines del ESP32 sin los drivers y resistencias adecuadas
- Es fundamental asegurarse de que la fuente de 12V tenga capacidad suficiente para alimentar todos los inyectores
- Se debe desconectar la alimentación antes de hacer cambios en el circuito
- Es importante verificar todas las conexiones antes de aplicar energía al circuito
- El ESP32 funciona con 3.3V, no conecte entradas a 5V directamente, ya que podría dañar el microcontrolador

## Para principiantes
- Si es su primera vez trabajando con ESP32, puede ser útil comenzar solo con LEDs en lugar de inyectores reales
- La conexión USB proporciona suficiente energía para el ESP32 y los LEDs, pero necesitará una fuente externa para inyectores reales
- Puede monitorizar el funcionamiento a través del Monitor Serial del IDE de Arduino a 115200 hertz

## Contacto y soporte
Para soporte o para realizar donaciones, se puede visitar la página del repositorio de GitHub: https://github.com/edgarefraindp/AutomotiveGuide_es