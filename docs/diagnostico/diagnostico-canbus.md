# Diagnóstico de Problemas en Sistema CAN Bus

> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

## Diagnóstico Cuando el Escáner No Lee el CAN Bus

### ¿Qué hacer cuando el escáner no comunica?

Cuando un escáner de diagnóstico no logra establecer comunicación con el sistema CAN bus del vehículo, esto puede deberse a múltiples factores que van desde problemas eléctricos simples hasta fallos complejos en el sistema de comunicaciones.

### Comparación con modelos anteriores

En los vehículos anteriores al año 2000, la falta de comunicación generalmente significaba un problema en el conector de diagnóstico o en la ECU específica. En los vehículos modernos con múltiples redes, el problema puede estar en la red CAN, en el Gateway, en alguna ECU que está saturando la red, o en la configuración del propio escáner.

### Cómo funciona el diagnóstico CAN bus

El sistema CAN funciona con dos líneas (CAN-H y CAN-L) que en estado de reposo están ambas a 2.5V. Cuando se transmite un bit dominante (0), CAN-H sube a 3.5V y CAN-L baja a 1.5V, creando una diferencia de 2V. Los mensajes se estructuran en tramas con identificadores y datos, y cualquier alteración en este sistema puede impedir la comunicación.

### En palabras sencillas

Imagine el sistema CAN como una sala de conferencias donde todos hablan según reglas estrictas. Si alguien "grita" constantemente (una ECU defectuosa), nadie más puede comunicarse. Si los "micrófonos" están desconectados (cables dañados), no hay comunicación. O si el "traductor" (Gateway) no funciona, las diferentes "salas" (redes) no pueden entenderse entre sí.

### Proceso de diagnóstico simplificado

#### 1. Verificaciones básicas iniciales
1. **Verificar alimentación en conector OBD-II**:
   * **Herramienta**: Multímetro
   * **Procedimiento**: Medir voltaje en pin 16 (+) y pin 4 o 5 (masa)
   * **Valor esperado**: 12-14V con motor encendido
   * **Solución si falla**: Revisar fusibles relacionados con el sistema de diagnóstico (consultar manual del vehículo)

2. **Comprobar continuidad de líneas CAN en conector OBD-II**:
   * **Herramienta**: Multímetro
   * **Procedimiento**: Medir resistencia entre pin 6 (CAN-H) y pin 14 (CAN-L)
   * **Valor esperado**: Aproximadamente 60 ohms (equivalente a dos resistencias terminadoras de 120 ohms en paralelo)
   * **Solución si falla**: Si la resistencia es infinita, hay un cable cortado; si es cercana a cero, hay un cortocircuito

### Verificación de resistencias terminadoras del CAN bus

#### ¿Qué son las resistencias terminadoras?

Las resistencias terminadoras son componentes críticos en cualquier red CAN bus. Se trata de resistencias de 120 ohms (±10%) ubicadas en los extremos de la red CAN para:

1. Evitar reflexiones de señal que provocarían errores de comunicación
2. Mantener el estado recesivo del bus cuando ningún nodo está transmitiendo
3. Garantizar la correcta diferencia de voltaje entre CAN-H y CAN-L

En un vehículo típicamente existen dos resistencias terminadoras, una en cada extremo físico de la red, generalmente integradas dentro de módulos importantes (por ejemplo, en la ECU principal y en otro módulo crítico como el ABS o Gateway).

#### Procedimiento detallado para comprobar las resistencias terminadoras

1. **Prueba básica desde el conector OBD-II**:
   * **Preparación**:
     * Motor apagado, llave fuera del contacto
     * Multímetro digital en modo resistencia (ohmios)
   * **Procedimiento**:
     * Localizar pines CAN en conector OBD-II: pin 6 (CAN-H) y pin 14 (CAN-L)
     * Conectar las puntas del multímetro entre estos dos pines
     * Leer el valor de resistencia
   * **Interpretación**:
     * **Valor correcto**: 60 ohms (±10%) - Indica que ambas resistencias de 120 ohms están presentes y conectadas en paralelo
     * **Valor cercano a 120 ohms**: Solo una resistencia está activa o conectada
     * **Valor muy alto o infinito**: Ambas resistencias están desconectadas o hay un cable roto
     * **Valor muy bajo**: Hay un cortocircuito entre CAN-H y CAN-L

2. **Prueba avanzada con módulos desconectados** (para técnicos experimentados):
   * **Preparación**:
     * Identificar los módulos que probablemente contienen las resistencias terminadoras (consultar manual del fabricante)
     * Asegurar que la batería esté desconectada antes de manipular conectores
   * **Procedimiento**:
     * Desconectar el primer módulo sospechoso de contener una resistencia terminadora
     * Medir nuevamente la resistencia entre CAN-H y CAN-L en el conector OBD-II
     * Si el valor cambia a aproximadamente 120 ohms, el módulo desconectado contiene una de las resistencias
     * Reconectar ese módulo y desconectar el siguiente módulo sospechoso
     * Repetir la medición para confirmar la ubicación de la segunda resistencia
   * **Interpretación**:
     * Si al desconectar un módulo la resistencia cambia de 60 ohms a 120 ohms: el módulo desconectado contiene una de las resistencias
     * Si al desconectar un módulo la resistencia sigue siendo 60 ohms: ese módulo no contiene una resistencia terminadora

3. **Verificación de resistencias terminadoras en módulos específicos**:
   * **Procedimiento**:
     * Desconectar completamente el módulo del vehículo
     * Usando el diagrama de pines del fabricante, identificar los pines CAN-H y CAN-L del módulo
     * Medir la resistencia entre estos pines
   * **Interpretación**:
     * Si la resistencia es aproximadamente 120 ohms: el módulo contiene una resistencia terminadora
     * Si la resistencia es muy alta o infinita: el módulo no contiene una resistencia terminadora o está dañada

#### Reparación de problemas de resistencia terminadora

1. **Cuando no se detecta ninguna resistencia**:
   * **Solución temporal**: Instalar una resistencia externa de 120 ohms entre CAN-H y CAN-L cerca del conector OBD-II para diagnósticos
   * **Solución definitiva**: Identificar y reemplazar el módulo con la resistencia terminadora defectuosa

2. **Cuando solo se detecta una resistencia (120 ohms)**:
   * Determinar cuál de los módulos con resistencia terminadora está defectuoso mediante prueba por desconexión
   * Revisar el cableado hacia ese módulo antes de considerar su reemplazo

3. **Cuando se detecta un cortocircuito (resistencia cercana a 0 ohms)**:
   * Inspeccionar visualmente el cableado del bus CAN buscando daños físicos
   * Revisar áreas comunes de problemas: pasos por el firewall, áreas cercanas a fuentes de calor, puntos de flexión (puertas, maletero)
   * Utilizar un probador de continuidad para localizar el punto exacto del cortocircuito

> **Para principiantes**: Las resistencias terminadoras son como las tapas en los extremos de una tubería de agua. Sin ellas, la "presión" de la señal eléctrica no se mantiene adecuadamente y la comunicación falla. Un valor de aproximadamente 60 ohms entre CAN-H y CAN-L con el motor apagado es una buena señal de que el "sistema de tuberías" del CAN bus está correctamente "cerrado" en ambos extremos.

#### 2. Verificación visual de cableado
1. **Inspeccionar físicamente**:
   * Buscar cables pelados, oxidados o dañados
   * Revisar conectores sueltos, especialmente cerca del módulo Gateway y ECU principal
   * Verificar posibles daños por agua o sobrecalentamiento
   * Comprobar si hay modificaciones incorrectas o instalaciones aftermarket mal ejecutadas

#### 3. Análisis con herramientas específicas

### Verificación de señales con osciloscopio

#### Forma y características de la señal CAN bus en el osciloscopio

La observación de las señales CAN bus con un osciloscopio es una de las herramientas más poderosas para el diagnóstico de problemas de comunicación. A continuación se detalla cómo debe verse una señal CAN saludable y cómo interpretar diferentes anomalías:

1. **Señal CAN normal (saludable)**:
   * **Forma de onda**: Ondas cuadradas complementarias en CAN-H y CAN-L
   * **Niveles de voltaje**:
     * **Estado recesivo** (bit 1): Ambas líneas a aproximadamente 2.5V
     * **Estado dominante** (bit 0): CAN-H sube a 3.5V y CAN-L baja a 1.5V
   * **Diferencia de potencial**: 0V en estado recesivo, 2V en estado dominante
   * **Flancos**: Deben ser limpios y alineados entre ambas señales, con tiempos de subida/bajada rápidos pero no excesivamente abruptos
   * **Base de tiempo recomendada**: 10-50 µs/división para ver varios bits, 1-5 µs/división para analizar la calidad de los flancos

2. **Esquema básico de la señal CAN**:
   ```
   Voltaje
   (V)
   3.5V  ┌─┐     ┌───┐   ┌─┐                 CAN-H
         │ │     │   │   │ │
   2.5V  │ │─────│   │───│ │─────────────    Estado recesivo
         │ │     │   │   │ │
   1.5V  └─┘     └───┘   └─┘                 CAN-L
         
         [Dominante][Recesivo][Dominante]    Estados
         
         <─────── Tiempo ───────>
   ```

3. **Evaluación de la calidad de la señal**:
   * **Tiempo de bit**: Para CAN a 500 kbps, cada bit dura 2 µs
   * **Punto de muestreo**: Aproximadamente al 75% de la duración del bit
   * **Tiempo de subida/bajada**: Idealmente entre 15-300 ns (nanosegundos)
   * **Relación señal/ruido**: El ruido no debe superar los 0.5V pico a pico

4. **Anomalías comunes y su apariencia**:
   
   * **Reflexiones de señal** (problemas con resistencias terminadoras):
     * Apariencia: "Rebotes" u oscilaciones en los flancos de subida y bajada
     * Causa: Falta de resistencia terminadora o valor incorrecto
     * Diagnóstico visual: Ondas secundarias después de cada transición
     
   * **Ruido de alta frecuencia**:
     * Apariencia: Señal con "pelusa" o pequeñas oscilaciones rápidas superpuestas
     * Causa: Interferencias electromagnéticas, problemas de apantallamiento
     * Diagnóstico visual: Distorsiones aleatorias de alta frecuencia
     
   * **Niveles de voltaje incorrectos**:
     * Apariencia: La diferencia entre CAN-H y CAN-L no llega a 2V, o el nivel de reposo no está en 2.5V
     * Causa: Problemas de alimentación, módulos defectuosos, corrosión en conectores
     * Diagnóstico visual: Ondas "aplastadas" o desplazadas verticalmente
     
   * **Tráfico excesivo/saturación**:
     * Apariencia: Muy pocas transiciones al estado recesivo, dominancia casi constante
     * Causa: ECU defectuosa enviando datos continuamente
     * Diagnóstico visual: Largos períodos en estado dominante, pocas pausas
   
   * **Desalineación de señales**:
     * Apariencia: Los flancos de CAN-H y CAN-L no están perfectamente alineados
     * Causa: Diferente longitud de cables, capacitancia desigual
     * Diagnóstico visual: Una señal cambia ligeramente antes que la otra

5. **Patrones de trama CAN en el osciloscopio**:

   Una trama CAN completa tiene la siguiente estructura que puede observarse en el osciloscopio:
   
   * **Start of Frame (SOF)**: Siempre un bit dominante (0)
   * **Identificador** (11 o 29 bits): Patrón variable según el mensaje
   * **RTR**: Bit de solicitud de transmisión remota
   * **Campo de control**: Indica longitud de datos
   * **Campo de datos**: 0-8 bytes de información
   * **CRC**: Verificación de errores
   * **ACK**: Acuse de recibo (debe ser dominante si hay comunicación)
   * **End of Frame (EOF)**: 7 bits recesivos (1) consecutivos

   En un osciloscopio, una trama completa se ve como una serie de pulsos variables seguidos de un período de reposo.

6. **Comparativa de señales por velocidad**:

   La duración de los bits varía según la velocidad del bus:
   * **125 kbps**: Cada bit dura 8 µs
   * **250 kbps**: Cada bit dura 4 µs
   * **500 kbps**: Cada bit dura 2 µs
   * **1 Mbps**: Cada bit dura 1 µs
   
   Ajuste la base de tiempo del osciloscopio en consecuencia para visualizar adecuadamente la señal.

> **Para principiantes**: Al observar señales CAN en un osciloscopio, imagine que está viendo una "conversación" entre los módulos. Los bits dominantes (cuando CAN-H sube y CAN-L baja) son como alguien "hablando", mientras que los bits recesivos (cuando ambas líneas están a 2.5V) son como "silencio" o "escuchando". Una señal saludable muestra un equilibrio entre hablar y escuchar, con transiciones limpias entre ambos estados.

#### 3. Análisis con herramientas específicas
1. **Verificación de actividad en el bus con osciloscopio**:
   * **Herramienta**: Osciloscopio digital (mínimo 2 canales)
   * **Procedimiento**: 
     * Conectar canal 1 a CAN-H (pin 6 del OBD-II)
     * Conectar canal 2 a CAN-L (pin 14 del OBD-II)
     * Configurar escala de tiempo: 100 µs/división
     * Configurar escala de voltaje: 1V/división
     * Dar contacto al vehículo y observar
   * **Interpretación**:
     * Si hay actividad (ondas cuadradas opuestas) pero el escáner no lee: problema en el escáner o en el Gateway
     * Si no hay actividad: problema en la red CAN principal o ECU maestra

2. **Uso de adaptador CAN-USB para verificación de mensajes**:
   * **Herramienta**: Adaptador CAN a USB + software especializado
   * **Procedimiento**: Conectar al OBD-II y analizar los mensajes recibidos
   * **Interpretación**:
     * Si se reciben mensajes pero son incoherentes: ECU defectuosa saturando la red
     * Si no se reciben mensajes: red inactiva o Gateway aislando la red de diagnóstico

#### 4. Diagnóstico avanzado
1. **Aislamiento del problema por desconexión selectiva**:
   * **Procedimiento**: Desconectar ECUs una por una (comenzando por módulos no críticos)
   * **Precaución**: Algunos vehículos requieren procedimiento especial de desconexión
   * **Observación**: Si al desconectar una ECU se restablece la comunicación, esa unidad es probablemente la causa

2. **Reinicio completo del sistema**:
   * **Procedimiento**:
     * Desconectar batería durante 10-15 minutos
     * Presionar pedal de freno para descargar capacitores residuales
     * Reconectar batería y verificar comunicación

3. **Verificación del Gateway de comunicaciones**:
   * **Localización**: Consultar ubicación específica según modelo (generalmente cerca del conector OBD o en el BCM)
   * **Procedimiento**: Verificar alimentación y estado físico
   * **Consideración**: En algunos vehículos, el Gateway se puede "resetear" o requiere reaprendizaje

### Cuando el problema persiste

1. **Verificar protocolo correcto en el escáner**:
   * Asegurarse de que el escáner está configurado para el protocolo adecuado del vehículo
   * Probar con otro escáner de diferente marca si es posible

2. **Comprobar actualizaciones**:
   * Verificar si el escáner tiene la última actualización de software
   * Consultar si existen actualizaciones específicas para ese modelo de vehículo

3. **Considerar problemas de tensión o masa**:
   * Verificar estado de la batería (voltaje y capacidad)
   * Comprobar masas generales del vehículo, especialmente las relacionadas con la ECU principal

4. **Opción alternativa de diagnóstico**:
   * En algunos vehículos es posible acceder a modos de diagnóstico alternativos:
     * Secuencias de encendido/apagado
     * Uso de pedal acelerador en secuencias específicas
     * Lectura de códigos por parpadeo de testigos
   * Consultar manual de servicio del fabricante para procedimientos específicos

### Herramienta básica de lectura de CAN bus

Si el escáner regular no funciona, una herramienta autofabricada con Arduino puede ayudar a verificar la actividad básica del CAN bus. Esta herramienta se detalla en la sección de utilidades del proyecto:
* [Lector CAN bus Arduino](/utilities/arduino/lectorCanBus/README.md)
* [Lector CAN bus ESP32](/utilities/esp32/lectorCanBus/README.md)

### Consejo práctico

> **Para principiantes**: Antes de diagnosticar problemas complejos de red CAN, asegúrese de que el problema no sea algo tan simple como un fusible quemado o un contacto oxidado en el conector OBD. Más del 30% de los problemas de comunicación se resuelven con verificaciones básicas de alimentación y continuidad.

[Volver al índice](../diagnostico/README.md)