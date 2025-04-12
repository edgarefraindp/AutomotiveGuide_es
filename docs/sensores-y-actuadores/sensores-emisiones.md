# SENSORES DE EMISIONES

Los sensores de emisiones son críticos para el cumplimiento de las normativas ambientales cada vez más estrictas (Euro 6d, Tier 3, etc.). Estos sensores permiten a los sistemas de control del motor ajustar parámetros para reducir emisiones contaminantes.

## 3.1 Sensor de Temperatura de Gases de Escape (EGTS)

- **Función**: Mide la temperatura en diferentes puntos del sistema de escape
- **Tecnología**: Termistor o termopar
- **Ubicación**: 
  - Antes y después del catalizador
  - Antes y después del filtro de partículas (DPF/FAP)
  - En conducto de EGR
- **Señal típica**: 
  - Termistor: Resistencia variable (30K-300K ohms en frío, 300-1K ohms en caliente)
  - Termopar: Voltaje proporcional a temperatura (mV)
- **Rango de operación**: -40°C a +900°C
- **Síntomas de fallo**: Regeneraciones DPF defectuosas, mayor consumo, mensaje de error en tablero
- **Diagnóstico**: Medición de resistencia en frío, verificación de continuidad

## 3.2 Sensor de Presión Diferencial (DPF/FAP)

- **Función**: Mide la diferencia de presión entre la entrada y salida del filtro de partículas
- **Tecnología**: Sensor piezorresistivo de doble entrada
- **Ubicación**: Montado en el vano motor, conectado al DPF/FAP mediante tubos
- **Señal típica**: 0.5-4.5V correspondiendo a presiones diferenciales de 0-100 kPa
- **Síntomas de fallo**: 
  - Luz de avería del motor (MIL) encendida
  - Regeneraciones excesivas o insuficientes
  - Mensaje de DPF saturado
- **Diagnóstico**: 
  - Verificar obstrucción en tubos de conexión
  - Comprobar señal en distintos estados de carga
  - Verificar con manómetro externo cuando sea posible

## 3.3 Sensor NOx

- **Función**: Mide la concentración de óxidos de nitrógeno en los gases de escape
- **Tecnología**: Electroquímico de tipo cerámico (zirconio)
- **Ubicación**: 
  - Después del catalizador
  - Después del sistema SCR (Reducción Catalítica Selectiva)
- **Señal típica**: Digital a través de bus CAN o LIN
- **Características**: 
  - Incorpora calentador interno
  - Requiere temperatura mínima de operación (>200°C)
  - Alta sensibilidad a contaminación por aceite o combustible
- **Síntomas de fallo**: 
  - Mayor consumo de AdBlue/urea
  - Mensajes de error de sistema SCR
  - No supera pruebas de emisiones
- **Diagnóstico**: 
  - Requiere escáner avanzado
  - Verificación de estado de calentador
  - Comprobación de señal en tiempo real

## Importancia en Vehículos Modernos

Los sensores de emisiones son cada vez más sofisticados para cumplir con las normas Euro 6d, WLTP y RDE (Real Driving Emissions). En muchos mercados, estos sensores están legalmente protegidos contra manipulaciones, y su alteración puede conllevar sanciones.

El diagnóstico de estos sistemas requiere equipamiento especializado y conocimiento de los protocolos específicos de cada fabricante.

[Volver a Sensores Principales](sensores-principales.md) | [Volver al índice](../sensores-y-actuadores.md)