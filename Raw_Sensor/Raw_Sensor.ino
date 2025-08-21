/*
  Proyecto: Sensor de pulso -> LED por rango de valor RAW
  Descripción:
    - Lee el valor analógico del sensor en A0.
    - Si el valor crudo está dentro de un rango definido (umbral bajo y alto),
      enciende el LED en el pin 13.
    - También imprime el valor crudo por el puerto serial para calibración.
*/

// -------------------- Configuración de pines --------------------
const int PIN_SENSOR = A0;   // Pin analógico donde está conectado el sensor
const int PIN_LED    = 13;   // LED (pin digital 13 o LED integrado en la placa)

// -------------------- Rango de activación --------------------
int UMBRAL_BAJO = 500;   // Valor mínimo del rango (ajustar según sensor)
int UMBRAL_ALTO = 600;   // Valor máximo del rango (ajustar según sensor)

// -------------------- Variables --------------------
int valorSensor = 0;     // Almacena la lectura del sensor

void setup() {
  pinMode(PIN_LED, OUTPUT);   // Configura LED como salida
  Serial.begin(9600);         // Inicia comunicación serial
}

void loop() {
  valorSensor = analogRead(PIN_SENSOR); // Lee el valor del sensor (0–1023)
  Serial.println(valorSensor);          // Imprime valor crudo (raw) en Serial Monitor

  // Si el valor está dentro del rango, enciende el LED
  if (valorSensor >= UMBRAL_BAJO && valorSensor <= UMBRAL_ALTO) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
  }

  delay(20); // Pausa corta para estabilidad
}
