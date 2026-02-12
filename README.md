# 🫀 Análisis de Arritmia Sinusal Respiratoria (RSA) con AD8232 y Arduino

## 📌 Descripción

Este proyecto implementa un sistema de análisis de la señal ECG utilizando un sensor **AD8232** conectado a un **Arduino**.  
El código detecta los latidos cardíacos (picos R), calcula los intervalos entre ellos y obtiene métricas de variabilidad cardíaca para analizar la presencia de Arritmia Sinusal Respiratoria (RSA).

---

## 🎯 ¿Qué mide el sistema?

El programa calcula y muestra:

- **Frecuencia cardíaca (BPM)**  
- **Intervalo RR promedio (RR_avg)**  
- **RMSSD** (variabilidad de corto plazo entre latidos consecutivos)  
- **SDNN** (variabilidad total de los intervalos RR)  
- **Frecuencia respiratoria estimada** (a partir de las oscilaciones de los RR)  
- **Detección de acoplamiento RSA** (indica si existe arritmia sinusal respiratoria)

---

## ⚙️ ¿Cómo funciona el código?

### 1️⃣ Lectura de señal ECG
El Arduino lee la señal analógica proveniente del AD8232 mediante el pin A0.

---

### 2️⃣ Detección de picos R
Se identifican los latidos utilizando:
- Umbrales de amplitud.
- Un período refractario de 200 ms para evitar duplicados.

Cuando se detecta un pico válido, se calcula el intervalo RR.

---

### 3️⃣ Cálculo de intervalos RR
Cada intervalo RR se obtiene restando el tiempo entre dos picos consecutivos.  
Solo se aceptan valores fisiológicamente válidos (300–2000 ms).  
Los intervalos se almacenan en un buffer circular.

---

### 4️⃣ Cálculo de métricas de variabilidad (HRV)

A partir de los intervalos almacenados:

- Se calcula el promedio (RR_avg).
- Se obtiene RMSSD mediante diferencias cuadráticas sucesivas.
- Se calcula SDNN como desviación estándar.
- Se estiman oscilaciones respiratorias contando cruces respecto a la media.

---

### 5️⃣ Ventana de análisis

Cada 5 segundos el sistema:
- Calcula todas las métricas.
- Evalúa si hay acoplamiento RSA.
- Muestra los resultados por el Monitor Serial.

---

## 📤 Salida

El sistema imprime en el Monitor Serial las métricas calculadas junto con la indicación de si se detecta o no RSA.

---
