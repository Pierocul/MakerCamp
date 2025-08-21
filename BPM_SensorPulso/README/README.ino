README — Guía completa
1) ¿Qué hace este proyecto?

Lee un sensor de pulso analógico, detecta los latidos al cruzar un umbral, calcula el IBI (Inter-Beat Interval, tiempo entre latidos) y lo convierte a BPM.
Si el BPM supera 80 (o el valor que definas), enciende el LED.

Fórmula: BPM = 60000 / IBI_ms
(60000 ms equivalen a 1 minuto)

2) Hardware requerido

Arduino UNO/Nano/Mega (o compatible).

Sensor de pulso con salida analógica (p. ej. PulseSensor u otro fotopletismográfico).

LED (opcional si no usas el integrado) + resistencia 220 Ω.

Cables dupont.

Conexiones básicas:

Sensor → VCC a 5V (o según tu sensor), GND a GND, OUT a A0.

LED externo (opcional) → ánodo al pin 13 con resistencia a 5V, cátodo a GND.
(O usa el LED integrado del pin 13 en muchas placas).

3) Cómo funciona (paso a paso)

Lectura: analogRead(A0) obtiene un valor 0–1023 del sensor.

Suavizado: un promedio móvil de 4 muestras reduce ruido.

Detección de pico: cuando la señal suavizada supera UMBRAL_RAW y veníamos desde abajo, marcamos un latido.

IBI: medimos tiempo entre este latido y el anterior con millis().

BPM: 60000 / IBI_ms.

LED: si BPM > BPM_UMBRAL, LED ON; si no, OFF.

Timeout: si pasan >2 s sin latidos, apagamos LED y ponemos BPM=0.

4) Calibración imprescindible

Cada sensor y dedo es distinto. Debes ajustar UMBRAL_RAW:

Carga el sketch.

Abre Serial Plotter (115200 baudios).

Observa Raw y Suav mientras pones el dedo.

Ajusta UMBRAL_RAW para que:

Esté por encima del nivel base (sin dedo o valle).

Esté por debajo del pico típico del latido.

Ejemplo de partida: UMBRAL_RAW = 520 ~ 600. Ajusta fino según tu gráfica.

Si el LED nunca enciende y BPM siempre es 0 → umbral muy alto.
Si cuenta latidos locos → umbral muy bajo o ruido.

5) Parámetros clave (y por qué)

BPM_UMBRAL (por defecto 80): define cuándo enciende el LED.

N_SAMPLES_SUAV (4): ventana de suavizado; más grande = más estable pero menos “rápido”.

IBI_MIN_MS (300 ms ≈ 200 BPM): filtra picos falsos muy seguidos.

IBI_MAX_MS (2000 ms ≈ 30 BPM): filtra intervalos demasiado largos por ruido o pérdida de dedo.

TIMEOUT_MS (2000 ms): evita dejar LED encendido si dejas de medir.

6) Consejos de montaje y uso

Presión del dedo: ni muy fuerte ni muy floja.

Luz ambiente: evita luz directa (puede meter ruido).

Cables cortos y buen GND común.

Movimiento: mantén el dedo quieto.

Serial Plotter: excelente para ver forma de onda y decidir el umbral.

7) Resolución de problemas

BPM = 0 siempre

Umbral muy alto, no detecta picos.

Dedo mal puesto o sin contacto.

Sensor a 3.3V con placa a 5V (mismatch de niveles).

BPM muy inestable

Umbral muy bajo (ruido).

Sube N_SAMPLES_SUAV a 5–8.

Mejora sujeción y evita luz directa.

LED queda prendido “sin razón”

Revisa TIMEOUT_MS.

Verifica que BPM realmente > BPM_UMBRAL (mira el Serial).

8) Personalización rápida

Cambiar el umbral de alarma de BPM:
const int BPM_UMBRAL = 100; (por ejemplo para entrenamiento).

Hacer parpadear el LED solo en cada latido y solo si BPM > umbral:
Mueve el digitalWrite(HIGH/LOW) dentro del bloque de detección y agrega un delay(30) pequeño tras el HIGH para ver el “flash”.

9) Seguridad y responsabilidad

Este código es educativo. No es un dispositivo médico ni debe usarse para diagnóstico.

10) Extensiones opcionales (si quieres refinar)

Umbral adaptativo: usar un umbral relativo a un promedio/pico reciente.

Filtro digital: pasa-altos / pasa-bajos con IIR sencillo para estabilizar.

Promedio de BPM: promediar 3–5 latidos para un BPM más “lento” pero estable.

OLED/LCD: mostrar BPM en pantalla en lugar de la consola serial.