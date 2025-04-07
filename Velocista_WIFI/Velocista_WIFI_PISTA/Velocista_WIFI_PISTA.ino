//Incluir Librerias
#include <QTRSensors16.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <Preferences.h>

//GUARDAR EN MEMORIA
#define MAX_DATOS 500  // Máximo de datos guardados en la pista
#define VENTANA_PROMEDIO 5  // Tamaño de la ventana del promedio móvil

Preferences memoriaNVS;  // Objeto para manejar la memoria NVS
float pista[MAX_DATOS];  // Array para almacenar la pista
int indice = 0;  // Índice del array
bool primeraVuelta = true;  // Variable para saber si es la primera vuelta
bool VueltaCompletada=0;
//VALOR PROMEDIO
float errores[VENTANA_PROMEDIO];  // Array para el promedio móvil
int indicePromedio = 0;  // Índice del promedio móvil



//WIFI
const char *ssid = "Sollow_WIFI";
const char *password = "sofiaswifi";
byte WIFI_Status=1;
// Configuración de IP fija
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Inicia el servidor web
WiFiServer server(80);

//Modulo de Inicio
const byte MInit = D3;
int Estado;

//TURBINA
//Creación del Objeto
Servo myTurbina;

//PIN PARA EL CONTROL DE TURBINA
const byte Tur = D4;
int ValTurb = 150,minvaltur=50,maxvaltur=180; 
float KTurb=0.6;

//Variables para sensores
#define NUM_SENSORS 16            // Numero de sensores usados
#define NUM_SAMPLES_PER_SENSOR 3  // Numero de muestras
#define IN_PIN A2                 // PIN de entrada del multiplexor


// Inicialización del sensor, digitales D9,D10,D0,D1
QTRSensorsMux qtra((unsigned char[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
                   NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, (unsigned char)IN_PIN, (unsigned char[]){ D9, D10, D0, D1 });
unsigned int sensorValues[NUM_SENSORS];


//Variables para el controlador
float Tm = 9.0;  //tiempo de muestreo en mili segundos

//Variables del controlador PID
float Referencia = 0.0, Control = 0.0, Kp = 2.0, Ti = 0, Td = 0.02;
float Salida = 0.0, Error = 0.0, Error_ant = 0.0;  //variables de control
float offset = 1, Vmax = 250, E_integral;

//Variables del WIFI
String var1 = "0",var2 = "0",var3 = "0",var4 = "0",var5 = "0",var6 = "0";

//Variables Auxiliares
unsigned long int Tinicio = 0;
bool conect = false,turen = false;

//CREACIÓN DE PWM
const uint16_t Frecuencia = 5000;
const byte Canales[] = { 0, 1 };
const byte Resolucion = 10;

//Variables del Control del Motor
const int PWMI = D6;
const int PWMD = D8;
const int DirI = D5;
const int DirD = D7;



void setup() {
  Serial.begin(9600);

  //Inicialización de Pines
  Inicializacion_Pines();

  //Inicialización de la turbina
  Inicializacion_turbina();

  //Inicialización de Sensores
  Inicializacion_Sensores();

  //Creación del PWM
  CrearPWM();

  //Inicializacion Bluetooth
  Inicializacion_WIFI();

  //Para El funcionamiento de la Turbina
  delay(1000);

  myTurbina.write(ValTurb);
    
}

void loop() {
  
  //Estado = digitalRead(MInit);
  //Estado=1; //Para Desactivar el modulo de Inicio
  while (Estado) {
    //Estado = digitalRead(MInit);
    Tinicio = millis();                         // toma el valor en milisengundos
    Salida = Lectura_Sensor();                  // funcion de lectura de la variable salida del  proceso
    Control = Controlador(Referencia, Salida);  // funcion de la ley de control
    Esfuerzo_Control(Control);                  // funcion encargada de enviar el esfuerzo de control
    Tm = Tiempo_Muestreo(Tinicio);
    myTurbina.write(ValTurb);
    //Esfuerzo_Turbina(); //Turbina Variable
    if(WIFI_Status){
      Datos();
    }
    turen = true; //Variable que indica que se entro en el while
    //Serial.println("Entro");
  }
  if(turen){
    ledcWrite(Canales[0], 0);
    ledcWrite(Canales[1], 0);
    myTurbina.write(0);
    if(WIFI_Status){
      Datos();
    }
    //Serial.println("no");
  }
  ledcWrite(Canales[0], 0);
  ledcWrite(Canales[1], 0);
  if(WIFI_Status){
    Datos();
  }
}

//Para leer el sensor
float Lectura_Sensor(void) {
  Salida = (qtra.readLine(sensorValues) / 7500.0) - 1.0;
  //Serial.println(Salida);
  return Salida;  // retorno la variable de salidad del proceso normalizada entre 0-1, al olgoritmo de control
}

//Controlador para Motores
float Controlador(float Referencia, float Salida) {  // Funcion para la ley de control
  float E_derivativo;
  float Control;


  Error = Referencia - Salida;
  Error = (Error > -0.2 && Error < 0) ? 0 : (Error > 0 && Error < 0.2) ? 0: Error;

  Error = PromedioMovil(Error);

  if (primeraVuelta) {
    ReconocerPista(Error);  // Guarda la pista en la primera vuelta
    if (DetectarVuelta(Error)) {  
      primeraVuelta = false;  // Finaliza la primera vuelta
      GuardarPistaEnNVS();  // Guarda la pista en memoria
    }
  }

  // 🏎️ Usar la pista guardada para ajustar la velocidad
  float AjusteVelocidad = 1.0;
  if (!primeraVuelta) {
    int posicion = indice % MAX_DATOS;  // Usamos el índice actual
    if (abs(pista[posicion]) < 0.2) {  
      AjusteVelocidad = 1.5;  // Aumenta velocidad en rectas
    } else {  
      AjusteVelocidad = 0.7;  // Reduce velocidad en curvas
    }
  }

  E_integral = E_integral + (((Error * (Tm / 1000.0)) + ((Tm / 1000.0) * (Error - Error_ant))) / 2.0);
  E_integral = (E_integral > 100.0) ? 100.0 : (E_integral < -100.0) ? -100: E_integral;
  E_derivativo = (Error - Error_ant) / (Tm / 1000.0);
  Control = Kp * (Error + Ti * E_integral + Td * E_derivativo);
  Error_ant = Error;

  Control = AjusteVelocidad * Kp * (Error + Ti * E_integral + Td * E_derivativo);
  Control = constrain(Control, -2.5, 2.5);
  //Control = (Control > 2.5) ? 2.5 : (Control < -2.5) ? -2.5: Control;
  
  
  //Serial.println(Control);
  return Control;
}

void Esfuerzo_Control(float Control) {  //envia el esfuerzo de control en forma de PWM
  float s1, s2;

  s1 = (offset - Control);
  s2 = (offset + Control);

  ledcWrite(Canales[0], floor(constrain(abs(s1), 0.0, 1.0) * Vmax));
  ledcWrite(Canales[1], floor(constrain(abs(s2), 0.0, 1.0) * Vmax));

  if (s1 <= 0.0) {  // Motor Derecho
    digitalWrite(DirD, LOW);
  } else {
    digitalWrite(DirD, HIGH);
  }

  if (s2 <= 0.0) {  //Motor Izquierdo
    digitalWrite(DirI, LOW);
  } else {
    digitalWrite(DirI, HIGH);
  }
}

//Turbina Variable
void Esfuerzo_Turbina(){
  float estur;
  estur=constrain(round(minvaltur+((KTurb*abs(Error))*(maxvaltur-minvaltur))),minvaltur,maxvaltur);
  //Serial.println(" Esfuerzo: " + String(estur));
  myTurbina.write(estur);
}

unsigned long int Tiempo_Muestreo(unsigned long int Tinicio) {  //, unsigned int Tm){ // Funcion que asegura que el tiempo de muestreo sea el mismo siempre
  unsigned long int T = millis() - Tinicio;
  return T;
}

void CrearPWM() {
  ledcSetup(Canales[0], Frecuencia, Resolucion);
  ledcSetup(Canales[1], Frecuencia, Resolucion);
  ledcAttachPin(PWMD, Canales[0]);
  ledcAttachPin(PWMI, Canales[1]);
}
void Inicializacion_turbina() {
  //Mensajes de Inicio
  ESP32PWM::allocateTimer(2);
  myTurbina.setPeriodHertz(50);       //frecuencia de la señal cuadrada
  myTurbina.attach(Tur, 1000, 2000);  //(pin,min us de pulso, máx us de pulso)
  myTurbina.write(0); 
}

void Inicializacion_Sensores() {
  //Calibración Inicial de Pines Sensor
  for (int i = 0; i < 300; i++) {  // make the calibration take about 10 seconds
    qtra.calibrate();              // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
  }
  //delay(2000);
}

void Inicializacion_Pines() {
  pinMode(PWMD, OUTPUT);
  pinMode(PWMI, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(MInit, INPUT);
  digitalWrite(DirI, LOW);
  digitalWrite(PWMI, LOW);
}

void Inicializacion_WIFI(){
  // Configuración del Access Point
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  /*Serial.println("Access Point Iniciado");
  Serial.print("IP del servidor: ");
  Serial.println(WiFi.softAPIP());*/
  server.begin();
}

void Datos(){
  WiFiClient client = server.available();
  if (client) {
    //Serial.println("Cliente conectado");
    String request = "";
    // Lee la solicitud completa del cliente
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') break;
      }
    }
    
    Serial.println("Solicitud recibida:");
    Serial.println(request);
    request.trim();  // Elimina espacios y saltos de línea


    // Extraer las 5 variables desde la URL
    if (request.indexOf("GET /?") != -1 && request.indexOf("accion=leer") == -1) {
      var1 = getValue(request, "var1=");
      var2 = getValue(request, "var2=");
      var3 = getValue(request, "var3=");
      var4 = getValue(request, "var4=");
      var5 = getValue(request, "var5=");
      var6 = getValue(request, "var6=");
      
      /*// Muestra las variables correctamente en el Serial
      Serial.println("Variables actualizadas:");
      Serial.println("Var1: " + var1);
      Serial.println("Var2: " + var2);
      Serial.println("Var3: " + var3);
      Serial.println("Var4: " + var4);
      Serial.println("Var5: " + var5);
      Serial.println("Var6: " + var6);*/

      if (var1 != "") Kp = var1.toFloat();
      if (var2 != "") Td = var2.toFloat();
      if (var3 != "") Ti = var3.toFloat();
      if (var4 != "") ValTurb = var4.toFloat();
      if (var5 != "") Vmax = var5.toFloat();
      if (var6 != "") offset = var6.toFloat();

      /*Serial.println("Variables actualizadas:");
      Serial.println("Kp: " + String(Kp));
      Serial.println("Td " + String(Td));
      Serial.println("Ti " + String(Ti));
      Serial.println("ValTurb: " + String(ValTurb));
      Serial.println("Vmax: " + String(Vmax));
      Serial.println("offset: " + String(offset));*/
      client.println("OK");
    }

    // ✅ Responde a la app solo con los valores
    if (request.indexOf("accion=leer") != -1) {
      //Serial.println("Botón Leer presionado desde la app");
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println(String(Kp) + "," + String(Td) + "," + String(Ti) + "," + String(ValTurb) + "," + String(Vmax) + "," + String(offset) + "," +  String(Estado));
      client.println();
    }
    if (request.indexOf("accion=inicio") != -1) {
      Estado = 1;
      /*Serial.print("Estado: ");
      Serial.println(Estado);*/
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("OK");
      client.println();
    }
    if (request.indexOf("accion=parar") != -1) {
      Estado = 0;
      /*Serial.print("Estado: ");
      Serial.println(Estado);*/
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("OK");
      client.println();
    }

    if (request.indexOf("accion=vueltainicio") != -1 && VueltaCompletada == 1) {
      VueltaCompletada = 0;  // Se activa solo una vez
      Serial.print("Vuelta: ");
      Serial.println(VueltaCompletada);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("Vuelta OK");
      client.println();
    }

    if (request.indexOf("accion=vuelta") != -1 && VueltaCompletada == 0) {
      VueltaCompletada = 1;  // Se activa solo una vez
      Serial.print("Vuelta: ");
      Serial.println(VueltaCompletada);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("Vuelta OK");
      client.println();
    }

    if (request == "") {
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-type:text/plain");
      client.println();
      client.println("Error: solicitud vacía");
      client.println();
    }

    client.stop();
    //Serial.println("Cliente desconectado");
  }
}

String getValue(String data, String key) {
  int start = data.indexOf(key);
  if (start == -1) return "";
  start += key.length();
  int end = data.indexOf("&", start);
  if (end == -1) end = data.indexOf(" ", start);
  return data.substring(start, end);
}

// Función para calcular el promedio móvil
float PromedioMovil(float nuevoError) {
  errores[indicePromedio] = nuevoError;  // Guardar nuevo error en la ventana
  indicePromedio = (indicePromedio + 1) % VENTANA_PROMEDIO;  // Mover índice
  
  float suma = 0;
  for (int i = 0; i < VENTANA_PROMEDIO; i++) {
    suma += errores[i];
  }
  return suma / VENTANA_PROMEDIO;
}

// Función para reconocer la pista en la primera vuelta
void ReconocerPista(float error) {
  if (indice < MAX_DATOS) {
    pista[indice] = error;
    indice++;
  } else {
    primeraVuelta = false;  // Finaliza el reconocimiento de la pista
    GuardarPistaEnNVS();  // Guarda la pista en memoria
  }
}

// Función para guardar la pista en memoria NVS
void GuardarPistaEnNVS() {
  if (!VueltaCompletada) {
    memoriaNVS.begin("pista", false);  // Abre NVS
    for (int i = 0; i < MAX_DATOS; i++) {
      memoriaNVS.putFloat(String(i).c_str(), pista[i]);  // Guarda cada valor
    }
    memoriaNVS.end();  // Cierra NVS
  }
}

// Función para cargar la pista desde la memoria NVS
void CargarPistaDesdeNVS() {
  memoriaNVS.begin("pista", true);  // Abre NVS en modo lectura
  for (int i = 0; i < MAX_DATOS; i++) {
    pista[i] = memoriaNVS.getFloat(String(i).c_str(), 0.0);  // Carga cada valor
  }
  memoriaNVS.end();  // Cierra NVS
}

void Inicializacion_Memoria(){
  memoriaNVS.begin("pista", true);  // Abre la memoria NVS en modo lectura
  if (memoriaNVS.isKey("0")) {  // Si hay datos guardados
    CargarPistaDesdeNVS();  // Cargar la pista en la memoria del robot
    primeraVuelta = false;  // Ya tiene datos guardados, no necesita aprender
  }
  memoriaNVS.end();
}

bool DetectarVuelta(float errorActual) {
  // Compara los primeros valores guardados con el error actual
  if (indice >= MAX_DATOS / 2) {  // Solo compara después de recorrer la mitad
    for (int i = 0; i < 10; i++) {  // Compara con los primeros 10 datos de la pista
      if (abs(errorActual - pista[i]) > 0.05) {  
        return false;  // Si hay mucha diferencia, aún no completó la vuelta
      }
    }
    return true;  // Si los valores son similares, ha completado la vuelta
  }
  return false;
}
