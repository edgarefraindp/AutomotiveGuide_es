# SENSORES DEL SISTEMA DE GESTIÓN DEL MOTOR

Los sensores del sistema de gestión del motor son fundamentales para el funcionamiento eficiente y limpio del motor. Proporcionan información esencial a la ECU (Unidad de Control del Motor) para optimizar la mezcla aire-combustible, el tiempo de encendido y otras variables críticas.

## 1.1 Sensor CKP (Sensor de Posición del Cigüeñal)

El sensor CKP (Crankshaft Position Sensor) es uno de los sensores más críticos del motor, ya que:

- **Función**: Determina la posición exacta y velocidad de rotación del cigüeñal
- **Tecnología**: Generalmente inductivo o de efecto Hall
- **Ubicación**: Normalmente instalado cerca del volante de inercia o polea del cigüeñal
- **Señal típica**: 
  - Inductivo: Señal senoidal de CA (0.5V a 100V según RPM)
  - Efecto Hall: Señal digital cuadrada (0-5V o 0-12V)
- **Síntomas de fallo**: Dificultad para arrancar, cortes de encendido, el motor se para inesperadamente
- **Diagnóstico**: Medición de resistencia (300-1500 ohms en tipo inductivo), comprobación de señal con osciloscopio

## 1.2 Sensor CMP (Sensor de Posición del Árbol de Levas)

- **Función**: Identifica la posición del árbol de levas para sincronización de la inyección y el encendido
- **Tecnología**: Principalmente efecto Hall u óptico
- **Ubicación**: Montado en el árbol de levas o en la tapa de distribución
- **Señal típica**: Digital (0-5V o 0-12V)
- **Síntomas de fallo**: Dificultad al arrancar, funcionamiento irregular, mayor consumo
- **Diagnóstico**: Comprobación de señal con osciloscopio, verificación de alimentación y masa

## 1.3 Sensores MAF/MAP (Medidor de Flujo de Aire/Presión del Múltiple)

### MAF (Mass Air Flow)
- **Función**: Mide la cantidad exacta de aire que ingresa al motor
- **Tecnología**: Hilo caliente o película caliente
- **Ubicación**: Entre el filtro de aire y el cuerpo de aceleración
- **Señal típica**: Voltaje analógico (0-5V) o frecuencia variable
- **Diagnóstico**: Verificar contaminación, señal con osciloscopio, resistencia del elemento

### MAP (Manifold Absolute Pressure)
- **Función**: Mide la presión absoluta en el colector de admisión
- **Tecnología**: Sensor piezorresistivo
- **Ubicación**: En el colector de admisión o conectado mediante manguera
- **Señal típica**: Voltaje analógico (0.5-4.5V)
- **Diagnóstico**: Verificar hermeticidad de conexiones, señal con osciloscopio

## 1.4 Sensores de Oxígeno (Sondas Lambda)

- **Función**: Miden la concentración de oxígeno en los gases de escape
- **Tipos**:
  - Convencionales (Banda estrecha)
  - De banda ancha (Wideband)
- **Ubicación**: Pre-catalizador (sensor 1) y post-catalizador (sensor 2)
- **Señal típica**: 
  - Convencional: 0.1-0.9V
  - Banda ancha: Señal modulada por corriente
- **Síntomas de fallo**: Aumento de consumo, pérdida de potencia, emisiones elevadas
- **Diagnóstico**: Verificar respuesta y tiempo de conmutación con osciloscopio

## 1.5 Sensor de Presión de Riel de Combustible

- **Función**: Mide la presión en el riel de combustible (especialmente crítico en sistemas common-rail)
- **Tecnología**: Piezorresistivo
- **Ubicación**: Montado en el riel de combustible
- **Señal típica**: 0.5-4.5V correspondiendo a presiones de 0-2000 bar
- **Síntomas de fallo**: Arranque difícil, pérdida de potencia, ruido excesivo
- **Diagnóstico**: Verificar señal con osciloscopio, comprobar resistencia interna

## 1.6 Sensor de Temperatura del Refrigerante (ECT)

- **Función**: Mide la temperatura del refrigerante del motor
- **Tecnología**: Termistor NTC (coeficiente de temperatura negativo)
- **Ubicación**: En el bloque del motor o cabeza de cilindros, en contacto con el refrigerante
- **Señal típica**: Resistencia variable (5-100K ohms en frío, 200-400 ohms en caliente)
- **Síntomas de fallo**: Ventilador funcionando constantemente, sobrecalentamiento, mayor consumo
- **Diagnóstico**: Medir resistencia a diferentes temperaturas, verificar curva característica

## 1.7 Sensor de Detonación (Knock Sensor)

- **Función**: Detecta la detonación o pistoneo en los cilindros
- **Tecnología**: Piezoeléctrico
- **Ubicación**: En el bloque del motor, entre los cilindros
- **Señal típica**: Señal de CA de baja amplitud (mV)
- **Síntomas de fallo**: Pistoneo audible, pérdida de potencia, daño potencial al motor
- **Diagnóstico**: Comprobar resistencia (50-100K ohms) y señal con osciloscopio

## 1.8 Sensor de Posición del Acelerador (TPS)

- **Función**: Mide la posición de la mariposa del acelerador
- **Tecnología**: Potenciómetro o sensor de efecto Hall
- **Ubicación**: En el cuerpo de aceleración
- **Señal típica**: Voltaje variable (0.5-4.5V)
- **Síntomas de fallo**: Ralentí irregular, aceleración brusca, consumo elevado
- **Diagnóstico**: Verificar voltaje en diferentes posiciones, comprobación de continuidad

## 1.9 Sensor de Temperatura del Aire de Admisión (IAT)

- **Función**: Mide la temperatura del aire que ingresa al motor
- **Tecnología**: Termistor NTC
- **Ubicación**: En el conducto de admisión o integrado en el sensor MAF
- **Señal típica**: Resistencia variable (5-10K ohms a temperatura ambiente)
- **Síntomas de fallo**: Mayor consumo, emisiones elevadas, pérdida de rendimiento
- **Diagnóstico**: Medir resistencia a diferentes temperaturas, verificar curva característica

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)