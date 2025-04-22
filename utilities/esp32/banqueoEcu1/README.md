> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Banqueo ECU

## Descripción General

El "banqueo" es un proceso para probar, diagnosticar y analizar una ECU (Unidad de Control Electrónico) automotriz fuera del vehículo, en un banco de trabajo. Este proyecto utiliza un ESP32 para crear un sistema capaz de emular las señales de entrada que la ECU espera recibir y monitorear las señales que la ECU genera como respuesta, permitiendo diagnosticar fallas en la unidad de control.

## ¿Para qué sirve?

El banqueo de ECU permite:

- Determinar si una ECU está funcionando correctamente antes de reemplazarla
- Identificar fallas específicas en los circuitos internos de la ECU
- Comprender el funcionamiento y lógica de operación de una ECU específica
- Diagnosticar ECUs cuando no se cuenta con información técnica del fabricante
- Crear un mapa de conexiones (pinout) de la ECU cuando no se dispone de esta información

## Proceso de Banqueo Completo

### Fase 1: Identificación y mapeo de pines (Proceso Manual)

Antes de utilizar este proyecto con el ESP32, se debe realizar un proceso inicial de identificación:

1. **Identificación visual**:
   - Identificar los conectores y pines de la ECU
   - Documentar la distribución de pines con fotografías o diagramas
   - Buscar información técnica de la ECU (si está disponible), tal como esquemas eléctricos, diagramas de pines, etc.
   - Buscar marcas, etiquetas o códigos que puedan indicar la función de cada pin

2. **Mediciones básicas con multímetro**:
   - Identificar pines de alimentación y tierra (normalmente los de mayor grosor)
   - Medir resistencia entre pines para identificar circuitos internos
   - Buscar pines conectados directamente a tierra o alimentación

3. **Análisis con osciloscopio**:
   - Conectar la ECU a alimentación adecuada (12V para la mayoría de ECUs automotrices)
   - Configurar el osciloscopio para mediciones de baja frecuencia (inicialmente)
   - Realizar un barrido sistemático de cada pin mientras se activa la ECU
   
4. **Identificación de señales principales**:
   - **Señales de reloj**: Señales cuadradas de frecuencia constante (típicamente entre 8-40 MHz)
   - **Líneas de comunicación**: Señales de datos con patrones regulares (CAN: 2.5V línea dominante/recesiva)
   - **Entradas de sensores**: Señales variables de bajo voltaje (0-5V)
   - **Salidas de actuadores**: Pulsos de potencia (señales PWM a inyectores, bobinas, etc.)
   - **Señales de referencia**: Voltajes constantes (12v ref, 5V ref, 3.3V ref)

5. **Documentación de resultados**:
   - Crear una tabla o diagrama con la función aparente de cada pin
   - Registrar formas de onda de cada señal identificada
   - Anotar frecuencias y amplitudes de las señales importantes

### Fase 2: Configuración del ESP32 para banqueo

Una vez identificadas las señales principales o partiendo de un esquema de pines oficial, se puede proceder a la configuración del ESP32 para emular las señales de entrada y monitorear las salidas de la ECU.

1. **Emulación de sensores**:
   - Generar señales que emulan a los sensores principales (MAF, MAP, TPS, CKP, etc.)
   - Variar estas señales según patrones predefinidos de funcionamiento normal
   
2. **Monitoreo de salidas**:
   - Capturar y analizar señales de activación de actuadores (inyectores, bobinas)
   - Verificar los tiempos de respuesta y patrones de activación
   
3. **Detección de anomalías**:
   - Comparar las señales de salida con patrones esperados
   - Detectar ausencia de señales en actuadores
   - Identificar fluctuaciones anormales o inconsistencias

## Componentes necesarios

- Placa ESP32
- Osciloscopio de al menos 2 canales (preferiblemente 4)
- Multímetro digital
- Fuente de alimentación variable (0-15V) o una batería de automóvil
- Cables de conexión (jumper wires)
- Protoboard o placa de pruebas
- Conectores hembra/macho para la ECU
- Cables y conectores para la ECU específica
- Resistencias, capacitores y otros componentes auxiliares según ECU
- Adaptadores de nivel lógico (si es necesario)

## Configuración del Hardware

| Función | Pin ESP32 | Descripción |
|---------|-----------|-------------|
| Emulación CKP | GPIO 16 | Señal del sensor de posición del cigüeñal |
| Emulación CMP | GPIO 17 | Señal del sensor de posición del árbol de levas |
| Emulación MAP | GPIO 18 | Señal del sensor de presión absoluta del múltiple |
| Emulación TPS | GPIO 19 | Señal del sensor de posición del acelerador |
| Emulación MAF | GPIO 21 | Señal del sensor de flujo de masa de aire |
| Emulación ECT | GPIO 22 | Señal del sensor de temperatura del refrigerante |
| Emulación IAT | GPIO 23 | Señal del sensor de temperatura del aire |
| Monitor Inyector 1 | GPIO 25 | Monitoreo del pulso del inyector 1 |
| Monitor Inyector 2 | GPIO 26 | Monitoreo del pulso del inyector 2 |
| Monitor Inyector 3 | GPIO 27 | Monitoreo del pulso del inyector 3 |
| Monitor Inyector 4 | GPIO 32 | Monitoreo del pulso del inyector 4 |
| Monitor Bobina | GPIO 33 | Monitoreo del pulso de la bobina de ignición |
| Control Bomba | GPIO 34 | Control de la bomba de combustible |
| Comunicación | GPIO 35 | Línea de comunicación (K-Line, CAN, etc.) |

## Uso del Sistema

1. **Configuración inicial**:
   - Conectar las salidas del ESP32 a los pines correspondientes de la ECU
   - Asegurar que la ECU tenga alimentación adecuada
   - Iniciar el ESP32 con el programa de este proyecto

2. **Emulación de señales**:
   - El sistema comenzará generando señales básicas que emulan el motor en ralentí
   - A través de la interfaz serie, se pueden ajustar los parámetros de las señales
   - Emular diferentes condiciones de operación (aceleración, carga, etc.)

3. **Monitoreo y diagnóstico**:
   - Observar las respuestas de la ECU a las señales emuladas
   - Verificar que los pulsos de inyectores cambien adecuadamente con la aceleración
   - Confirmar que los tiempos de ignición se modifiquen según la carga simulada
   - Detectar anomalías en la sincronización o duración de los pulsos

## En palabras sencillas

Este sistema funciona como un "simulador" que engaña a la computadora del vehículo haciéndole creer que está conectada a un motor real. Le enviamos señales falsas que imitan a los sensores (como si le estuviéramos diciendo "el motor está frío" o "el acelerador está a la mitad") y observamos cómo responde la computadora. Si la computadora no responde correctamente (por ejemplo, no activa un inyector cuando debería), podemos determinar que hay un problema en esa parte específica de la ECU.

## Tipos de anomalías detectables

- **Ausencia de pulsos**: Cuando un inyector o bobina no recibe señal de activación
- **Pulsos irregulares**: Cuando la duración o frecuencia de los pulsos no corresponde con lo esperado
- **Sin respuesta a cambios**: Cuando la ECU no modifica las señales de salida al variar las entradas
- **Comunicación defectuosa**: Cuando los protocolos de comunicación no funcionan correctamente
- **Señales de referencia incorrectas**: Voltajes de referencia ausentes o fuera de rango

## Consejo práctico

Es recomendable comenzar con un "perfil" básico de funcionamiento, como un motor en ralentí, antes de simular condiciones más complejas. Esto permite establecer una línea base de comportamiento esperado y facilita la identificación de anomalías cuando se emulan condiciones más exigentes.

## Precauciones ⚠️

- Verificar siempre la polaridad y niveles de voltaje antes de conectar la ECU
- Algunas ECUs tienen protecciones que se activan si no detectan ciertos sensores críticos
- Utilizar adaptadores de nivel lógico cuando sea necesario (la mayoría de ECUs operan con señales de 0-5V o 0-12V, mientras que el ESP32 utiliza 0-3.3V)
- Tener especial cuidado con las conexiones a señales de alta frecuencia como CKP/CMP

## Para principiantes

Si es la primera vez que realiza un banqueo de ECU, comience con modelos más antiguos y simples (como sistemas de inyección monopunto) antes de intentar con ECUs modernas que controlan múltiples sistemas. Las ECUs más antiguas tienen lógica más sencilla y son más fáciles de diagnosticar con este método.

## Diagnóstico de Fallas a través del Banqueo

El proceso de banqueo no solo permite verificar si una ECU funciona, sino que también ayuda a diagnosticar problemas específicos. A continuación, se presenta una guía de interpretación de resultados y posibles diagnósticos.

### Interpretación de Resultados y Tabla de Diagnóstico

| Comportamiento Observado | Posible Falla | Diagnóstico | Solución Recomendada |
|--------------------------|---------------|-------------|------------------------|
| No hay respuesta en ninguna salida | Falla general de alimentación | La ECU no está recibiendo alimentación adecuada o tiene un problema en su circuito de potencia principal | 1. Revisar fusibles internos de la ECU<br>2. Verificar reguladores de voltaje internos<br>3. Comprobar capacitores de filtrado |
| La ECU no responde a señal CKP/CMP | Falla en circuito de sincronización | El circuito que procesa las señales de sincronización está dañado | 1. Revisar componentes del circuito de entrada de señales CKP/CMP<br>2. Verificar el estado de amplificadores operacionales o comparadores |
| Algunos inyectores no pulsan | Falla en etapa de potencia parcial | Transistores o circuitos de potencia dañados para canales específicos | 1. Reemplazar transistores o circuitos integrados de potencia<br>2. Verificar resistencias limitadoras de corriente |
| Todos los inyectores pulsan pero con tiempos irregulares | Falla en procesador o memoria | El microprocesador funciona pero no calcula correctamente los tiempos | 1. Verificar las memorias EEPROM/FLASH<br>2. Comprobar si hay soldaduras frías en el procesador<br>3. Considerar reprogramación si es posible |
| No hay respuesta a cambios en TPS | Falla en circuito de entrada analógica | El circuito de acondicionamiento de señal del TPS está dañado | 1. Verificar amplificadores operacionales del circuito de entrada<br>2. Comprobar resistencias de polarización<br>3. Revisar convertidores A/D |
| Pulsos de inyectores no varían con temperatura | Falla en circuito de entrada de sensores ECT/IAT | La ECU no está procesando correctamente las señales de temperatura | 1. Verificar circuito de acondicionamiento para sensores de temperatura<br>2. Comprobar circuito de referencia de voltaje |
| Respuesta lenta o retrasada a cambios | Falla en el procesador | El procesador está funcionando a velocidad reducida o con problemas | 1. Verificar circuito de reloj/cristal<br>2. Comprobar capacitores de desacoplo<br>3. Verificar voltajes de alimentación del procesador |
| La ECU se reinicia durante operación | Falla en sistema de reset/watchdog | Problemas con el circuito de supervisión o de alimentación | 1. Verificar circuito de reset<br>2. Comprobar capacitores de filtrado<br>3. Revisar reguladores de voltaje |
| Comunicación diagnóstica no funciona | Falla en transceiver de comunicación | El circuito que maneja la comunicación diagnóstica está dañado | 1. Verificar circuitos integrados de comunicación<br>2. Comprobar resistencias terminadoras en líneas CAN<br>3. Revisar transistores de interfaz |

### Proceso de Prueba Sistemático

Para realizar un diagnóstico efectivo mediante banqueo, siga este proceso sistemático:

1. **Pruebas de alimentación básicas**:
   - Medir consumo de corriente en reposo y durante operación
   - Verificar estabilidad del voltaje en diferentes puntos de prueba
   - Observar comportamiento durante arranque (cold start)
   
2. **Pruebas de procesamiento central**:
   - Comprobar reacción a señal CKP/CMP (sincronización)
   - Verificar secuencia de arranque y establecimiento de ralentí
   - Observar transición entre diferentes modos de operación
   
3. **Pruebas de entradas analógicas**:
   - Variar señal de TPS y observar respuesta en tiempo de inyección
   - Modificar señales de temperatura y verificar enriquecimiento
   - Cambiar señal MAP y observar adaptación de pulsos de inyección
   
4. **Pruebas de salidas**:
   - Medir amplitud y forma de los pulsos de inyectores
   - Verificar sincronización entre inyectores y señal de encendido
   - Comprobar activación de relés auxiliares y bomba de combustible
   
5. **Pruebas de comunicación**:
   - Intentar establecer comunicación diagnóstica
   - Verificar lectura de códigos de falla almacenados
   - Comprobar respuesta a comandos de diagnóstico específicos

### Patrones de Falla Comunes y su Significado

#### 1. Fallas en Inyectores:

**Normal**: En una ECU funcional, los pulsos de inyección deberían:
- Ser proporcionales a la carga del motor (TPS, MAP)
- Tener un ancho de pulso entre 1.5 a 15ms dependiendo de la carga
- Sincronizarse precisamente con el ciclo del motor (señal CKP)
- Mostrar formas de onda limpias con flancos bien definidos

**Anomalías y su significado**:
- **Pulsos ausentes**: Etapa de potencia dañada, controlador específico dañado.
- **Flancos de subida lentos**: Problemas en transistores de potencia o resistencias pull-up.
- **Duración errática**: Problema en el procesador o en el temporizador interno.
- **Amplitud reducida**: Problemas en la etapa de potencia o fuente de alimentación.

#### 2. Fallas en la Bobina de Ignición:

**Normal**: La señal de activación de bobina debería:
- Tener un tiempo de carga (dwell) inversamente proporcional a RPM
- Sincronizarse perfectamente con la posición del cigüeñal
- Mostrar tiempos de dwell entre 2 a 5ms típicamente
- Tener un disparo limpio y preciso

**Anomalías y su significado**:
- **No hay señal**: Circuito de control de ignición dañado.
- **Timing incorrecto**: Falla en microprocesador o circuito de sincronización.
- **Dwell no variable**: Falla en la lógica de control adaptativo de dwell.
- **Ruido en la señal**: Problemas en los componentes de supresión de ruido.

#### 3. Fallas en Procesamiento de Sensores:

**Normal**: La ECU debería:
- Responder inmediatamente a cambios en las señales de entrada
- Ajustar proporcionalmente los pulsos según los valores de sensores
- Mostrar enriquecimiento específico durante arranque en frío
- Seguir una lógica de control coherente con el mapa de la ECU

**Anomalías y su significado**:
- **No responde a TPS**: Circuito de entrada analógica dañado o conversor A/D defectuoso.
- **No responde a temperatura**: Circuito de acondicionamiento para NTC dañado.
- **Responde a algunos sensores pero no a otros**: Canales específicos del multiplexor o conversor A/D dañados.
- **Respuesta extremadamente lenta**: Problemas en el microprocesador o cristal de reloj.

### Solución de Problemas Específicos

#### Error al Emular Sensores
Si encuentra dificultades al emular señales de sensores:

1. **Problema**: La ECU detecta sensor fuera de rango o implausible
   - **Solución**: Verificar que las señales emuladas estén dentro de los rangos esperados. Algunas ECUs tienen verificación de plausibilidad que rechaza valores fuera de rangos predefinidos.

2. **Problema**: La ECU no responde a la señal emulada
   - **Solución**: Verificar niveles de voltaje y ajustarlos a los esperados por la ECU. Algunas ECUs requieren referencias de voltaje muy precisas.

3. **Problema**: La señal CKP no es reconocida
   - **Solución**: Verificar la polaridad, amplitud y patrón exacto de dientes. Muchas ECUs son extremadamente sensibles a irregularidades en esta señal.

#### Problemas con el Monitoreo de Actuadores

1. **Problema**: Falsos disparos o lecturas erróneas en los actuadores
   - **Solución**: Agregar filtrado de señal adicional. Verificar aislamiento eléctrico entre circuitos.

2. **Problema**: No se detectan pulsos cortos
   - **Solución**: Ajustar la configuración de captura para eventos de menor duración y aumentar la frecuencia de muestreo.

3. **Problema**: Interferencia entre canales de monitoreo
   - **Solución**: Mejorar aislamiento entre canales, agregar optoacopladores adicionales para cada canal monitoreado.

### Interpretación Avanzada de Señales

Para técnicos con experiencia en el uso de osciloscopios, estos son los patrones que debe buscar:

1. **Señales de inyectores sanos**:
   - Flanco de subida rápido (<5μs)
   - Flanco de bajada limpio con posible pico inductivo controlado
   - Ancho de pulso coherente entre cilindros del mismo banco
   - Voltaje alto de 12-14V y bajo de <0.7V

2. **Señales de bobina sanas**:
   - Tiempo de dwell constante para RPM constante
   - Flancos limpios con picos inductivos controlados
   - Sincronización precisa respecto a señal CKP/CMP
   - Voltajes de control dentro de especificación

3. **Comunicaciones**:
   - Señales CAN con dominantes y recesivos claramente definidos
   - Sin ruido o glitches en las señales de comunicación
   - Velocidad de comunicación estable y correcta

### Casos Especiales

#### ECUs con Inmovilizador

Para ECUs con sistemas de inmovilizador integrado:
- Es posible que algunas funciones estén limitadas si el inmovilizador no está satisfecho
- Pueden requerir emulación de señales de transponder o bypass del sistema
- Generalmente el motor arrancará pero se detendrá después de algunos segundos

#### Sistemas Multiplexados

Para vehículos con sistemas altamente multiplexados:
- La ECU puede requerir comunicación con otros módulos para funcionar correctamente
- Es posible que sea necesario emular señales CAN de otros módulos
- Pueden existir tiempos de espera si no se detectan otros módulos en la red

#### ECUs de Última Generación

Las ECUs más recientes pueden incluir:
- Sistemas de protección contra manipulaciones
- Algoritmos de diagnóstico avanzados que detectan emulación
- Requerimientos de señales más precisas y con verificaciones cruzadas

### Extensiones para el Sistema de Banqueo

Para mejorar el sistema actual, considere estas adiciones:
- Amplificadores para aumentar los niveles de señal a 0-12V donde sea necesario
- Redes resistivas para emular sensores resistivos como NTC con mayor precisión
- Filtros para las señales de entrada y salida reduciendo ruido y falsas lecturas
- Circuitos de protección adicionales para evitar daños a la ECU o al ESP32
- Adaptadores específicos para diferentes marcas y modelos de ECUs