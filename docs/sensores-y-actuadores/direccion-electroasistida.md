# SISTEMAS DE DIRECCIÓN ELECTROASISTIDA

Los sistemas de dirección electroasistida (EPS - Electric Power Steering) han reemplazado casi completamente a los sistemas hidráulicos tradicionales en vehículos modernos, ofreciendo mayor eficiencia energética, adaptabilidad y la posibilidad de integración con sistemas ADAS.

## 7.1 Motor de Asistencia Eléctrica

- **Función**: Proporcionar asistencia a la dirección mediante fuerza eléctrica
- **Tecnología**: 
  - Motor DC con escobillas (sistemas más antiguos)
  - Motor DC sin escobillas (BLDC, estándar actual)
  - Motor síncrono de imanes permanentes (premium)
- **Configuraciones**:
  - Columna (C-EPS): Motor montado en la columna de dirección
  - Piñón (P-EPS): Motor asiste al piñón de la dirección
  - Cremallera (R-EPS): Motor acoplado directamente a la cremallera
- **Características**:
  - Eficiencia: 85-95% (vs. 50-60% en sistema hidráulico)
  - Potencia típica: 500W-1200W
  - Voltaje: 12V o 48V (sistema eléctrico reforzado)
- **Síntomas de fallo**:
  - Pérdida de asistencia (dirección pesada)
  - Asistencia intermitente
  - Ruidos anormales
  - Luz de avería encendida
- **Diagnóstico**:
  - Verificación de códigos de error con escáner
  - Medición de consumo de corriente
  - Comprobación de conexiones y alimentación

## 7.2 Sensor de Par de Dirección

- **Función**: Medir la fuerza y dirección aplicada por el conductor al volante
- **Tecnología**: 
  - Potenciómetros redundantes
  - Sensores magnetorresistivos
  - Sensores inductivos RVDT
- **Ubicación**: En la columna de dirección o integrado en la caja de dirección
- **Señal típica**: Dual (redundante) analógica/digital
- **Características**:
  - Alta precisión (0.01-0.1 Nm)
  - Detección de dirección del par
  - Respuesta rápida (<10ms)
- **Síntomas de fallo**:
  - Activación de modo seguro (reducción de asistencia)
  - Asistencia inconsistente
  - Retorno anormal del volante
- **Diagnóstico**:
  - Verificación de valores con escáner
  - Comprobación de calibración
  - Lectura de señales con osciloscopio (cuando es posible)

## 7.3 Unidad de Control de Dirección Asistida

- **Función**: Gestionar la asistencia en función de múltiples parámetros
- **Entradas**:
  - Sensor de par
  - Velocidad del vehículo
  - Régimen del motor
  - Estado del ESC/ESP
  - Sensor de ángulo de dirección
- **Características**:
  - Ajuste adaptativo de la asistencia según velocidad
  - Compensación de deriva (viento lateral, peralte)
  - Amortiguación de impactos del camino
  - Interfaz con sistemas ADAS
- **Ubicación**: Incorporada al conjunto del motor o montada separadamente
- **Síntomas de fallo**:
  - Códigos de error múltiples
  - Comportamiento errático de la asistencia
  - Pérdida de funciones específicas
- **Diagnóstico**:
  - Lectura de DTC con escáner específico
  - Verificación de comunicación CAN
  - Actualizaciones de software (cuando aplique)

## 7.4 Dirección Asistida Variable

- **Función**: Adaptar el nivel de asistencia y la relación de dirección según condiciones
- **Tecnologías**:
  - Desmultiplicación variable mecánica
  - Desmultiplicación variable electrónica
  - Control activo de par asistido
- **Características avanzadas**:
  - Dirección activa en las 4 ruedas
  - Cancelación activa del subviraje/sobreviraje
  - Integración con sistemas de mantenimiento de carril
- **Aplicaciones específicas**:
  - Maniobras a baja velocidad (estacionamiento)
  - Conducción deportiva
  - Asistencia en situaciones de emergencia
- **Síntomas de fallo**:
  - Comportamiento inconsistente de la dirección
  - No cambia entre modos (normal/sport/confort)
  - Funcionamiento en modo degradado
- **Diagnóstico**:
  - Verificación de parámetros específicos
  - Test de actuación con escáner
  - Calibración tras intervenciones

## Calibración y Procedimientos Específicos

Los sistemas de dirección electroasistida modernos requieren procedimientos específicos de:

1. **Inicialización tras desconexión de batería**:
   - Típicamente involucra girar el volante de tope a tope
   - Mantener el vehículo en reposo con el motor en marcha
   - Procedimiento específico con escáner en algunos modelos

2. **Calibración tras sustitución de componentes**:
   - Calibración del punto central
   - Aprendizaje de límites de recorrido
   - Adaptación sensor de par
   - Programación/codificación de la ECU

## Integración con Sistemas de Conducción Autónoma

En vehículos más avanzados (2020+), el sistema EPS está integrado con:
- Asistente de mantenimiento de carril
- Asistente de aparcamiento
- Control de crucero adaptativo
- Conducción semi-autónoma

Esta integración requiere calibraciones precisas y compatibilidad total entre componentes, siendo generalmente necesario el uso de herramientas de diagnóstico de nivel concesionario para intervenciones mayores.

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)