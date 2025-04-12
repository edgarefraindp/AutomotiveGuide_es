# ACTUADORES PRINCIPALES

Los actuadores en vehículos modernos son componentes que convierten señales eléctricas en acciones mecánicas o físicas. Son responsables de ejecutar las órdenes de las unidades de control para regular el funcionamiento de los diferentes sistemas del vehículo.

## 1. SISTEMA DE GESTIÓN DEL MOTOR

### 1.1 Inyectores de Combustible

- **Función**: Pulverizar combustible en cantidad, tiempo y patrón precisos
- **Tipos**:
  - Inyectores de puerto (MPI/PFI): 3-5 bar de presión
  - Inyectores directos (GDI/TSI/TFSI): 50-350 bar
  - Inyectores diésel common rail: 300-2500 bar
- **Tecnología**: 
  - Inyectores electromagnéticos
  - Inyectores piezoeléctricos (respuesta más rápida)
- **Características modernas**:
  - Multiinyección (hasta 9 inyecciones por ciclo)
  - Control de cantidad mínima (0.5-1.5 mg/inyección)
  - Ángulo de pulverización variable
- **Señal de control**: PWM (modulación por ancho de pulso) de alta frecuencia
- **Síntomas de fallo**:
  - Ralentí irregular
  - Pérdida de potencia
  - Mayor consumo y emisiones
  - Código de fallo de cilindro específico
- **Diagnóstico**:
  - Medición de resistencia (2-16 ohms típico)
  - Prueba de equilibrio de cilindros
  - Comprobación de patrón de pulverización
  - Análisis de forma de onda con osciloscopio

### 1.2 Bobinas de Encendido

- **Función**: Generar alta tensión para las bujías
- **Tipos**:
  - Bobina sobre bujía (COP)
  - Bobina con cables de alta tensión (DIS)
  - Bobinas integradas en módulo (cassette)
- **Tecnología**: 
  - Transformador con transistor de potencia integrado
  - Control por IGBTs
- **Características modernas**:
  - Capacidad multi-chispa
  - Energía de chispa ajustable
  - Diagnóstico integrado de circuito primario
- **Señal de control**: Digital, 5-12V
- **Síntomas de fallo**:
  - Fallo de encendido en cilindro específico
  - Pérdida de potencia
  - Consumo elevado
  - Luz MIL parpadeante
- **Diagnóstico**:
  - Medición de resistencia primaria (0.5-2 ohms)
  - Medición de resistencia secundaria (6-15K ohms)
  - Análisis de señal primaria/secundaria con osciloscopio

### 1.3 Actuador de Control del Turbo (Geometría Variable/Wastegate)

- **Función**: Regular la presión de sobrealimentación
- **Tipos**:
  - Válvula de descarga (wastegate)
  - Álabes de geometría variable (VTG)
- **Tecnología**: 
  - Actuador neumático con electroválvula
  - Actuador eléctrico directo (más preciso)
- **Ubicación**: Integrado en el turbocompresor
- **Señal de control**: 
  - PWM (actuador neumático)
  - Posición directa (actuador eléctrico)
- **Síntomas de fallo**:
  - Presión de sobrealimentación baja/excesiva
  - Respuesta lenta o retardada
  - Ruido de "flutter" del turbo
  - Modo limp-home activado
- **Diagnóstico**:
  - Comprobación de presión real vs. deseada
  - Verificación de movimiento completo
  - Test de actuación con escáner

### 1.4 Actuador de Distribución Variable de Válvulas (VVT)

- **Función**: Modificar la sincronización de apertura/cierre de válvulas
- **Tipos**:
  - Variador de fase (adelanto/atraso)
  - Variador de elevación y fase
  - Desactivación de cilindros
- **Tecnología**: 
  - Actuador hidráulico con control por electroválvula
  - Actuador eléctrico directo (sistemas más recientes)
- **Ubicación**: Extremo del árbol de levas o integrado en la polea
- **Síntomas de fallo**:
  - Ralentí inestable
  - Pérdida de potencia a ciertos regímenes
  - Ruido mecánico (traqueteo)
  - Mayor consumo
- **Diagnóstico**:
  - Verificación de valores reales vs. deseados
  - Comprobación de tiempos de respuesta
  - Análisis de presión de aceite en el sistema

### 1.5 Cuerpo de Aceleración Electrónico

- **Función**: Regular el flujo de aire hacia el motor
- **Tecnología**: 
  - Motor DC con reductora
  - Doble sensor de posición (TPS)
  - Resorte de seguridad para posición de reposo
- **Señal de control**: PWM bidireccional
- **Síntomas de fallo**:
  - Modo de emergencia (potencia limitada)
  - Ralentí irregular
  - Respuesta retardada al acelerador
- **Diagnóstico**:
  - Comprobación de resistencia del motor
  - Verificación de señales de los TPS
  - Limpieza de la mariposa cuando sea necesario
  - Procedimiento de aprendizaje/adaptación

### 1.6 Válvula de Marcha Mínima (IAC/Motor Paso a Paso)

- **Función**: Regular la entrada de aire en ralentí
- **Tecnología**: 
  - Motor paso a paso
  - Electroválvula proporcional
- **Ubicación**: En bypass del cuerpo de aceleración o integrada
- **Señal de control**: Secuencia de pasos o PWM
- **Síntomas de fallo**:
  - Ralentí inestable (alto o bajo)
  - Parada del motor al soltar acelerador
  - RPM fluctuantes
- **Diagnóstico**:
  - Comprobación de resistencia de bobinas
  - Verificación de movimiento mecánico
  - Limpieza del conducto de bypass

## 2. SISTEMAS DE EMISIONES Y POSTRATAMIENTO

### 2.1 Válvula EGR (Recirculación de Gases de Escape)

- **Función**: Recircular gases de escape para reducir NOx
- **Tipos**:
  - Neumática con control por electroválvula
  - Eléctrica con motor DC o paso a paso
  - Digital con posiciones discretas
- **Tecnología moderna**: 
  - EGR de baja presión con refrigeración
  - Control proporcional continuo
  - Sensor de posición integrado
- **Síntomas de fallo**:
  - Luz MIL encendida
  - Pérdida de potencia
  - Detonación (gasolina)
  - Exceso de humo negro (diésel)
- **Diagnóstico**:
  - Verificación de movimiento completo
  - Comprobación de obstrucciones por carbonilla
  - Test de actuación con escáner

### 2.2 Inyector de AdBlue/Urea (SCR)

- **Función**: Inyectar solución de urea para reducción de NOx
- **Tecnología**: 
  - Inyector calefactado
  - Control por PWM
  - Sistema de aire comprimido para atomización
- **Componentes asociados**:
  - Bomba de dosificación
  - Calentadores de línea
  - Sensores de nivel y calidad
- **Síntomas de fallo**:
  - Advertencia de "AdBlue/SCR System Fault"
  - Limitación progresiva de potencia/velocidad
  - Mayor consumo de AdBlue
- **Diagnóstico**:
  - Verificación de presión y caudal
  - Comprobación de calidad del fluido
  - Test de actuación con escáner específico

### 2.3 Válvula de Purga del Cánister (EVAP)

- **Función**: Controlar la purga de vapores de combustible
- **Tecnología**: 
  - Electroválvula PWM
  - Control de flujo proporcional
- **Ubicación**: Entre el cánister y el colector de admisión
- **Síntomas de fallo**:
  - Código P0440-P0457
  - Olor a combustible
  - Ralentí irregular tras repostaje
- **Diagnóstico**:
  - Verificación de resistencia (30-60 ohms típico)
  - Comprobación de flujo con bomba de vacío
  - Test de actuación con escáner

## 3. SISTEMAS ELÉCTRICOS E HÍBRIDOS

### 3.1 Inversores de Potencia

- **Función**: Convertir DC de batería a AC trifásica para motores eléctricos
- **Tecnología**: 
  - Transistores IGBT de potencia
  - Control por microprocesador
  - Sistema de refrigeración líquida
- **Características**:
  - Eficiencia >95%
  - Potencia: 50-300 kW
  - Voltaje: 200-800V
- **Síntomas de fallo**:
  - Códigos de fallo específicos
  - Activación de modo de emergencia
  - Pérdida de potencia o propulsión
- **Diagnóstico**:
  - Requiere equipamiento y formación específica
  - Verificación de aislamiento de alta tensión
  - Comprobación de sistema de refrigeración

### 3.2 Convertidor DC-DC (Reemplazo del Alternador)

- **Función**: Convertir alta tensión de batería principal a 12V para sistemas convencionales
- **Tecnología**: 
  - Convertidor de conmutación de alta frecuencia
  - Control integrado con BMS
- **Características**:
  - Potencia: 1.5-3 kW
  - Eficiencia >90%
  - Refrigeración líquida o por aire
- **Síntomas de fallo**:
  - Descarga de batería de 12V
  - Apagado de sistemas no críticos
  - Codes de fallo relacionados con sistema de carga
- **Diagnóstico**:
  - Verificación de tensión de salida (13.5-14.5V)
  - Comprobación de capacidad de carga bajo demanda
  - Escaneo de códigos específicos

### 3.3 Motor Eléctrico de Tracción

- **Función**: Convertir energía eléctrica en movimiento
- **Tipos**:
  - Síncrono de imanes permanentes (más común)
  - Asíncrono (inducción)
  - De reluctancia variable/conmutada
- **Características**:
  - Eficiencia: 90-97%
  - Potencia: 50-300 kW
  - RPM máximas: 10,000-20,000
- **Síntomas de fallo**:
  - Vibraciones
  - Ruidos anormales
  - Pérdida de potencia
  - Sobrecalentamiento
- **Diagnóstico**:
  - Medición de aislamiento
  - Verificación de sensores de temperatura
  - Comprobación de rodamientos

### 3.4 Sensor Resolver

- **Función**: Determinar posición y velocidad del rotor en motores eléctricos
- **Tecnología**: 
  - Transformador rotativo
  - Bobinas primaria y secundarias
- **Características**:
  - Alta precisión (<0.1°)
  - Resistencia a campos electromagnéticos
  - Funciona en condiciones extremas
- **Síntomas de fallo**:
  - Control errático del motor
  - Pérdida de eficiencia
  - Mensajes de error específicos
- **Diagnóstico**:
  - Verificación de señales con osciloscopio
  - Comprobación de resistencia de bobinas
  - Test específicos con escáner OEM

[Volver al índice](../sensores-y-actuadores.md)