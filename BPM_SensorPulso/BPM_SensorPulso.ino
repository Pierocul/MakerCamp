/*
  Proyecto: Sensor de pulso optimizado para Arduino Plotter
  Descripción:
    - Muestra datos en formato compatible con el Serial Plotter
    - Visualiza: señal cruda, umbrales, y BPM en tiempo real
    - Detecta BPM con algoritmo sensible para muñeca
    - LED de alerta cuando BPM > 80
*/

// -------------------- Configuración --------------------
const int PIN_SENSOR = A0;   
const int PIN_LED = 13;   
const int UMBRAL_BPM_ALERTA = 80;

// -------------------- Variables de análisis --------------------
const int BUFFER_SIZE = 30;
int buffer[BUFFER_SIZE];
int indiceBuffer = 0;
bool bufferCompleto = false;

float promedioMovil = 512;
float umbralSuperior = 520;
float umbralInferior = 504;
float sensibilidad = 1.8;

// -------------------- Detección de pulso --------------------
enum EstadoPulso { BUSCANDO_PICO, EN_PICO, BUSCANDO_VALLE };
EstadoPulso estado = BUSCANDO_PICO;
int contadorEstado = 0;

// -------------------- Variables BPM --------------------
unsigned long ultimoPulso = 0;
unsigned long tiempoActual = 0;
int bpmInstantaneo = 0;
int bpmSuavizado = 70;  // Valor inicial razonable
float bpmFloat = 70.0;

// Historial para suavizado
const int HIST_BPM = 5;
int historialBPM[HIST_BPM];
int indiceBPM = 0;

// -------------------- Variables de tiempo --------------------
unsigned long ultimoPlot = 0;
unsigned long inicioPrograma = 0;
bool calibrado = false;
int contadorCalibrar = 0;

// -------------------- Variables para el plotter --------------------
int valorParaPlot = 0;
int umbralSuperiorPlot = 0;
int umbralInferiorPlot = 0;
int bpmParaPlot = 0;
int senalPulso = 0;  // Indicador visual de detección

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(9600);
  
  // Inicializar arrays
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 512;
  }
  for (int i = 0; i < HIST_BPM; i++) {
    historialBPM[i] = 70;
  }
  
  inicioPrograma = millis();
  
  // Mensaje inicial (solo aparecerá en Serial Monitor, no en Plotter)
  delay(1000);
  Serial.println("Iniciando monitor de BPM...");
}

void loop() {
  tiempoActual = millis();
  int valorSensor = analogRead(PIN_SENSOR);
  
  // Actualizar buffer
  buffer[indiceBuffer] = valorSensor;
  indiceBuffer = (indiceBuffer + 1) % BUFFER_SIZE;
  if (indiceBuffer == 0) bufferCompleto = true;
  
  // Procesar cada 50ms (20 Hz - buena velocidad para el plotter)
  if (tiempoActual - ultimoPlot >= 50) {
    if (bufferCompleto) {
      actualizarEstadisticas();
      
      // Calibrar durante los primeros segundos
      if (!calibrado) {
        calibrarSensor();
      } else {
        detectarPulso();
      }
      
      // Preparar datos para el plotter
      prepararDatosPlotter(valorSensor);
      
      // Enviar datos al plotter
      enviarAPlotter();
    }
    ultimoPlot = tiempoActual;
  }
  
  // Control del LED
  if (bpmSuavizado > UMBRAL_BPM_ALERTA) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
  }
  
  delay(10);
}

void actualizarEstadisticas() {
  long suma = 0;
  int minVal = 1023, maxVal = 0;
  
  // Calcular estadísticas del buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    suma += buffer[i];
    if (buffer[i] < minVal) minVal = buffer[i];
    if (buffer[i] > maxVal) maxVal = buffer[i];
  }
  
  promedioMovil = (float)suma / BUFFER_SIZE;
  
  // Calcular desviación estándar
  float varianza = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    float diff = buffer[i] - promedioMovil;
    varianza += diff * diff;
  }
  float desviacion = sqrt(varianza / BUFFER_SIZE);
  
  // Ajustar umbrales dinámicamente
  umbralSuperior = promedioMovil + (desviacion * sensibilidad);
  umbralInferior = promedioMovil - (desviacion * sensibilidad);
  
  // Asegurar umbrales mínimos
  if (umbralSuperior - umbralInferior < 4) {
    umbralSuperior = promedioMovil + 2;
    umbralInferior = promedioMovil - 2;
  }
}

void calibrarSensor() {
  contadorCalibrar++;
  
  if (contadorCalibrar >= 60) {  // 3 segundos a 20Hz
    calibrado = true;
    
    // Ajustar sensibilidad según la variabilidad
    int valorMin = 1023, valorMax = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (buffer[i] < valorMin) valorMin = buffer[i];
      if (buffer[i] > valorMax) valorMax = buffer[i];
    }
    
    int rango = valorMax - valorMin;
    if (rango < 8) {
      sensibilidad = 1.2;  // Muy sensible
    } else if (rango < 20) {
      sensibilidad = 1.8;  // Sensibilidad normal
    } else {
      sensibilidad = 2.2;  // Menos sensible
    }
  }
}

void detectarPulso() {
  int valorActual = buffer[(indiceBuffer - 1 + BUFFER_SIZE) % BUFFER_SIZE];
  
  switch (estado) {
    case BUSCANDO_PICO:
      if (valorActual > umbralSuperior) {
        estado = EN_PICO;
        contadorEstado = 1;
        senalPulso = 50; // Señal visual en el plotter
      }
      break;
      
    case EN_PICO:
      contadorEstado++;
      if (valorActual < promedioMovil && contadorEstado >= 2) {
        // Pulso detectado
        registrarPulso();
        estado = BUSCANDO_VALLE;
        contadorEstado = 0;
        senalPulso = 100; // Pico más alto para indicar detección
      } else if (contadorEstado > 8) {
        // Muy largo, reiniciar
        estado = BUSCANDO_PICO;
        contadorEstado = 0;
      }
      break;
      
    case BUSCANDO_VALLE:
      contadorEstado++;
      if (valorActual < umbralInferior || contadorEstado > 10) {
        estado = BUSCANDO_PICO;
        contadorEstado = 0;
      }
      break;
  }
  
  // Reducir gradualmente la señal visual
  if (senalPulso > 0) {
    senalPulso -= 5;
  }
}

void registrarPulso() {
  if (ultimoPulso > 0) {
    unsigned long intervalo = tiempoActual - ultimoPulso;
    
    // Validar intervalo (40-180 BPM)
    if (intervalo >= 333 && intervalo <= 1500) {
      bpmInstantaneo = 60000 / intervalo;
      
      // Suavizar BPM
      historialBPM[indiceBPM] = bpmInstantaneo;
      indiceBPM = (indiceBPM + 1) % HIST_BPM;
      
      calcularBPMSuavizado();
    }
  }
  
  ultimoPulso = tiempoActual;
}

void calcularBPMSuavizado() {
  int suma = 0;
  int contador = 0;
  
  for (int i = 0; i < HIST_BPM; i++) {
    if (historialBPM[i] > 0) {
      suma += historialBPM[i];
      contador++;
    }
  }
  
  if (contador > 0) {
    float nuevoBPM = (float)suma / contador;
    
    // Filtro pasa-bajos para suavizar aún más
    bpmFloat = bpmFloat * 0.8 + nuevoBPM * 0.2;
    bpmSuavizado = (int)bpmFloat;
    
    // Limitar a rangos razonables
    if (bpmSuavizado < 40) bpmSuavizado = 40;
    if (bpmSuavizado > 200) bpmSuavizado = 200;
  }
}

void prepararDatosPlotter(int valorSensor) {
  // Escalar valores para que se vean bien en el plotter
  valorParaPlot = valorSensor;
  umbralSuperiorPlot = (int)umbralSuperior;
  umbralInferiorPlot = (int)umbralInferior;
  
  // Escalar BPM para que se vea en el mismo rango que la señal del sensor
  // Mapear BPM (40-180) a rango del sensor (aprox 200-800)
  bpmParaPlot = map(bpmSuavizado, 40, 180, 200, 800);
  
  // Limitar el rango
  if (bpmParaPlot < 200) bpmParaPlot = 200;
  if (bpmParaPlot > 800) bpmParaPlot = 800;
}

void enviarAPlotter() {
  // Solo mostrar BPM en el plotter
  Serial.print("BPM:");
  Serial.println(bpmSuavizado);    
   Serial.println(indiceBuffer);    // BPM real (sin escalar)
}