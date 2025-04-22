> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Engine Control Unit (ECU) para Motor de 4 Cilindros

Este proyecto implementa una ECU real para motores de 4 cilindros originalmente equipados con distribuidor y sistemas mecánicos (sin computadora). Está diseñado para ser instalado en motores reales, reemplazando el sistema de encendido mecánico por un control electrónico moderno.

## Características principales

- Control de encendido electrónico para motores de 4 cilindros
- Lectura de sensores reales:
  - Sensor de posición (RPM)
  - Sensor de presión del múltiple de admisión (MAP)
  - Sensor de oxígeno (O2) para mezcla rica/pobre
  - Potenciómetro para acelerador
- Visualización en pantalla OLED:
  - Revoluciones por minuto (RPM)
  - Presión del múltiple de admisión (kPa o voltaje)
  - Estado de la mezcla aire-combustible (rica/pobre)
- Lógica de avance de encendido configurable
- Pensado para facilitar la transición de la mecánica tradicional a la electrónica automotriz

## Diagrama de bloques sugerido

- Entradas:
  - Botón de encendido
  - Botón de accesorios
  - Potenciómetro (acelerador)
  - Sensor de posición (CKP o señal de distribuidor)
  - Sensor MAP (presión múltiple)
  - Sensor de oxígeno (O2)
- Salidas:
  - Bobinas de encendido (1 por cilindro)
  - Pantalla OLED (I2C)

## Ejemplo de conexión de pines

- PIN_BUTTON_IGNITION: Botón de encendido
- PIN_BUTTON_ACCESSORY: Botón de accesorios
- PIN_POT_THROTTLE: Potenciómetro de acelerador
- PIN_RPM_SENSOR: Sensor de posición (CKP/distribuidor)
- PIN_MAP_SENSOR: Sensor de presión de múltiple (entrada analógica)
- PIN_O2_SENSOR: Sensor de oxígeno (entrada analógica)
- PIN_OLED_SDA / PIN_OLED_SCL: Pantalla OLED
- PIN_COIL_1 a PIN_COIL_4: Bobinas de encendido

## Advertencias y recomendaciones de seguridad

- Este sistema está diseñado para instalarse en motores reales. La manipulación incorrecta puede causar daños al motor o lesiones personales.
- La instalación debe ser realizada por personal capacitado en electrónica automotriz.
- Desconectar la batería antes de cualquier conexión eléctrica.
- Utilizar fusibles y protecciones adecuadas.
- Verificar compatibilidad de sensores y bobinas antes de la instalación.
- No utilizar en vehículos de uso en vía pública sin validación y pruebas de seguridad.
- Asegurar conexiones firmes y aisladas para evitar cortocircuitos.
- Consultar con un especialista ante cualquier duda.

## Estado actual

- Estructura base del proyecto creada
- Lectura de botones, potenciómetro y sensor de posición implementada
- Pendiente: integración de pantalla OLED, lectura de sensores MAP y O2, control de bobinas y lógica de avance

---

> Para detalles técnicos y ejemplos de código, consulte el archivo `main.c` en la carpeta `main/`.
