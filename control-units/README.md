> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Unidades de Control Electrónico (ECUs)

Esta sección contiene proyectos y ejemplos relacionados con diferentes Unidades de Control Electrónico (ECUs) utilizadas en vehículos modernos.

## Proyectos Incluidos

- **engine-control-unit**: Simulador básico de una ECU de motor utilizando ESP32-S3 y pantalla LCD.

## Propósito

El objetivo es proporcionar ejemplos prácticos y código fuente para entender el funcionamiento interno de las ECUs y cómo interactúan con los sensores y actuadores del vehículo.

## Estructura

Cada subcarpeta representa una unidad de control específica, implementada con microcontroladores modernos y pensada para ayudar a técnicos y estudiantes a comprender el funcionamiento de los sistemas electrónicos en vehículos actuales.

## Primera Unidad: Unidad de Control del Motor (Engine Control Unit)

La primera ECU implementada es la **unidad de control del motor**. Sus principales características serán:

- Control real del encendido electrónico en motores de 4 cilindros originalmente equipados con distribuidor y sistemas mecánicos (sin computadora)
- Lectura y visualización de las revoluciones del motor (RPM)
- Lectura y visualización de la presión del múltiple de admisión (MAP)
- Lectura y visualización del sensor de oxígeno para indicar si la mezcla aire-combustible es rica o pobre
- Visualización de todos estos parámetros en una pantalla integrada

Este proyecto está orientado a facilitar la transición de la mecánica tradicional a la electrónica automotriz, permitiendo observar y controlar en tiempo real los parámetros principales del motor, reemplazando el distribuidor y sistemas mecánicos por una ECU moderna.

---

## Advertencias y recomendaciones de instalación

- Este proyecto está diseñado para ser instalado en motores reales de 4 cilindros que originalmente utilizan distribuidor y sistemas mecánicos de encendido.
- La instalación y puesta en marcha debe ser realizada únicamente por personal capacitado, siguiendo todas las normas de seguridad automotriz.
- Es fundamental desconectar la batería antes de realizar cualquier conexión eléctrica.
- Se recomienda utilizar fusibles y protecciones adecuadas en todas las líneas de alimentación.
- Verificar la compatibilidad de los sensores y actuadores antes de la instalación.
- No utilizar este sistema en vehículos de uso en vía pública sin la debida validación y pruebas de seguridad.
- El técnico debe asegurarse de que todas las conexiones estén firmes y aisladas para evitar cortocircuitos o daños a la ECU y al motor.
- Ante cualquier duda, consultar con un especialista en electrónica automotriz.

---

> Para más información sobre cada unidad, consulte el README específico dentro de cada subcarpeta.
