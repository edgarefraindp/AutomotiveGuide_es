> **Repositorio**: [https://github.com/edgarefraindp/AutomotiveGuide_es](https://github.com/edgarefraindp/AutomotiveGuide_es)  
> **Para donaciones y soporte**: Visite la página del repositorio en GitHub

---

# Lector CAN Bus para ESP32

## Introducción

Este proyecto es la versión ESP32 del Lector CAN Bus, una herramienta diagnóstica para verificar actividad en redes CAN cuando los escáneres convencionales no logran comunicarse. La implementación en ESP32 ofrece ventajas adicionales como mayor potencia de procesamiento, conectividad WiFi/Bluetooth, y mayor memoria, permitiendo funcionalidades extendidas como el registro de datos o la conectividad con aplicaciones móviles.

## Características adicionales respecto a la versión Arduino

- **Conectividad inalámbrica**: Permite monitorear el CAN bus remotamente mediante WiFi
- **Interfaz web**: Visualización de datos CAN en tiempo real desde cualquier navegador
- **Almacenamiento de logs**: Registro de actividad CAN en la memoria Flash integrada 
- **Análisis estadístico avanzado**: Mayor capacidad de procesamiento para patrones complejos
- **Múltiples interfaces CAN**: Posibilidad de monitorear dos redes CAN simultáneamente
- **Filtrado avanzado de mensajes**: Capacidad para filtrar mensajes específicos por ID o contenido
- **Modo de baja energía**: Para diagnósticos de larga duración con batería

## Componentes necesarios

Los componentes son similares a la versión Arduino, con algunas diferencias:

| Componente | Descripción | Cantidad |
|------------|-------------|----------|
| ESP32 DevKit | Placa de desarrollo ESP32 | 1 |
| Transceptor CAN SN65HVD230 | Interfaz CAN a 3.3V (compatible con ESP32) | 1-2 |
| Pantalla OLED I2C | Visualización de datos (opcional) | 1 |
| LEDs | Para indicación visual | 2-4 |
| Pulsadores | Para cambiar entre modos | 2-3 |
| Interruptor | Para activar/desactivar resistencia terminadora | 1 |
| Resistencia 120Ω | Resistencia terminadora | 1 |
| Caja protectora | Para montar el circuito | 1 |
| Conector OBD-II | Para conectar al vehículo | 1 |

## Implementación

Este proyecto está actualmente en desarrollo. El código se actualizará próximamente con las siguientes funcionalidades:

- **Modo monitor básico**: Visualización de IDs y datos en formato simple
- **Modo monitor avanzado**: Análisis detallado con interpretación de mensajes
- **Modo diagnóstico**: Comunicación activa con módulos siguiendo protocolo OBD2
- **Modo registro**: Almacenamiento de actividad para análisis posterior
- **Modo análisis estadístico**: Patrones de comunicación y detección de anomalías

## Diferencias con la versión Arduino

| Característica | Arduino | ESP32 |
|----------------|---------|-------|
| Velocidad CPU | 16 MHz | 240 MHz |
| Memoria | ~2-32 KB | 520 KB |
| Conectividad | Serial | WiFi + BT + Serial |
| Interfaces CAN | 1 (con módulo) | 1-2 (con módulos) |
| Visualización | LCD 16x2 | OLED gráfico / Web |
| Análisis | Básico | Avanzado |
| Alimentación | 5V | 3.3V (más eficiente) |

## Compatibilidad con vehículos

Al igual que la versión Arduino, esta herramienta es compatible con cualquier vehículo que utilice CAN bus estándar (ISO 11898) a velocidades entre 125 kbps y 1 Mbps, lo que incluye prácticamente todos los vehículos desde 2008 en adelante.

## Próximos pasos

El desarrollo completo de este proyecto incluirá:
- Implementación de código base ESP-IDF
- Desarrollo de interfaz web para visualización remota
- Creación de aplicación móvil complementaria
- Soporte para protocolos adicionales (ISO-TP, J1939)
- Capacidad de actualización OTA (Over-The-Air)

## Véase también

- [Versión Arduino del Lector CAN bus](../../arduino/lectorCanBus/README.md)
- [Diagnóstico de problemas en sistema CAN Bus](../../../docs/diagnostico/diagnostico-canbus.md)

[Volver al índice](../README.md)