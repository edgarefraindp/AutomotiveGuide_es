# SENSORES Y ACTUADORES DEL SISTEMA DE FRENOS

Los sistemas de frenos modernos incorporan numerosos sensores y actuadores que permiten funciones avanzadas como ABS, ESP, control de tracción y asistencia al frenado de emergencia.

## 5.1 Sensor ABS (Velocidad de Ruedas)

- **Función**: Medir la velocidad de rotación de cada rueda
- **Tecnología**: 
  - Pasivo (inductivo): El más común hasta 2010
  - Activo (efecto Hall o magnetorresistivo): Estándar desde 2010
- **Ubicación**: En cada buje de rueda o integrado en el rodamiento
- **Señal típica**:
  - Pasivo: Señal senoidal de CA (frecuencia y amplitud proporcionales a la velocidad)
  - Activo: Señal digital cuadrada (requiere alimentación)
- **Características**:
  - Sensor activo: Detección de velocidad cero y dirección de rotación
  - Resolución: 48-100 dientes en la rueda fónica
- **Síntomas de fallo**:
  - Luz de ABS/ESP encendida
  - Desactivación de sistemas de asistencia
  - Mensajes de error específicos
- **Diagnóstico**:
  - Comprobación visual (cables, conectores)
  - Medición de resistencia (pasivo: 800-2000 ohms)
  - Verificación de señal con osciloscopio

## 5.2 Sensor de Posición del Pedal de Freno

- **Función**: Detectar la posición y velocidad de accionamiento del pedal
- **Tecnología**: 
  - Interruptor doble (redundante)
  - Potenciómetro o sensor Hall (en sistemas avanzados)
- **Ubicación**: En el conjunto del pedal de freno
- **Señal típica**:
  - Interruptor: 0V/12V (abierto/cerrado)
  - Potenciómetro: 0.5-4.5V variable
- **Aplicaciones**:
  - Activación de luces de freno
  - Desactivación de control de crucero
  - Función de retención automática (Auto Hold)
  - Asistencia al frenado de emergencia
- **Diagnóstico**:
  - Verificar continuidad en diferentes posiciones
  - Comprobar tensión de señal
  - Test de actuación con escáner

## 5.3 Sensor de Desgaste de Pastillas

- **Función**: Alertar cuando las pastillas de freno alcanzan un límite de desgaste
- **Tecnología**: 
  - Contacto eléctrico (cable integrado que se rompe al alcanzar el límite)
  - Sensores capacitivos (en sistemas más avanzados)
- **Ubicación**: Integrado en las pastillas de freno
- **Señal típica**: Circuito abierto/cerrado (0V/12V)
- **Características**:
  - Algunos vehículos modernos calculan el desgaste basándose en uso y estilo de conducción
- **Síntomas de fallo**:
  - Falsa indicación de desgaste
  - No indica desgaste cuando debería
- **Diagnóstico**:
  - Comprobación de continuidad
  - Verificación de testigo en cuadro de instrumentos

## 5.4 Actuador de Freno Electrónico de Estacionamiento

- **Función**: Accionar/liberar el freno de estacionamiento electrónicamente
- **Tecnología**: 
  - Motor eléctrico con reductor
  - Actuador electromecánico
- **Tipos**:
  - Integrado en pinza (cada rueda trasera tiene su motor)
  - Centralizado (un motor actúa sobre cables)
- **Ubicación**: En pinzas traseras o centralizado en túnel de transmisión
- **Características**:
  - Función Auto-Hold (retención automática)
  - Liberación automática al iniciar marcha
  - Aplicación automática al apagar el motor
- **Síntomas de fallo**:
  - Mensaje de error en cuadro de instrumentos
  - Imposibilidad de liberar/aplicar freno
  - Ruidos anormales durante operación
- **Diagnóstico**:
  - Comprobación de alimentación y comunicación
  - Test de actuación con escáner
  - Procedimientos de emergencia (liberación manual)

## 5.5 Unidad Hidráulica de Control ABS/ESP

- **Función**: Modular la presión hidráulica en cada circuito de freno
- **Componentes**:
  - Bomba hidráulica eléctrica
  - Electroválvulas moduladoras (8-12)
  - Acumulador de presión
  - Sensores de presión integrados
- **Ubicación**: Vano motor, conectada al cilindro maestro
- **Características**:
  - Integra las funciones de ABS, ESP, asistencia al frenado
  - Controla sistemas de frenado automático de emergencia
- **Síntomas de fallo**:
  - Pedal duro o esponjoso
  - Ruidos anormales durante funcionamiento ABS
  - Luces de avería de sistemas de asistencia
- **Diagnóstico**:
  - Lectura de DTC con escáner
  - Pruebas de actuación de válvulas
  - Sangrado específico (generalmente requiere escáner)

## 5.6 Sensor de Ángulo de Dirección

- **Función**: Medir el ángulo de giro del volante
- **Tecnología**: Encoder óptico o magnético
- **Ubicación**: En la columna de dirección, bajo el volante
- **Señal típica**: Digital, comunicación por CAN-BUS
- **Características**:
  - Alta resolución (0.1 grados o mejor)
  - Detección de posición absoluta
  - Detección de velocidad de giro
- **Aplicaciones**:
  - Control de estabilidad (ESP)
  - Dirección asistida eléctrica variable
  - Sistemas de asistencia de carril
- **Diagnóstico**:
  - Requiere calibración tras sustitución
  - Lectura de valores con escáner
  - Comprobación de comunicación CAN

## Integración de Sistemas

Los sensores y actuadores del sistema de frenos están altamente integrados con otros sistemas del vehículo:
- Sistemas ADAS (frenado autónomo de emergencia)
- Control de crucero adaptativo
- Asistentes de aparcamiento
- Sistemas de conducción semi-autónoma

Esta integración requiere que el diagnóstico y reparación considere siempre el sistema como un conjunto, verificando todas las interfaces y comunicaciones entre los diferentes módulos.

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)