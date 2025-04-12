# PEDAL DE ACELERADOR ELECTRÓNICO (APPS/ETC)

El sistema de pedal de acelerador electrónico (también conocido como "Drive By Wire") ha reemplazado completamente los tradicionales cables mecánicos en los vehículos modernos.

## Componentes Principales

### Sensor de Posición del Pedal del Acelerador (APPS)

- **Función**: Detecta la posición y velocidad de movimiento del pedal del acelerador
- **Tecnología**: Doble potenciómetro o sensor de efecto Hall (redundante por seguridad)
- **Ubicación**: Integrado en el conjunto del pedal
- **Señal típica**: Dos señales analógicas de 0.5-4.5V con correlación diferente (por ejemplo, señal 2 = mitad de señal 1)
- **Alimentación**: 5V referencia
- **Síntomas de fallo**: Modo de emergencia (limp mode), restricción de RPM, aceleración limitada
- **Diagnóstico**: 
  - Medición de resistencia en diferentes posiciones del pedal
  - Comprobación de señales con osciloscopio
  - Verificación de correlación entre ambos potenciómetros

### Cuerpo de Aceleración Electrónico

- **Función**: Controla el flujo de aire hacia el motor según las órdenes de la ECU
- **Componentes**:
  - Motor eléctrico DC o paso a paso
  - Sensor de posición de la mariposa (TPS, redundante)
  - Resorte de retorno (posición de seguridad)
- **Ubicación**: En el colector de admisión
- **Señales típicas**:
  - TPS: Doble señal analógica 0.5-4.5V
  - Control del motor: PWM (modulación por ancho de pulso)
- **Diagnóstico**:
  - Comprobación de resistencia del motor
  - Verificación de señales TPS
  - Test de actuación mediante escáner

## Modo de Funcionamiento

1. El conductor pisa el pedal del acelerador
2. El sensor APPS envía la información de posición a la ECU
3. La ECU procesa esta información junto con otras variables (velocidad, temperatura, etc.)
4. La ECU envía la señal al cuerpo de aceleración para abrir la mariposa según lo requerido
5. El TPS del cuerpo de aceleración confirma la posición real de la mariposa

## Ventajas del Sistema Drive-by-Wire

- Control preciso de la dosificación de combustible
- Mejor respuesta y adaptabilidad a diferentes condiciones
- Integración con control de crucero, control de tracción y sistemas de estabilidad
- Menor emisión de contaminantes
- Modos configurables de respuesta (eco, normal, sport)

## Procedimientos de Adaptación

Muchos vehículos requieren un procedimiento de adaptación después de la sustitución del cuerpo de aceleración o la desconexión de la batería:

1. Llave en contacto sin arrancar por 30 segundos
2. Arrancar el motor y dejarlo en ralentí 2-3 minutos
3. Apagar y repetir si es necesario
4. En algunos casos se requiere adaptación con escáner diagnóstico

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)