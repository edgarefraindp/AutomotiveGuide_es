# SENSORES Y ACTUADORES DE LA CABINA

Los sensores y actuadores de la cabina son responsables del confort, la comodidad y la seguridad de los ocupantes. En vehículos modernos, estos sistemas son cada vez más sofisticados y ofrecen mayor nivel de personalización y automatización.

## 6.1 Sensor Anti-atrapamiento de Ventanillas

- **Función**: Detectar obstáculos durante el cierre de ventanillas para prevenir lesiones
- **Tecnología**: 
  - Sensor de Hall para medir variaciones de corriente en el motor
  - Sensores capacitivos (tecnología más reciente)
- **Ubicación**: Integrado en el mecanismo elevalunas o en la unidad de control de puerta
- **Funcionamiento**: 
  - Detecta cambios en la resistencia al movimiento
  - Invierte el movimiento al detectar un obstáculo
- **Características**:
  - Sensibilidad ajustable
  - Memorización de posiciones (función one-touch)
- **Síntomas de fallo**:
  - Detenciones e inversiones sin motivo
  - No detecta obstáculos
  - Función one-touch no funciona
- **Diagnóstico**:
  - Reinicialización del sistema (procedimiento específico)
  - Comprobación de valores con escáner
  - Test de funcionamiento

## 6.2 Sensores de Ocupación de Asientos

- **Función**: Detectar la presencia y clasificar al ocupante para control de airbags
- **Tecnología**: 
  - Sensores de presión
  - Sensores capacitivos
  - Sensores ultrasónicos (sistemas avanzados)
- **Ubicación**: Bajo el acolchado del asiento
- **Funciones avanzadas**:
  - Clasificación de ocupante (adulto, niño, objeto)
  - Detección de posición incorrecta (OOP - Out Of Position)
- **Aplicaciones**:
  - Control adaptativo de despliegue de airbags
  - Alerta de cinturón desabrochado
  - Memorización de posiciones de asiento
- **Síntomas de fallo**:
  - Testigo de airbag encendido
  - Falsas alertas de cinturón
  - Desactivación permanente de airbag
- **Diagnóstico**:
  - Lectura de valores con escáner
  - Verificación de calibración
  - Comprobación de conectores (alta sensibilidad)

## 6.3 Sensores de Calidad del Aire

- **Función**: Medir la calidad del aire para control automático de climatización
- **Tecnología**: 
  - Sensores de compuestos orgánicos volátiles (VOC)
  - Sensores de partículas (PM2.5, PM10)
  - Sensores de CO2 (en vehículos premium)
- **Ubicación**: En el conducto de entrada de aire o en el panel de control de clima
- **Aplicaciones**:
  - Activación automática de recirculación
  - Control de filtros de aire ionizadores
  - Alertas de calidad de aire deficiente
- **Síntomas de fallo**:
  - Activación innecesaria de recirculación
  - No activación cuando es necesario
  - Mensajes de error en pantalla
- **Diagnóstico**:
  - Verificación de valores con escáner
  - Comprobación de circuito eléctrico
  - Test de respuesta a contaminantes

## 6.4 Sensores de Lluvia y Luz

- **Función**: Automatizar la activación de limpiaparabrisas y luces
- **Tecnología**: 
  - Sensor óptico con emisor y receptor de infrarrojos (lluvia)
  - Fotodiodo (luz)
- **Ubicación**: En la base del espejo retrovisor, tras el parabrisas
- **Características**:
  - Sensibilidad ajustable
  - Integración con sistema de luces automáticas
  - En algunos casos, detección de túneles
- **Síntomas de fallo**:
  - Activación intermitente sin causa
  - No activación cuando llueve
  - Retraso excesivo en la respuesta
- **Diagnóstico**:
  - Verificar limpieza del parabrisas en zona del sensor
  - Comprobar parámetros con escáner
  - Test de funcionalidad simulando condiciones

## 6.5 Actuadores de Climatización Automática

- **Componentes principales**:
  - Motores para compuertas de distribución
  - Motores para control de temperatura (mezcla)
  - Actuadores para control de velocidad del ventilador
- **Tecnología**: 
  - Motores paso a paso
  - Servomotores con potenciómetro de retroalimentación
- **Ubicación**: 
  - Dentro del tablero, integrados en la caja de climatización
  - Bajo el tablero, accesibles desde el hueco de pies
- **Síntomas de fallo**:
  - Modos de distribución no cambian
  - Temperatura incorrecta/inconsistente
  - Ruidos al cambiar modos
- **Diagnóstico**:
  - Test de actuación con escáner
  - Comprobación de valores de potenciómetros
  - Verificación de alimentación y control

## 6.6 Espejos con Memoria y Ajuste Automático

- **Función**: Ajustar automáticamente la posición de espejos según preferencias del conductor
- **Componentes**:
  - Motores de posicionamiento
  - Sensores de posición (potenciómetros)
  - Elementos calefactores
  - Sensores de luz para atenuación automática
- **Funciones avanzadas**:
  - Inclinación al dar marcha atrás
  - Plegado automático al bloquear
  - Atenuación automática antideslumbramiento
- **Síntomas de fallo**:
  - No memoriza posiciones
  - Movimiento irregular o ruidoso
  - No vuelve a posición memorizada
- **Diagnóstico**:
  - Comprobación de comunicación CAN/LIN
  - Verificación de valores de memoria con escáner
  - Test de funcionamiento de motores

## Integración y Comunicación

Los sensores y actuadores de la cabina están típicamente interconectados a través de:
- Bus CAN Confort/Carrocería (velocidad baja)
- Bus LIN (comunicación económica para funciones simples)
- Bus MOST (para sistemas multimedia en vehículos premium)

Esta interconexión permite funciones avanzadas como:
- Personalización por conductor
- Sincronización entre sistemas
- Diagnóstico centralizado
- Actualizaciones remotas (OTA)

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)