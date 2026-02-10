#define LOMENOS 11
#define LOMAS   9
#define DELAY_MUESTREO 10  // 10ms = 100 Hz
// ==================== CONFIGURACIÓN ====================
// Detección de picos
int UMBRAL_BAJO = 300;     // Ajustar según tu señal
int UMBRAL_ALTO = 500;     // Ajustar según tu señal
const int REFRACTARIO_MS = 200;  // Período refractario
// Buffer de intervalos RR
const int MAX_RR = 40;            // Máximo de intervalos guardados
unsigned int rr_intervals[MAX_RR]; // Intervalos en ms
int rr_count = 0;                 // Cantidad de intervalos guardados
int rr_index = 0;                 // Índice del buffer circular
// Variables de detección
unsigned long ultimo_pico = 0;
int picos_detectados = 0;
// Ventana de análisis (20 segundos)
const unsigned long VENTANA_ANALISIS = 5000; // 20s en ms
unsigned long inicio_ventana = 0;
// ==================== FUNCIONES ====================
// Función 1: Detectar si hay un pico R
bool detectarPicoR(int valor_filtrado) {
  unsigned long ahora = millis();
  
  // Verificar si está en rango de umbrales
  if (valor_filtrado > UMBRAL_BAJO && valor_filtrado < UMBRAL_ALTO) {
    // Verificar período refractario
    if ((ahora - ultimo_pico) > REFRACTARIO_MS) {
      return true;
    }
  }
  return false;
}
// Función 2: Calcular y guardar intervalo RR
void guardarIntervaloRR(unsigned long tiempo_pico) {
  if (ultimo_pico > 0) {  // No es el primer pico
    unsigned int intervalo_rr = tiempo_pico - ultimo_pico;
    
    // Validar rango fisiológico (300-2000 ms = 30-200 bpm)
    if (intervalo_rr >= 300 && intervalo_rr <= 2000) {
      rr_intervals[rr_index] = intervalo_rr;
      rr_index = (rr_index + 1) % MAX_RR;
      if (rr_count < MAX_RR) rr_count++;
    }
  }
  ultimo_pico = tiempo_pico;
  picos_detectados++;
}
// Función 3: Calcular promedio de intervalos RR
unsigned int calcularRR_avg() {
  if (rr_count == 0) return 0;
  
  unsigned long suma = 0;
  for (int i = 0; i < rr_count; i++) {
    suma += rr_intervals[i];
  }
  return suma / rr_count;
}
// Función 4: Calcular RMSSD (variabilidad de corto plazo)
unsigned int calcularRMSSD() {
  if (rr_count < 2) return 0;
  
  unsigned long suma_diff_sq = 0;
  
  for (int i = 1; i < rr_count; i++) {
    int diff = (int)rr_intervals[i] - (int)rr_intervals[i-1];
    suma_diff_sq += (unsigned long)(diff * diff);
  }
  
  // Raíz cuadrada aproximada
  unsigned long media_sq = suma_diff_sq / (rr_count - 1);
  return (unsigned int)sqrt(media_sq);
}
// Función 5: Calcular SDNN (desviación estándar)
unsigned int calcularSDNN() {
  if (rr_count == 0) return 0;
  
  unsigned int media = calcularRR_avg();
  unsigned long suma_sq = 0;
  
  for (int i = 0; i < rr_count; i++) {
    int diff = (int)rr_intervals[i] - (int)media;
    suma_sq += (unsigned long)(diff * diff);
  }
  
  unsigned long varianza = suma_sq / rr_count;
  return (unsigned int)sqrt(varianza);
}
// Función 6: Detectar cruces por cero (oscilaciones)
int contarCrucesCero() {
  if (rr_count < 2) return 0;
  
  unsigned int media = calcularRR_avg();
  int cruces = 0;
  int signo_anterior = 0;
  
  for (int i = 0; i < rr_count; i++) {
    int diff = (int)rr_intervals[i] - (int)media;
    int signo = (diff >= 0) ? 1 : -1;
    
    if (i > 0 && signo != signo_anterior) {
      cruces++;
    }
    signo_anterior = signo;
  }
  
  return cruces;
}
// Función 7: Estimar frecuencia respiratoria
int estimarFrecuenciaRespiratoria() {
  if (rr_count < 5) return 0;
  
  int cruces = contarCrucesCero();
  unsigned int rr_avg = calcularRR_avg();
  
  // Tiempo total de la ventana en segundos
  unsigned long tiempo_total_s = ((unsigned long)rr_count * (unsigned long)rr_avg) / 1000UL;
  
  if (tiempo_total_s == 0) return 0;
  
  // Cada ciclo respiratorio = 2 cruces (subida + bajada)
  int ciclos_resp = cruces / 2;
  
  // Respiraciones por minuto
  int resp_rate = (ciclos_resp * 60) / tiempo_total_s;
  
  return resp_rate;
}
// Función 8: Detectar acoplamiento RSA
bool detectarAcoplamientoRSA(unsigned int rmssd, unsigned int sdnn, int resp_rate) {
  // Criterios de acoplamiento:
  // 1. RMSSD > 30 ms (hay variabilidad respiratoria)
  // 2. Frecuencia respiratoria entre 8-25 rpm
  // 3. SDNN razonable (<200 ms, sin arritmias severas)
  
  if (rmssd > 30 && resp_rate >= 8 && resp_rate <= 25 && sdnn < 200) {
    return true;
  }
  return false;
}
// Función 9: Calcular frecuencia cardíaca instantánea
int calcularBPM() {
  if (picos_detectados < 2) return 0;
  
  unsigned long tiempo_transcurrido = millis() - inicio_ventana;
  if (tiempo_transcurrido == 0) return 0;
  
  // BPM = (picos * 60000) / tiempo_en_ms
  int bpm = (picos_detectados * 60000UL) / tiempo_transcurrido;
  return bpm;
}
// Función 10: Mostrar análisis completo RSA
void mostrarAnalisisRSA() {
  Serial.println("\n========== ANÁLISIS RSA ==========");
  
  // Calcular métricas
  unsigned int rr_avg = calcularRR_avg();
  unsigned int rmssd = calcularRMSSD();
  unsigned int sdnn = calcularSDNN();
  int resp_rate = estimarFrecuenciaRespiratoria();
  int bpm = calcularBPM();
  bool rsa = detectarAcoplamientoRSA(rmssd, sdnn, resp_rate);
  
  // Mostrar resultados
  Serial.print("HR (frecuencia cardiaca) : ");
  Serial.print(bpm);
  Serial.println(" bpm");
  
  Serial.print("RR_avg (Tiempo promedio entre picos RR): ");
  Serial.print(rr_avg);
  Serial.println(" ms");
  
  Serial.print("RMSSD (Variabilidad entre latidos consecutivos): ");
  Serial.print(rmssd);
  Serial.println(" ms");
  
  Serial.print("SDNN (Variabilidad sobre el total): ");
  Serial.print(sdnn);
  Serial.println(" ms");
  
  Serial.print("Freq_Resp (Frecuencia Respiratoria): ");
  Serial.print(resp_rate);
  Serial.println(" rpm");
  
  Serial.print("RSA (Arritmia respiratoria): ");
  Serial.println(rsa ? "SI - Acoplamiento Detectado" : "NO - Sin Acoplamiento");
  
  Serial.print("Intervalos guardados: ");
  Serial.println(rr_count);
  
  Serial.println("==================================\n");
}
// Función 11: Reiniciar ventana de análisis
void reiniciarVentana() {
  inicio_ventana = millis();
  picos_detectados = 0;
  // NO reiniciamos rr_intervals, mantenemos histórico
}
// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  pinMode(LOMENOS, INPUT);
  pinMode(LOMAS, INPUT);
  
  Serial.println("=== Análisis RSA - AD8232 ===");
  Serial.println("Esperando señal ECG...\n");
  
  inicio_ventana = millis();
}
// ==================== LOOP PRINCIPAL ====================
void loop() {
  // Verificar conexión de electrodos
  if (digitalRead(LOMENOS) == 1 || digitalRead(LOMAS) == 1) {
    Serial.println(100);
    delay(500);
    return;
  }
  
  // Leer señal
  int valor = analogRead(A0);
  
  // Detectar pico R
  if (detectarPicoR(valor)) {
    unsigned long tiempo_actual = millis();
    guardarIntervaloRR(tiempo_actual);
    
    // Debug: mostrar cada pico detectado
    // Serial.println("¡Pico R detectado!");
  }
  
  // Mostrar señal en Serial Plotter (comentar si solo quieres análisis)
  Serial.println(valor);
  
  if (millis() - inicio_ventana >= VENTANA_ANALISIS) {
    // Serial.println("Pasaron 20 seg!");
    mostrarAnalisisRSA();
    reiniciarVentana();
  }
  
  delay(DELAY_MUESTREO);
}