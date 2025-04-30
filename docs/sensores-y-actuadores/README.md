> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Documentación - Sensores y Actuadores

Esta sección detalla el funcionamiento, diagnóstico y características de los principales sensores y actuadores encontrados en vehículos modernos (2020 en adelante).

## Contenido Principal

- **Introducción**: Visión general de los sistemas electrónicos.
- **Sensores Principales**: Descripción general de los sensores clave.
- **Sensores de Motor**: MAP, MAF, TPS, CKP, CMP, ECT, IAT, O2, Knock.
- **Sistema de Aceleración Electrónica**: Funcionamiento y diagnóstico del cuerpo de aceleración electrónico.
- **Sensores de Emisiones**: EGR, EVAP, Sensores de NOx, Filtro de partículas.
- **Sistemas ADAS**: Sensores de radar, lidar, cámaras.
- **Sensores de Frenos**: ABS, ESP, Sensores de ángulo de dirección.
- **Sensores y Actuadores de Cabina**: Climatización, confort, infotainment.
- **Dirección Electroasistida (EPS)**: Funcionamiento y diagnóstico.
- **Actuadores Principales**: Inyectores, bobinas, válvulas solenoides, motores paso a paso.
- **Sistemas de Comunicación**: CAN Bus, LIN Bus, FlexRay.

## Enfoque

La documentación está diseñada para mecánicos tradicionales, explicando los conceptos electrónicos de forma sencilla, comparando con sistemas anteriores y proporcionando guías de diagnóstico prácticas.

# Sensores y Actuadores en Vehículos Modernos

## ÍNDICE

1. [INTRODUCCIÓN](introduccion.md)
2. [SENSORES PRINCIPALES](sensores-principales.md)
   1. [SENSORES DEL SISTEMA DE GESTIÓN DEL MOTOR](sensores-motor.md)
      1. [Sensor CKP (Sensor de Posición del Cigüeñal)](sensores-motor.md#11-sensor-ckp-sensor-de-posición-del-cigüeñal)
      2. [Sensor CMP (Sensor de Posición del Árbol de Levas)](sensores-motor.md#12-sensor-cmp-sensor-de-posición-del-árbol-de-levas)
      3. [Sensores MAF/MAP (Medidor de Flujo de Aire/Presión del Múltiple)](sensores-motor.md#13-sensores-mafmap-medidor-de-flujo-de-airepresión-del-múltiple)
      4. [Sensores de Oxígeno (Sondas Lambda)](sensores-motor.md#14-sensores-de-oxígeno-sondas-lambda)
      5. [Sensor de Presión de Riel de Combustible](sensores-motor.md#15-sensor-de-presión-de-riel-de-combustible)
      6. [Sensor de Temperatura del Refrigerante (ECT)](sensores-motor.md#16-sensor-de-temperatura-del-refrigerante-ect)
      7. [Sensor de Detonación (Knock Sensor)](sensores-motor.md#17-sensor-de-detonación-knock-sensor)
      8. [Sensor de Posición del Acelerador (TPS)](sensores-motor.md#18-sensor-de-posición-del-acelerador-tps)
      9. [Sensor de Temperatura del Aire de Admisión (IAT)](sensores-motor.md#19-sensor-de-temperatura-del-aire-de-admisión-iat)
   2. [SISTEMA DE ACELERACIÓN ELECTRÓNICA (APPS/ETC)](sistema-aceleracion-electronica.md)
   3. [SENSORES DE EMISIONES](sensores-emisiones.md)
      1. [Sensor de Temperatura de Gases de Escape (EGTS)](sensores-emisiones.md#31-sensor-de-temperatura-de-gases-de-escape-egts)
      2. [Sensor de Presión Diferencial (DPF/FAP)](sensores-emisiones.md#32-sensor-de-presión-diferencial-dpffap)
      3. [Sensor NOx](sensores-emisiones.md#33-sensor-nox)
   4. [SISTEMAS DE ASISTENCIA A LA CONDUCCIÓN (ADAS)](sistemas-adas.md)
      1. [Cámaras y Radares](sistemas-adas.md#41-cámaras-y-radares)
      2. [Sensores de Estacionamiento (Ultrasónicos)](sistemas-adas.md#42-sensores-de-estacionamiento-ultrasónicos)
      3. [Sensores LIDAR](sistemas-adas.md#43-sensores-lidar)
   5. [SENSORES Y ACTUADORES DEL SISTEMA DE FRENOS](sensores-frenos.md)
      1. [Sensor ABS (Velocidad de Ruedas)](sensores-frenos.md#51-sensor-abs-velocidad-de-ruedas)
      2. [Sensor de Posición del Pedal de Freno](sensores-frenos.md#52-sensor-de-posición-del-pedal-de-freno)
      3. [Sensor de Desgaste de Pastillas](sensores-frenos.md#53-sensor-de-desgaste-de-pastillas)
      4. [Actuador de Freno Electrónico de Estacionamiento](sensores-frenos.md#54-actuador-de-freno-electrónico-de-estacionamiento)
      5. [Unidad Hidráulica de Control ABS/ESP](sensores-frenos.md#55-unidad-hidráulica-de-control-absesp)
      6. [Sensor de Ángulo de Dirección](sensores-frenos.md#56-sensor-de-ángulo-de-dirección)
   6. [SENSORES Y ACTUADORES DE LA CABINA](sensores-cabina.md)
      1. [Sensor Anti-atrapamiento de Ventanillas](sensores-cabina.md#61-sensor-anti-atrapamiento-de-ventanillas)
      2. [Sensores de Ocupación de Asientos](sensores-cabina.md#62-sensores-de-ocupación-de-asientos)
      3. [Sensores de Calidad del Aire](sensores-cabina.md#63-sensores-de-calidad-del-aire)
      4. [Sensores de Lluvia y Luz](sensores-cabina.md#64-sensores-de-lluvia-y-luz)
      5. [Actuadores de Climatización Automática](sensores-cabina.md#65-actuadores-de-climatización-automática)
      6. [Espejos con Memoria y Ajuste Automático](sensores-cabina.md#66-espejos-con-memoria-y-ajuste-automático)
   7. [SISTEMAS DE DIRECCIÓN ELECTROASISTIDA](direccion-electroasistida.md)
      1. [Motor de Asistencia Eléctrica](direccion-electroasistida.md#71-motor-de-asistencia-eléctrica)
      2. [Sensor de Par de Dirección](direccion-electroasistida.md#72-sensor-de-par-de-dirección)
      3. [Unidad de Control de Dirección Asistida](direccion-electroasistida.md#73-unidad-de-control-de-dirección-asistida)
      4. [Dirección Asistida Variable](direccion-electroasistida.md#74-dirección-asistida-variable)
3. [ACTUADORES PRINCIPALES](actuadores-principales.md)
   1. [SISTEMA DE GESTIÓN DEL MOTOR](actuadores-principales.md#1-sistema-de-gestión-del-motor)
      1. [Inyectores de Combustible](actuadores-principales.md#11-inyectores-de-combustible)
      2. [Bobinas de Encendido](actuadores-principales.md#12-bobinas-de-encendido)
      3. [Actuador de Control del Turbo (Geometría Variable/Wastegate)](actuadores-principales.md#13-actuador-de-control-del-turbo-geometría-variablewastegate)
      4. [Actuador de Distribución Variable de Válvulas (VVT)](actuadores-principales.md#14-actuador-de-distribución-variable-de-válvulas-vvt)
      5. [Cuerpo de Aceleración Electrónico](actuadores-principales.md#15-cuerpo-de-aceleración-electrónico)
      6. [Válvula de Marcha Mínima (IAC/Motor Paso a Paso)](actuadores-principales.md#16-válvula-de-marcha-mínima-iacmotor-paso-a-paso)
   2. [SISTEMAS DE EMISIONES Y POSTRATAMIENTO](actuadores-principales.md#2-sistemas-de-emisiones-y-postratamiento)
      1. [Válvula EGR (Recirculación de Gases de Escape)](actuadores-principales.md#21-válvula-egr-recirculación-de-gases-de-escape)
      2. [Inyector de AdBlue/Urea (SCR)](actuadores-principales.md#22-inyector-de-adbleurea-scr)
      3. [Válvula de Purga del Cánister (EVAP)](actuadores-principales.md#23-válvula-de-purga-del-cánister-evap)
   3. [SISTEMAS ELÉCTRICOS E HÍBRIDOS](actuadores-principales.md#3-sistemas-eléctricos-e-híbridos)
      1. [Inversores de Potencia](actuadores-principales.md#31-inversores-de-potencia)
      2. [Convertidor DC-DC (Reemplazo del Alternador)](actuadores-principales.md#32-convertidor-dc-dc-reemplazo-del-alternador)
      3. [Motor Eléctrico de Tracción](actuadores-principales.md#33-motor-eléctrico-de-tracción)
      4. [Sensor Resolver](actuadores-principales.md#34-sensor-resolver)
4. [SISTEMAS DE COMUNICACIÓN](sistemas-comunicacion.md)
   1. [REDES DE COMUNICACIÓN VEHICULAR](sistemas-comunicacion.md#1-redes-de-comunicación-vehicular)
      1. [Módulo de Control de Carrocería (BCM)](sistemas-comunicacion.md#11-módulo-de-control-de-carrocería-bcm)
      2. [Gateway de Comunicaciones](sistemas-comunicacion.md#12-gateway-de-comunicaciones)
