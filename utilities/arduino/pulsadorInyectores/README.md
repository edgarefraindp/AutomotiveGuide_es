> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Manual de Usuario: Pulsador de Inyectores

## Descripción
Este proyecto utiliza Arduino para simular señales de pulso para 4 inyectores de combustible, imitando diferentes órdenes de encendido comunes en motores de 4 cilindros. Es una herramienta útil para mecánicos que necesitan probar el funcionamiento de inyectores de combustible sin necesidad de instalarlos en un motor real.

## Requisitos
- Arduino UNO o compatible
- 4 LEDs o inyectores reales (con sus respectivos drivers si se usan inyectores reales)
- Transistores TIP120 (para conectar inyectores reales)
- Diodos 1N4007 (para protección contra corriente inversa)
- 1 potenciómetro de 10kΩ
- 1 pulsador
- Resistencias de 220Ω para los LEDs (si se utilizan LEDs)
- Resistencias de 1kΩ para las bases de los transistores
- Cables de conexión
- Fuente de alimentación adecuada (12V para inyectores reales)

## Conexiones
1. **Inyectores (o LEDs):**
   - Inyector #1 → Pin 9 (a través de transistor TIP120 si se usan inyectores reales)
   - Inyector #2 → Pin 10 (a través de transistor TIP120 si se usan inyectores reales)
   - Inyector #3 → Pin 11 (a través de transistor TIP120 si se usan inyectores reales)
   - Inyector #4 → Pin 12 (a través de transistor TIP120 si se usan inyectores reales)

2. **Potenciómetro (simula el acelerador):**
   - Terminal central → Pin A0
   - Terminales laterales → 5V y GND

3. **Pulsador para cambiar el orden de encendido:**
   - Un terminal → Pin 2
   - Otro terminal → GND

4. **LED de estado:**
   - Utiliza el LED integrado en el Arduino (Pin 13)

## Diagrama de conexión
```
                           +-----+
                           |     |
      +------------------o |     |
      |                    +-----+
      |                    Arduino
      |
LED/Inyector 1 ------ Pin 9
LED/Inyector 2 ------ Pin 10
LED/Inyector 3 ------ Pin 11
LED/Inyector 4 ------ Pin 12
      |
      +------ Resistencia 220Ω -------- GND
      
Potenciómetro:
      Terminal Central ---- Pin A0
      Terminal Lateral 1 -- 5V
      Terminal Lateral 2 -- GND
      
Pulsador:
      Terminal 1 ---------- Pin 2
      Terminal 2 ---------- GND
```

### Conexión de inyectores reales con transistor TIP120
Si desea conectar inyectores reales, debe utilizar un transistor TIP120 (Darlington NPN) para cada inyector, de la siguiente manera:

```
Arduino Pin (9,10,11,12) ---- Resistencia 1kΩ ---- Base del TIP120
                                                     |
                                                    Colector del TIP120 ---- Inyector ---- +12V
                                                     |                         |
                                                     |                        Diodo 1N4007 (cátodo hacia +12V)
                                                     |
                                                    Emisor del TIP120 ---- GND
```

## Funcionamiento
1. **Carga del programa:**
   - Se debe conectar el Arduino a una computadora mediante USB
   - Es necesario abrir el archivo `pulsadorInyectores.ino` en el IDE de Arduino
   - Se debe seleccionar el tipo de placa y puerto correctos
   - Se debe cargar el programa al Arduino

2. **Uso del dispositivo:**
   - El potenciómetro controla la velocidad de pulsación (simula la posición del acelerador)
     - Girando a la izquierda: Pulsos más lentos (menor RPM)
     - Girando a la derecha: Pulsos más rápidos (mayor RPM)
   
   - El pulsador cambia entre diferentes órdenes de encendido:
     - Orden 1-3-4-2 (común en motores de 4 cilindros en línea)
     - Orden 1-2-4-3 (algunos motores europeos)
     - Orden 1-3-2-4 (algunos motores japoneses)

   - El LED de estado (Pin 13) parpadea para indicar el funcionamiento

## Órdenes de encendido
El programa simula tres órdenes de encendido comunes:
1. **1-3-4-2:** Utilizado en muchos motores de 4 cilindros en línea
2. **1-2-4-3:** Común en algunos motores europeos
3. **1-3-2-4:** Utilizado en algunos motores japoneses

Se debe presionar el botón conectado al Pin 2 para cambiar entre estos órdenes.

## Solución de problemas comunes
1. **Los LEDs/inyectores no pulsan:**
   - Se recomienda verificar las conexiones de cada LED/inyector
   - Es necesario asegurarse de que el Arduino esté correctamente alimentado
   - Se debe comprobar que el programa se haya cargado correctamente

2. **No cambia el orden de encendido:**
   - Se debe verificar la conexión del pulsador
   - Es necesario asegurarse de que el pulsador esté conectado a GND correctamente

3. **La velocidad no cambia:**
   - Se recomienda comprobar la conexión del potenciómetro
   - Es importante verificar que el potenciómetro esté conectado correctamente a 5V y GND

## Conceptos básicos
- **Orden de encendido:** Es la secuencia en la que los cilindros de un motor reciben la chispa para la combustión. En este proyecto se simulan diferentes órdenes usando LEDs o inyectores.
- **Duración del pulso:** Representa el tiempo que el inyector permanece abierto. En un motor real, esto determina la cantidad de combustible inyectado.
- **Tiempo de ciclo:** Intervalo entre ciclos de inyección, relacionado con las RPM del motor.

## Modificaciones posibles
- Se pueden ajustar las constantes `MIN_PULSE_TIME` y `MAX_PULSE_TIME` para cambiar el rango de duración de los pulsos
- Es posible modificar `MIN_CYCLE_TIME` y `MAX_CYCLE_TIME` para alterar la velocidad de los ciclos

## Precauciones de seguridad
- Al conectar inyectores reales, es necesario utilizar transistores TIP120 como drivers y una fuente de alimentación de 12V adecuada
- Los transistores TIP120 pueden calentarse durante el uso prolongado; se recomienda considerar añadir disipadores de calor si es necesario
- Siempre se deben incluir diodos 1N4007 en paralelo con los inyectores para proteger el circuito contra corrientes inversas
- No se deben conectar los inyectores directamente a los pines del Arduino sin los transistores TIP120 y resistencias adecuadas
- Es fundamental asegurarse de que la fuente de 12V tenga capacidad suficiente para alimentar todos los inyectores
- Se debe desconectar la alimentación antes de hacer cambios en el circuito
- Es importante verificar todas las conexiones antes de aplicar energía al circuito

## Contacto y soporte
Para soporte o para realizar donaciones, se puede visitar la página del repositorio de GitHub: https://github.com/edgarefraindp/AutomotiveGuide_es