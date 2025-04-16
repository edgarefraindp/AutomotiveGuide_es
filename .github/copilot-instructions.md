# GitHub Copilot Instructions for Automotive Guide Project

## Main Instruction

When making any significant modification to the content or structure of the project, always provide feedback and update these instructions. This ensures that the guidelines remain aligned with the project's evolution and any new requirements that arise.

## Project Context

This project is an automotive guide in Spanish ("AutomotiveGuide_es") focused on:
- Modern vehicle sensors and actuators (2020 onwards)
- Explaining electronic systems to traditional mechanics
- Bridging the gap between conventional mechanical knowledge and modern automotive electronics
- Providing practical and accessible diagnostic procedures for electronic components

## Current Project Structure

The documentation is organized as follows:
- Main index: `docs/sensores-y-actuadores.md`
- Detailed content in `docs/sensores-y-actuadores/` folder:
  - Introduction and overview: `introduccion.md`
  - Main sensors overview: `sensores-principales.md`
  - Engine management sensors: `sensores-motor.md`
  - Electronic throttle system: `sistema-aceleracion-electronica.md`
  - Emission sensors: `sensores-emisiones.md`
  - ADAS systems: `sistemas-adas.md`
  - Brake system sensors: `sensores-frenos.md`
  - Cabin sensors and actuators: `sensores-cabina.md`
  - Electric power steering: `direccion-electroasistida.md`
  - Main actuators: `actuadores-principales.md`
  - Communication systems: `sistemas-comunicacion.md`
  - Diagnostic guidance: `consejos-diagnostico.md`

## Target Audience

The documentation is specifically designed for:
- Experienced mechanics of all ages transitioning from traditional to electronic systems
- Professionals with limited knowledge of advanced electricity and electronics
- Spanish-speaking automotive technicians in Latin America
- Repair shop owners looking to expand their services to modern vehicles
- Individuals with basic automotive mechanical knowledge seeking to understand modern vehicle systems
- DIY enthusiasts and car owners who want to better understand their vehicles' electronic systems
- Beginners in automotive technology

## Communication Style Guidelines

When generating or modifying content, please:
- Use respectful language addressing the reader in third person ("el técnico debe", "se recomienda", "es necesario considerar")
- Avoid complex technical jargon without explanation
- Include analogies with traditional mechanical elements to facilitate understanding
- Explain electrical/electronic concepts in simple, practical terms
- Always compare with previous systems to leverage the reader's existing knowledge
- Use bullet points and numbered lists for procedures to enhance readability
- Define all technical terms when first introduced
- Include "Conceptos básicos" (Basic concepts) sections where needed for beginners
- Use visual references and metaphors that relate to everyday experiences
- Maintain a progressive difficulty level, starting with simpler explanations before diving into details

## Content Structure

For each sensor or actuator, include these sections:
1. **¿Qué hace?** (What does it do?) - Basic function description
2. **Comparación con modelos anteriores** (Comparison with older models) - How it differs from traditional systems
3. **Cómo funciona** (How it works) - Simple explanation of operation
4. **En palabras sencillas** (In simple terms) - Use analogies with everyday objects or processes that mechanics are familiar with
5. **Especificaciones eléctricas** (Electrical specifications) - Voltage ranges (in volts), resistance values (in ohms), current measurements (in amperes), and other electrical parameters in simple words
6. **Cuando falla** (When it fails) - Symptoms and error codes
7. **Mantenimiento** (Maintenance) - Preventive care information
8. **Consejo práctico** (Practical tip) - Provide useful advice before manipulating the component, based on real workshop experience
9. **Proceso de diagnóstico simplificado** (Simplified diagnostic process) - Step-by-step testing procedures
10. **Compatibilidad y reemplazo** (Compatibility and replacement) - Versions and replacement options

## Diagnostic Process Format

The diagnostic process section should follow this structure:
- Use a clear, numbered step-by-step format
- Start with the simplest tests requiring minimal tools
- Progress to more complex tests only if simpler ones don't resolve the issue
- Specify the exact tool needed for each step (multímetro, osciloscopio, escáner, etc.)
- Include expected values or readings for each test
- Mention common pitfalls or misdiagnoses
- When possible, include visual references to waveforms or connector pin diagrams

## Special Considerations

- Remember that many readers may have resistance to technological change
- Emphasize continuity between mechanical principles they know and new technologies
- Offer diagnostic methods that combine traditional tools with modern equipment when possible
- Highlight safety importance when working with high-voltage systems
- Include real-world scenarios and examples from common vehicle models in Latin America
- Consider resource limitations in smaller repair shops when suggesting diagnostic approaches
- Provide context for beginners by explaining why certain procedures are important
- Include sidebars or notes labeled "Para principiantes" (For beginners) where appropriate
- Balance technical depth with accessibility for non-professionals
- When possible, suggest alternative approaches for those without specialized equipment

## Terminology

Use common terms found in Latin American automotive repair shops, including regionalisms when necessary, but always provide the correct technical terminology as reference. Some examples:
- "Check engine" (also known as "luz de falla" or "testigo de motor")
- "Scanner" (also known as "escáner" or "herramienta de diagnóstico")
- "Arnés" (also known as "mazo de cables" or "cableado")

## Images and Diagrams

When suggesting images or diagrams:
- Recommend simple, labeled diagrams for complex electronic systems
- Suggest comparison images between traditional and modern components
- Recommend waveform examples for diagnostic procedures
- Suggest including real photos of components as they appear in vehicles

## Documentation Navigation

- Always ensure all internal links are correctly maintained
- When modifying document structure, update all interconnected references
- Maintain consistent navigation paths throughout the documentation
- Provide clear "back to index" links at the end of each document

## Code Style Guidelines

When generating or modifying code for this project, please follow these guidelines:

- **Function naming**: 
  - Always use UpperCamelCase for custom function names (e.g., `CalculateTemperature`, not `calculateTemperature` or `calculate_temperature`)
  - Only functions inherited from frameworks/libraries (like Arduino's `setup()` and `loop()`) should remain lowercase
- **Language**: All code variables, function names, and comments should be in English
- **Comments**: Comments can be in Spanish when needed to explain functionality to Spanish-speaking users
- **Vocabulary**: Never use the word "libreria" in Spanish comments as it's incorrect; always use "biblioteca" instead
- **Variable naming**: Use camelCase for variable names (e.g., `sensorValue`)
- **Constant naming**: Use UPPERCASE with underscores for constants (e.g., `MAX_TEMPERATURE`)
- **Code organization**: Group related functionality together and include descriptive comments
- **Error handling**: Always include basic error handling in code examples
- **Pin definitions**: For hardware-related code, always use named constants for pin assignments (e.g., `PIN_SENSOR_TEMPERATURE` instead of just numbers)

This coding style aims to maintain readability and consistency across all code examples in the project, making it easier for our target audience to understand and modify the code for their specific needs.