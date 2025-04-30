# Control de Luces Estilo Audi para Vehículos

Este proyecto simula el sistema de iluminación de un vehículo moderno, con efectos secuenciales similares a los utilizados en vehículos Audi. El controlador maneja las siguientes luces:

- Luces altas y bajas
- Luces de posición (cuartos) delanteras y traseras
- Direccionales delanteras y traseras con 16 LEDs y efecto secuencial avanzado
- Luces de freno (stop)
- Luces de emergencia
- Secuencia de bienvenida al encender el vehículo

## ¿Qué hace?

Este controlador emula el comportamiento de los sistemas de iluminación de vehículos modernos, incluyendo efectos secuenciales avanzados en las direccionales que simulan movimiento. Al encenderse el vehículo, se ejecuta una secuencia de bienvenida tipo Audi.

Las direccionales utilizan una matriz de 16 LEDs por cada "cuarto" (delantera izquierda, delantera derecha, trasera izquierda, trasera derecha) controlados por un multiplexor de 4 bits, lo que permite efectos de iluminación mucho más sofisticados y similares a los vehículos premium actuales.

## Comparación con modelos anteriores

A diferencia de los sistemas tradicionales donde las luces simplemente se encienden o apagan, o los sistemas más recientes con 3-4 LEDs secuenciales, este proyecto implementa una matriz de 16 LEDs por direccional. Esto permite crear efectos visuales más fluidos y atractivos, similares a los encontrados en vehículos Audi de gama alta recientes, donde la iluminación es un elemento distintivo de diseño y seguridad.

## Cómo funciona

El sistema utiliza multiplexores de 4 bits para controlar matrices de 16 LEDs en cada direccional. Esto se logra mediante:

1. **Circuito multiplexor**: Se utilizan 4 pines de control (representando valores binarios) para seleccionar cuál de los 16 LEDs se ilumina en un momento dado.
2. **Pines de habilitación**: Cada "cuarto" (grupo de direccionales) tiene su propio pin de habilitación, permitiendo activar independientemente cada grupo.
3. **Persistencia visual**: Mediante la técnica de multiplexación rápida, se encienden y apagan los LEDs a alta velocidad, creando la ilusión de que múltiples LEDs están encendidos simultáneamente.

### En palabras sencillas

Imagine que tiene 16 LEDs por cada luz direccional, pero solo cuenta con unos pocos cables para controlarlos. En lugar de usar un cable para cada LED (lo que requeriría 64 cables en total para las 4 direccionales), se utiliza un sistema similar a una "centralita telefónica" que decide qué LED debe encenderse en cada momento. Es como si tuviéramos un "director" (el Arduino) que le dice a un "asistente" (el multiplexor) exactamente qué LED encender y en qué momento, logrando crear patrones de movimiento fluidos con solo 8 cables (4 para indicar qué LED y 4 para indicar en qué grupo).

## Especificaciones eléctricas

- Voltaje de operación: 5V (Arduino)
- Consumo estimado: 100-250mA (dependiendo del número de LEDs activos simultáneamente)
- Entradas: 7 señales (encendido, freno, interruptor de luces, luces altas, direccionales izquierda/derecha, emergencia)
- Salidas: 
  - 8 salidas digitales para luces individuales (bajas, altas, posición, freno)
  - 8 salidas digitales para el sistema de direccionales (4 para el multiplexor + 4 de habilitación)

## Conexiones

### Entradas
- PIN 32: Señal de encendido del vehículo
- PIN 33: Señal del pedal de freno
- PIN 34: Interruptor de luces (posición 1)
- PIN 35: Interruptor de luces (posición 2)
- PIN 36: Palanca de luces altas
- PIN 37: Palanca direccional izquierda
- PIN 38: Palanca direccional derecha
- PIN 39: Botón de luces de emergencia

### Salidas (LEDs)
- Luces bajas: Pines 2-3
- Luces altas: Pines 4-5
- Luces de posición delanteras: Pines 6-7
- Luces de posición traseras: Pines 24-25
- Luces de freno: Pines 26-27
- Sistema de direccionales:
  * Pines 8-11: Control del multiplexor (4 bits A0-A3)
  * Pin 12: Habilitación direccional delantera izquierda
  * Pin 13: Habilitación direccional delantera derecha
  * Pin 22: Habilitación direccional trasera izquierda
  * Pin 23: Habilitación direccional trasera derecha

## Diagrama de conexión del multiplexor

```
    +------+    +----------+    +-----+
    |      |    |          |    | LED |
    | Pines|--->| 4-to-16  |--->| 0-15|
    | 8-11 |    |Multiplexor|    |     |
    +------+    +----------+    +-----+
        |
        |       +-----+
        |       |     |
        +------>|Enable|----> Cada grupo
                |Pins |      de 16 LEDs
                +-----+
```

Cada uno de los 4 grupos de luces direccionales (delantero izquierdo, delantero derecho, trasero izquierdo, trasero derecho) consta de 16 LEDs controlados por el mismo multiplexor, pero con diferentes señales de habilitación.

## Cuando falla

Posibles problemas y soluciones:

- **Las luces no encienden**: Verificar conexiones y polaridad de los LEDs.
- **Las secuencias no funcionan correctamente**: Comprobar que las entradas estén funcionando (usar Serial.print para depurar).
- **Comportamiento errático**: Verificar posibles cortocircuitos o conexiones inestables.

## Mantenimiento

- Revisar periódicamente las conexiones para asegurar que no haya contactos falsos.
- Comprobar el estado de los interruptores/botones de entrada, ya que pueden deteriorarse con el uso.

## Consejo práctico

Cuando monte este circuito, es recomendable usar resistencias limitadoras de corriente para cada LED (típicamente 220-330 ohm). Para simplificar el cableado, considere usar transistores o controladores de LED si planea conectar luces reales de mayor potencia.

## Proceso de diagnóstico simplificado

1. Verificar alimentación del Arduino (LED de power encendido)
2. Ejecutar el ejemplo y observar la secuencia inicial de prueba
3. Si alguna luz no funciona:
   - Comprobar conexiones
   - Verificar el LED
   - Comprobar que el pin correspondiente esté configurado como OUTPUT
4. Si una función no responde (ej: direccionales):
   - Verificar interruptores de entrada
   - Usar Serial.println() para depurar valores leídos
   - Comprobar lógica de activación

## Modificaciones posibles

- Ajustar velocidad de las secuencias modificando el parámetro de duración en las funciones de animación
- Cambiar el patrón de la secuencia de bienvenida
- Modificar el número de LEDs activos en el barrido (parámetro `numActiveLedsInSweep`)
- Crear nuevos patrones de animación implementando nuevas funciones generadoras de patrones
- Implementar efectos de atenuación mediante PWM para direccionales con intensidad variable
- Añadir sensores para ajuste automático de intensidad según condiciones ambientales

## Materiales necesarios

- 1 Arduino (Mega recomendado por el número de pines)
- 64 LEDs para direccionales (16 por cada cuarto: delantera izquierda/derecha, trasera izquierda/derecha)
- 4 multiplexores 4-a-16 (como el 74HC154 o similar)
- 8 LEDs adicionales para luces de posición, freno, bajas y altas
- Resistencias de 220-330 ohm (una por cada salida del multiplexor)
- 7 pulsadores o interruptores para las entradas
- Protoboard y cables de conexión
- Opcional: transistores para manejar cargas mayores

## Compatibilidad

Este código está desarrollado para Arduino Mega debido al alto número de pines requeridos. Para Arduino Uno o Nano, se necesitaría utilizar registros de desplazamiento adicionales para expandir el número de salidas disponibles.